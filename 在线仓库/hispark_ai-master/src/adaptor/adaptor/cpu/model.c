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
#include "std_def.h"
#include "ai_mcu.h"
#include "ai.h"

OH_AI_ModelHandle OH_AI_ModelCreate(void)
{
    MSModelHandle model_handle = MSModelCreate();
    return (OH_AI_ModelHandle)model_handle;
}

OH_AI_Status OH_AI_ModelBuildFromFile(
    OH_AI_ModelHandle model, const char *model_path, const OH_AI_ContextHandle model_context)
{
    UNUSED(model);
    UNUSED(model_path);
    UNUSED(model_context);
    return OH_AI_STATUS_SUCCESS;
}

OH_AI_Status OH_AI_ModelBuild(
    OH_AI_ModelHandle model, const void *model_data, size_t data_size, const OH_AI_ContextHandle model_context)
{
    if (model == NULL || model_context == NULL) {
        return OH_AI_STATUS_FAILED;
    }
    
    MSStatus status = MSModelBuild(
        (MSModelHandle)model,
        model_data,
        data_size,
        kMSModelTypeMindIR,
        (MSContextHandle)model_context
    );
    if (status == kMSStatusSuccess) {
        return OH_AI_STATUS_SUCCESS;
    } else {
        return OH_AI_STATUS_FAILED;
    }
}

OH_AI_Status OH_AI_ModelBuildFromName(
    OH_AI_ModelHandle model, const char *model_name, const OH_AI_ContextHandle model_context)
{
    UNUSED(model);
    UNUSED(model_name);
    UNUSED(model_context);
    return OH_AI_STATUS_SUCCESS;
}

void OH_AI_ModelDestroy(OH_AI_ModelHandle *model)
{
    if (model == NULL) {
        return;
    }

    MSModelDestroy((MSModelHandle *)model);

    *model = NULL;
}

OH_AI_TensorHandleArray OH_AI_ModelGetInputs(const OH_AI_ModelHandle model)
{
    OH_AI_TensorHandleArray stub_array;
    stub_array.handle_num = 0;
    stub_array.handle_list = NULL;

    if (model == NULL) {
        return stub_array;
    }

    MSTensorHandleArray inputs = MSModelGetInputs((MSModelHandle)model);

    stub_array.handle_num = inputs.handle_num;
    stub_array.handle_list = (OH_AI_TensorHandle *)inputs.handle_list;

    return stub_array;
}

OH_AI_TensorHandleArray OH_AI_ModelGetOutputs(const OH_AI_ModelHandle model)
{
    OH_AI_TensorHandleArray stub_array;
    stub_array.handle_num = 0;
    stub_array.handle_list = NULL;

    if (model == NULL) {
        return stub_array;
    }

    MSTensorHandleArray outputs = MSModelGetOutputs((MSModelHandle)model);
    
    stub_array.handle_num = outputs.handle_num;
    stub_array.handle_list = (OH_AI_TensorHandle *)outputs.handle_list;
    
    return stub_array;
}

OH_AI_Status OH_AI_ModelPredict(
    OH_AI_ModelHandle model, const OH_AI_TensorHandleArray inputs, OH_AI_TensorHandleArray *outputs)
{
    MSModelHandle ms_model = (MSModelHandle)model;
    
    MSTensorHandleArray ms_inputs;
    ms_inputs.handle_num = inputs.handle_num;
    ms_inputs.handle_list = (MSTensorHandle *)inputs.handle_list;

    MSStatus status = MSModelPredict(ms_model, ms_inputs, (MSTensorHandleArray*)outputs, NULL, NULL);
    if (status == kMSStatusSuccess) {
        return OH_AI_STATUS_SUCCESS;
    } else {
        return OH_AI_STATUS_FAILED;
    }
}
