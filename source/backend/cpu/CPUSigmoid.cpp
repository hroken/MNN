//
//  CPUSigmoid.cpp
//  MNN
//
//  Created by MNN on 2018/08/09.
//  Copyright © 2018, Alibaba Group Holding Limited
//

#include "backend/cpu/CPUSigmoid.hpp"
#include <math.h>
#include "backend/cpu/CPUBackend.hpp"
#include "backend/cpu/compute/CommonOptFunction.h"
#include "core/Macro.h"

#ifdef MNN_USE_NEON
#include <arm_neon.h>
#endif

namespace MNN {
ErrorCode CPUSigmoid::onExecute(const std::vector<Tensor*>& inputs, const std::vector<Tensor*>& outputs) {
    MNN_ASSERT(1 == inputs.size());
    MNN_ASSERT(1 == outputs.size());
    auto inputData  = inputs[0]->host<float>();
    auto outputData = outputs[0]->host<float>();

    const int dataSize = outputs[0]->elementSize();
    MNNExp(outputData, inputData, dataSize);

#ifdef MNN_USE_NEON
    if(dataSize % 4 == 0)
    {
        // neon optimization for sigmid cpu 
        float32x4_t value = vdupq_n_f32(1.0);
        float32x4_t out = vld1q_f32(outputData);

        for (int i = 0; i < dataSize; i+=4) {

            if(i != 0)
                out = vld1q_f32(outputData);
            out = vrecpeq_f32(vaddq_f32(value,out));
            vst1q_f32 (outputData ,out);
            outputData += 4;
        }
    }
    else
#endif
    {
        for (int i = 0; i < dataSize; i++) {
            outputData[i] = 1.0f / (1.0f + outputData[i]);
        }
    }
    return NO_ERROR;
}

class CPUSigmoidCreator : public CPUBackend::Creator {
public:
    virtual Execution* onCreate(const std::vector<Tensor*>& inputs, const std::vector<Tensor*>& outputs,
                                const MNN::Op* op, Backend* backend) const {
        return new CPUSigmoid(backend);
    }
};

REGISTER_CPU_OP_CREATOR(CPUSigmoidCreator, OpType_Sigmoid);
} // namespace MNN
