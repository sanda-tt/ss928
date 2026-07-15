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

#define AI_MCU_SAMPLE_NOT_QUANT 0             /* Route No.1: Not Quant */
#define AI_MCU_SAMPLE_MICRO_QUANT 0           /* Route No.2: Micro Quant */
#define AI_MCU_SAMPLE_TFLITE_QUANT 1          /* Route No.3: TFLite Quant */

/* Task Param */
#define TASKS_MCU_AI_STACK_SIZE 0x1000       /* Max Stack Size in this task */
#define TASKS_MCU_AI_PRIO (osPriority_t)(17) /* Task Priority in LiteOS */
#define TASKS_MCU_AI_DELAY_MS 10              /* Default */
#define TASKS_MCU_AI_TICKS_PER_SECOND 24000  /* Ticks to ms */
#define TASKS_MCU_AI_PRINT_FLOAT_MULTIPILER 100000 /* Float Print Bits. For example: 0.99998 */

/* Model Param */
#define AI_MCU_SAMPLE_INPUT_1_SIZE 600 /* Example: 1 * 25 * 24 * 1 */
/* To Add More Inputs
 * #define AI_MCU_SAMPLE_INPUT_2_SIZE <Tensor Size 2>
 * #define AI_MCU_SAMPLE_INPUT_3_SIZE <Tensor Size 3>
*/

/* third party param (TFLite) */
#define AI_MCU_SAMPLE_TFLITE_INPUT_DATATYPE uint8_t /* support change to int8_t */
#define AI_MCU_SAMPLE_TFLITE_OUTPUT_DATATYPE uint8_t /* support change to int8_t */
#define AI_MCU_SAMPLE_TFLITE_INPUT_1_QUANT_MULTIPILER 1.0 /* Example: 0.031646765768527985 */
#define AI_MCU_SAMPLE_TFLITE_INPUT_1_QUANT_ZP 0 /* Example: 53 */
#define AI_MCU_SAMPLE_TFLITE_OUTPUT_1_QUANT_MULTIPILER 1.0 /* Example: 0.00390625 */
#define AI_MCU_SAMPLE_TFLITE_OUTPUT_1_QUANT_ZP 0 /* Example: 0 */
/* To Add More input/output quant params
 * #define AI_MCU_SAMPLE_TFLITE_INPUT_2_QUANT_MULTIPILER <Input 2 Scale From Model >
 * #define AI_MCU_SAMPLE_TFLITE_INPUT_2_QUANT_ZP <Input 2 Zero Point From Model >
 * #define AI_MCU_SAMPLE_TFLITE_INPUT_3_QUANT_MULTIPILER <Input 3 Scale From Model >
 * #define AI_MCU_SAMPLE_TFLITE_INPUT_3_QUANT_ZP <Input 3 Zero Point From Model >
 * #define AI_MCU_SAMPLE_TFLITE_OUTPUT_2_QUANT_MULTIPILER <Input 2 Scale From Model >
 * #define AI_MCU_SAMPLE_TFLITE_OUTPUT_2_QUANT_ZP <Input 2 Zero Point From Model >
*/

struct ai_mcu_param {
    OH_AI_ModelHandle model;
    OH_AI_ContextHandle context;
    OH_AI_TensorHandleArray inputs;
    OH_AI_TensorHandleArray outputs;
};

/* Input Data */
const float input_buffer_fp32[AI_MCU_SAMPLE_INPUT_1_SIZE] = { 0.0 };
/* To Add More Inputs
 * static float input_buffer_fp32_2[AI_MCU_SAMPLE_INPUT_2_SIZE] = { 0.0 };
 * static float input_buffer_fp32_3[AI_MCU_SAMPLE_INPUT_3_SIZE] = { 0.0 };
*/
/* To Add Input Datatype for index
 * static int32_t input_buffer_fp32_2[AI_MCU_SAMPLE_INPUT_2_SIZE] = { 0.0 };
*/

static void ai_mcu_sample_load_data(void *input_buffer, void *data_buf, size_t size, float scale, float zp)
{
#if AI_MCU_SAMPLE_NOT_QUANT
    unused(scale);
    unused(zp);
    size_t mem_size = size * sizeof(float);
    memcpy_s(input_buffer, mem_size, data_buf, mem_size);
#elif AI_MCU_SAMPLE_MICRO_QUANT
    unused(scale);
    unused(zp);
    size_t mem_size = size * sizeof(float);
    memcpy_s(input_buffer, mem_size, data_buf, mem_size);
#elif AI_MCU_SAMPLE_TFLITE_QUANT
    for (size_t b = 0; b < size; b++) {
        ((AI_MCU_SAMPLE_TFLITE_INPUT_DATATYPE *)input_buffer)[b] = (((float *)data_buf)[b] / scale + zp);
    }
#endif
}
/* To Add Input Datatype for index (used the function below)
    static void ai_mcu_sample_index_load_data(void *input_buffer, void *data_buf, size_t size, float scale, float zp)
    {
    #if AI_MCU_SAMPLE_NOT_QUANT
        unused(scale);
        unused(zp);
        size_t mem_size = size * sizeof(int32_t);
        memcpy_s(input_buffer, mem_size, data_buf, mem_size);
    #elif AI_MCU_SAMPLE_MICRO_QUANT
        unused(scale);
        unused(zp);
        size_t mem_size = size * sizeof(int32_t);
        memcpy_s(input_buffer, mem_size, data_buf, mem_size);
    #else
        for (size_t b = 0; b < size; b++) {
          ((AI_MCU_SAMPLE_TFLITE_INPUT_DATATYPE *)input_buffer)[b] = (((int32_t *)data_buf)[b] / scale + zp);
        }
    #endif
    }
*/

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
/* To change printf precision (such as 8 bit)
 * 1. change {osal_printk("[-%d.%05d]",} ----> {osal_printk("[-%d.%08d]",}
 * 2. change {TASKS_MCU_AI_PRINT_FLOAT_MULTIPILER 100000} ---> {TASKS_MCU_AI_PRINT_FLOAT_MULTIPILER 100000000}
*/

