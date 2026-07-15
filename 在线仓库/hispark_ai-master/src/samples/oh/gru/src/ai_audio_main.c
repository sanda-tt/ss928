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
#include "app_init.h"
#include "osal_debug.h"
#include "common_def.h"
#include "cmsis_os2.h"
#include "log_common.h"
#include "log_uart.h"
#include "log_oam_logger.h"
#include "log_oml_exception.h"
#include "securec.h"
#include "tcxo.h"
#include "ai.h"

/* Task Param */
#define TASKS_MCU_AI_STACK_SIZE 0x3000       /* Max Stack Size in this task */
#define TASKS_MCU_AI_PRIO (osPriority_t)(17) /* Task Priority in LiteOS */
#define TASKS_MCU_AI_DELAY_MS 10              /* Default */
#define TASKS_MCU_AI_TICKS_PER_SECOND 24000  /* Ticks to ms */
#define TASKS_MCU_AI_PRINT_FLOAT_MULTIPILER 100000 /* Float Print Bits. For example: 0.99998 */

/* Model Param */
#define AI_MCU_SAMPLE_GRU_TIMESTAMP 25
#define AI_MCU_SAMPLE_MFCC_INPUT_SIZE 10
#define AI_MCU_SAMPLE_HIDDEN_STATE_SIZE 154

struct ai_mcu_param {
    OH_AI_ModelHandle model;
    OH_AI_ContextHandle context;
    OH_AI_TensorHandleArray inputs;
    OH_AI_TensorHandleArray outputs;
};

/* Input Data */
const float mfcc_input_buffer[AI_MCU_SAMPLE_GRU_TIMESTAMP * AI_MCU_SAMPLE_MFCC_INPUT_SIZE] = {
    0.0
};

static void ai_mcu_sample_load_data(void *input_buffer, void *data_buf, size_t size)
{
    size_t mem_size = size * sizeof(float);
    memcpy_s(input_buffer, mem_size, data_buf, mem_size);
}

static void ai_mcu_sample_printf_float(float f)
{
    if (f < 0.0) {
        float unsigned_f = -f;
        osal_printk("[-%d.%05d]",
            (int)(unsigned_f * TASKS_MCU_AI_PRINT_FLOAT_MULTIPILER) / TASKS_MCU_AI_PRINT_FLOAT_MULTIPILER,
            (int)(unsigned_f * TASKS_MCU_AI_PRINT_FLOAT_MULTIPILER) % TASKS_MCU_AI_PRINT_FLOAT_MULTIPILER);
    } else {
        osal_printk("[%d.%05d]",
            (int)(f * TASKS_MCU_AI_PRINT_FLOAT_MULTIPILER) / TASKS_MCU_AI_PRINT_FLOAT_MULTIPILER,
            (int)(f * TASKS_MCU_AI_PRINT_FLOAT_MULTIPILER) % TASKS_MCU_AI_PRINT_FLOAT_MULTIPILER);
    }
}

static OH_AI_Status ai_mcu_sample_print_output_tensor(OH_AI_TensorHandle tensor)
{
    size_t data_size = OH_AI_TensorGetDataSize(tensor);
    osal_printk("[AI_MCU] Data size: [%zu]\n", data_size);
    osal_printk("Shape: [");
    size_t shape_num = 0;
    const int64_t *dims = OH_AI_TensorGetShape(tensor, &shape_num);
    if (dims == NULL) {
        osal_printk("[AI_MCU] OH_AI_TensorGetShape failed\n");
        return OH_AI_STATUS_FAILED;
    }
    for (size_t i = 0; i < shape_num; i++) {
        osal_printk("%d ", (int)dims[i]);
    }
    osal_printk("]\n");

    OH_AI_DataType data_type = OH_AI_TensorGetDataType(tensor);
    if (data_type == OH_AI_DATATYPE_UNKNOWN) {
        osal_printk("[AI_MCU] OH_AI_TensorGetDataType failed\n");
        return OH_AI_STATUS_FAILED;
    }
    osal_printk("DataType: %d\n", data_type);

    void *out_data = OH_AI_TensorGetMutableData(tensor);
    if (out_data == NULL) {
        osal_printk("[AI_MCU] OH_AI_TensorGetMutableData failed\n");
        return OH_AI_STATUS_FAILED;
    }
    osal_printk("[AI_MCU] Data: ");
    for (size_t i = 0; i < (size_t)(OH_AI_TensorGetElementNum(tensor)); i++) {
        float f = ((float *)out_data)[i];
        ai_mcu_sample_printf_float(f);
    }
    osal_printk("\n");
    return OH_AI_STATUS_SUCCESS;
}

static OH_AI_Status ai_mcu_sample_init(struct ai_mcu_param *sample_param)
{
    /* MS Model Init (Only once) */
    osal_printk("[AI_MCU] ai_mcu_sample_init\n");
    OH_AI_ModelHandle model = OH_AI_ModelCreate();
    OH_AI_ContextHandle context = OH_AI_ContextCreate();
    sample_param->model = model;
    sample_param->context = context;
    OH_AI_Status ret = OH_AI_ModelBuild(sample_param->model, NULL, 0, sample_param->context);
    if (ret != OH_AI_STATUS_SUCCESS) {
        osal_printk("[AI_MCU] OH_AI_ModelBuild failed (%d)\n", ret);
        return ret;
    }
    OH_AI_TensorHandleArray inputs = OH_AI_ModelGetInputs(sample_param->model);
    sample_param->inputs = inputs;
    if (inputs.handle_list == NULL) {
        osal_printk("[AI_MCU] OH_AI_ModelGetInputs failed\n");
        return OH_AI_STATUS_FAILED;
    }
    OH_AI_TensorHandleArray outputs = OH_AI_ModelGetOutputs(sample_param->model);
    sample_param->outputs = outputs;
    if (outputs.handle_list == NULL) {
        osal_printk("[AI_MCU] OH_AI_ModelGetOutputs failed\n");
        return OH_AI_STATUS_FAILED;
    }
    return OH_AI_STATUS_SUCCESS;
}

