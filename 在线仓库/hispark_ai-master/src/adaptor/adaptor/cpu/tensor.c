/**
 * Copyright (c) 2025-2025 HiSilicon (Shanghai) Technologies Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "ai_mcu.h"
#include "ai.h"

size_t OH_AI_TensorGetDataSize(const OH_AI_TensorHandle tensor)
{
    if (tensor == NULL) {
        return 0;
    }

    return MSTensorGetDataSize((MSTensorHandle)tensor);
}

const int64_t *OH_AI_TensorGetShape(const OH_AI_TensorHandle tensor, size_t *shape_num)
{
    if (tensor == NULL || shape_num == NULL) {
        if (shape_num != NULL) {
            *shape_num = 0;
        }
        return NULL;
    }

    return MSTensorGetShape((MSTensorHandle)tensor, shape_num);
}

int64_t OH_AI_TensorGetElementNum(const OH_AI_TensorHandle tensor)
{
    return MSTensorGetElementNum((MSTensorHandle)tensor);
}

OH_AI_DataType OH_AI_TensorGetDataType(const OH_AI_TensorHandle tensor)
{
    if (tensor == NULL) {
        return OH_AI_DATATYPE_UNKNOWN;
    }

    MSDataType data_type = MSTensorGetDataType((MSTensorHandle)tensor);
    if (data_type < kMSDataTypeUnknown || data_type > kMSDataTypeInvalid) {
        return OH_AI_DATATYPE_UNKNOWN;
    }

    return (OH_AI_DataType)data_type;
}

void *OH_AI_TensorGetMutableData(const OH_AI_TensorHandle tensor)
{
    if (tensor == NULL) {
        return NULL;
    }

    return MSTensorGetMutableData((MSTensorHandle)tensor);
}