static OH_AI_Status ai_mcu_sample_print_output_tensor(OH_AI_TensorHandle tensor, float scale, float zp)
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
#if AI_MCU_SAMPLE_NOT_QUANT
        unused(scale);
        unused(zp);
        float f = ((float *)out_data)[i];
        ai_mcu_sample_printf_float(f);
#elif AI_MCU_SAMPLE_MICRO_QUANT
        unused(scale);
        unused(zp);
        float f = ((float *)out_data)[i];
        ai_mcu_sample_printf_float(f);
#elif AI_MCU_SAMPLE_TFLITE_QUANT
        AI_MCU_SAMPLE_TFLITE_OUTPUT_DATATYPE d = ((AI_MCU_SAMPLE_TFLITE_OUTPUT_DATATYPE *)out_data)[i];
        float f = ((float)d - zp) * scale;
        ai_mcu_sample_printf_float(f);
#endif
    }
    osal_printk("\n");
    return OH_AI_STATUS_SUCCESS;
}
/* To Add Output Datatype for index (used the function below)
 1. change {float f = ((float *)out_data)[i];} --> {int32_t f = ((int32_t *)out_data)[i];}
 2. change {ai_mcu_sample_printf_float(f);} --> {osal_printk("%d", f);}
*/

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
    /* Add More Input
     * void *input_data_2 = OH_AI_TensorGetMutableData(sample_param.inputs.handle_list[1]);
     * if (input_data_2 == NULL) {
     *     osal_printk("[AI_MCU] OH_AI_TensorGetMutableData 2 failed\n");
     *     return OH_AI_STATUS_FAILED;
     * }
     * void *input_data_3 = OH_AI_TensorGetMutableData(sample_param.inputs.handle_list[2]);
     * if (input_data_3 == NULL) {
     *     osal_printk("[AI_MCU] OH_AI_TensorGetMutableData 2 failed\n");
     *     return OH_AI_STATUS_FAILED;
     * }
    */

    ai_mcu_sample_load_data(input_data, (void *)input_buffer_fp32, AI_MCU_SAMPLE_INPUT_1_SIZE,
        AI_MCU_SAMPLE_TFLITE_INPUT_1_QUANT_MULTIPILER, AI_MCU_SAMPLE_TFLITE_INPUT_1_QUANT_ZP);
    /*
     * Add More Input
     * ai_mcu_sample_load_data(input_data_2, (void *)input_buffer_fp32_2, AI_MCU_SAMPLE_INPUT_2_SIZE,
     *     AI_MCU_SAMPLE_TFLITE_INPUT_2_QUANT_MULTIPILER, AI_MCU_SAMPLE_TFLITE_INPUT_2_QUANT_ZP);
     * ai_mcu_sample_load_data(input_data_3, (void *)input_buffer_fp32_3, AI_MCU_SAMPLE_INPUT_3_SIZE,
     *     AI_MCU_SAMPLE_TFLITE_INPUT_3_QUANT_MULTIPILER, AI_MCU_SAMPLE_TFLITE_INPUT_3_QUANT_ZP);
    */

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
    OH_AI_TensorHandle output = sample_param.outputs.handle_list[0];
    if (output == NULL) {
        osal_printk("[AI_MCU] OH_AI_ModelGetOutputs failed\n");
        return OH_AI_STATUS_FAILED;
    }
    /* Add more output
     * OH_AI_TensorHandle output_2 = sample_param.outputs.handle_list[1];
     * if (output_2 == NULL) {
     *     osal_printk("[AI_MCU] OH_AI_ModelGetOutputs 2 failed\n");
     *     return OH_AI_STATUS_FAILED;
     * }
    */

    ret = ai_mcu_sample_print_output_tensor(output, AI_MCU_SAMPLE_TFLITE_OUTPUT_1_QUANT_MULTIPILER,
        AI_MCU_SAMPLE_TFLITE_OUTPUT_1_QUANT_ZP);
    if (ret != OH_AI_STATUS_SUCCESS) {
        osal_printk("[AI_MCU] ai_mcu_sample_print_output_tensor failed (%d)\n", ret);
        return ret;
    }
    /* Add more output
     * ret = ai_mcu_sample_print_output_tensor(output_2, AI_MCU_SAMPLE_TFLITE_OUTPUT_2_QUANT_MULTIPILER
     *     AI_MCU_SAMPLE_TFLITE_OUTPUT_2_QUANT_ZP);
     * if (ret != OH_AI_STATUS_SUCCESS) {
     *     osal_printk("[AI_MCU] ai_mcu_sample_print_output_tensor 2 failed (%d)\n", ret);
     *     return ret;
     * }
    */
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