static OH_AI_Status ai_mcu_sample_process(struct ai_mcu_param sample_param)
{
    osal_printk("[AI_MCU] ai_mcu_sample_process\n");

    /* MS Prepare Input Data */
    void *input_data = OH_AI_TensorGetMutableData(sample_param.inputs.handle_list[0]);
    if (input_data == NULL) {
        osal_printk("[AI_MCU] OH_AI_TensorGetMutableData failed\n");
        return OH_AI_STATUS_FAILED;
    }
    void *input_data_2 = OH_AI_TensorGetMutableData(sample_param.inputs.handle_list[1]);
    if (input_data_2 == NULL) {
        osal_printk("[AI_MCU] OH_AI_TensorGetMutableData 2 failed\n");
        return OH_AI_STATUS_FAILED;
    }
    float *input_states = (float *)input_data_2;
    for (size_t i = 0; i < AI_MCU_SAMPLE_HIDDEN_STATE_SIZE; i++) {
        input_states[i] = 0.0;
    }
    for (int timestamp = 0; timestamp < AI_MCU_SAMPLE_GRU_TIMESTAMP; timestamp++) {
        /* Prepare MFCC Input */
        ai_mcu_sample_load_data(input_data, (void *)(mfcc_input_buffer + timestamp *
            AI_MCU_SAMPLE_MFCC_INPUT_SIZE), AI_MCU_SAMPLE_MFCC_INPUT_SIZE);

        /* MS Model Predict */
        uint64_t l1 = uapi_tcxo_get_count();
        OH_AI_Status ret = OH_AI_ModelPredict(sample_param.model, sample_param.inputs, &(sample_param.outputs));
        uint64_t l2 = uapi_tcxo_get_count();
        osal_printk("[AI_MCU] Get Tcxo Time %d ms\n", ((int)l2 - (int)l1) / TASKS_MCU_AI_TICKS_PER_SECOND);
        if (ret != OH_AI_STATUS_SUCCESS) {
            osal_printk("[AI_MCU] OH_AI_ModelPredict failed (%d)\n", ret);
            return ret;
        }

        /* PostProcess */
        OH_AI_TensorHandle output = sample_param.outputs.handle_list[1];
        if (output == NULL) {
            osal_printk("[AI_MCU] OH_AI_ModelGetOutputs failed\n");
            return OH_AI_STATUS_FAILED;
        }
        void *output_hidden_state = OH_AI_TensorGetMutableData(sample_param.outputs.handle_list[1]);
        memcpy_s(input_data_2, AI_MCU_SAMPLE_HIDDEN_STATE_SIZE * sizeof(float), output_hidden_state,
            AI_MCU_SAMPLE_HIDDEN_STATE_SIZE * sizeof(float));
    }
    OH_AI_TensorHandle output_logits = sample_param.outputs.handle_list[0];
    if (output_logits == NULL) {
        osal_printk("[AI_MCU] OH_AI_ModelGetOutputs failed\n");
        return OH_AI_STATUS_FAILED;
    }
    OH_AI_Status ret = ai_mcu_sample_print_output_tensor(output_logits);
    if (ret != OH_AI_STATUS_SUCCESS) {
        osal_printk("[AI_MCU] ai_mcu_sample_print_output_tensor failed (%d)\n", ret);
        return ret;
    }
    return OH_AI_STATUS_SUCCESS;
}

static void ai_mcu_sample_destroy(struct ai_mcu_param *sample_param)
{
    osal_printk("[AI_MCU] ai_mcu_sample_destroy\n");
    /* MS Model Destroy (Only once) */
    OH_AI_ContextDestroy(&(sample_param->context));
    OH_AI_ModelDestroy(&(sample_param->model));
}

static void *ai_mcu_task(const char *arg)
{
    unused(arg);
    /* MS Model Init (Only once) */
    struct ai_mcu_param sample_param = {0};
    OH_AI_Status ret = ai_mcu_sample_init(&sample_param);
    if (ret != OH_AI_STATUS_SUCCESS) {
        osal_printk("[AI_MCU] ai_mcu_sample_init failed (%d)\n", ret);
        ai_mcu_sample_destroy(&sample_param);
        return NULL;
    }
    while (true) {
        ret = ai_mcu_sample_process(sample_param);
        if (ret != OH_AI_STATUS_SUCCESS) {
            osal_printk("[AI_MCU] ai_mcu_sample_process failed (%d)\n", ret);
            ai_mcu_sample_destroy(&sample_param);
            return NULL;
        }
        osDelay(TASKS_MCU_AI_DELAY_MS);
    }
    ai_mcu_sample_destroy(&sample_param);
    return NULL;
}

static void tasks_test_entry(void)
{
    osThreadAttr_t attr;
    attr.name = "AI_MCU_Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TASKS_MCU_AI_STACK_SIZE;
    attr.priority = TASKS_MCU_AI_PRIO;

    if (osThreadNew((osThreadFunc_t)ai_mcu_task, NULL, &attr) == NULL) {
        /* Create task fail. */
        osal_printk("[AI_MCU] Task Create Failed\n");
    }
}

/* Run the tasks_test_entry. */
app_run(tasks_test_entry);
