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
#ifndef AI_H_
#define AI_H_

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif  /* __cplusplus */
#endif  /* __cplusplus */

/**
 * @defgroup middleware_utils_ai_api AI
 * @ingroup middleware_utils
 * @{
 */

/**
 * @if Eng
 * @brief  Default compiler feature type of AI API.
 * @else
 * @brief  AI接口默认编译器特性
 * @endif
 */
#ifndef OH_AI_API
#ifdef _WIN32
#define OH_AI_API __declspec(dllexport)
#else
#define OH_AI_API __attribute__((visibility("default")))
#endif
#endif


/**
 * @if Eng
 * @brief  AI return status
 * @else
 * @brief  AI返回状态码
 * @endif
 */
typedef enum OH_AI_Status {
    OH_AI_STATUS_SUCCESS = 0,
    OH_AI_STATUS_FAILED = 1,
} OH_AI_Status;


/**
 * @if Eng
 * @brief  Data type
 * @else
 * @brief  数据类型
 * @endif
 */
typedef enum OH_AI_DataType {
    OH_AI_DATATYPE_UNKNOWN = 0,
    OH_AI_DATATYPE_OBJECTTYPE_STRING = 12,
    OH_AI_DATATYPE_OBJECTTYPE_LIST = 13,
    OH_AI_DATATYPE_OBJECTTYPE_TUPLE = 14,
    OH_AI_DATATYPE_OBJECTTYPE_TENSOR = 17,
    OH_AI_DATATYPE_NUMBERTYPE_BEGIN = 29,
    OH_AI_DATATYPE_NUMBERTYPE_BOOL = 30,
    OH_AI_DATATYPE_NUMBERTYPE_INT8 = 32,
    OH_AI_DATATYPE_NUMBERTYPE_INT16 = 33,
    OH_AI_DATATYPE_NUMBERTYPE_INT32 = 34,
    OH_AI_DATATYPE_NUMBERTYPE_INT64 = 35,
    OH_AI_DATATYPE_NUMBERTYPE_UINT8 = 37,
    OH_AI_DATATYPE_NUMBERTYPE_UINT16 = 38,
    OH_AI_DATATYPE_NUMBERTYPE_UINT32 = 39,
    OH_AI_DATATYPE_NUMBERTYPE_UINT64 = 40,
    OH_AI_DATATYPE_NUMBERTYPE_FLOAT16 = 42,
    OH_AI_DATATYPE_NUMBERTYPE_FLOAT32 = 43,
    OH_AI_DATATYPE_NUMBERTYPE_FLOAT64 = 44,
    OH_AI_DATATYPE_NUMBERTYPE_END = 46,
    OH_AI_DataTypeInvalid = INT32_MAX,
} OH_AI_DataType;

/**
 * @if Eng
 * @brief  Tensor handle
 * @else
 * @brief  张量句柄
 * @endif
 */
typedef void* OH_AI_TensorHandle;

/**
 * @if Eng
 * @brief  Tensor handle array
 * @else
 * @brief  张量句柄数组
 * @endif
 */
typedef struct OH_AI_TensorHandleArray {
    size_t handle_num;
    OH_AI_TensorHandle* handle_list;
} OH_AI_TensorHandleArray;

/**
 * @if Eng
 * @brief  Context handle
 * @else
 * @brief  上下文句柄
 * @endif
 */
typedef void* OH_AI_ContextHandle;

/**
 * @if Eng
 * @brief  Model handle
 * @else
 * @brief  模型句柄
 * @endif
 */
typedef void* OH_AI_ModelHandle;

/**
 * @if Eng
 * @brief  Initialize the AI module from a configuration file
 * @param  [in] config_file_path Define the configuration file path.
 * @retval OH_AI_Status
 * @else
 * @brief  从配置文件中初始化AI模块
 * @param  [in] config_file_path 定义配置文件的路径
 * @retval OH_AI_Status
 * @endif
 */
OH_AI_API OH_AI_Status OH_AI_InitFromFile(char* config_file_path);

/**
 * @if Eng
 * @brief  Initialize the AI module from configuration file buffer
 * @param  [in] config_data Define the buffer read from a configuration file.
 * @param  [in] data_size Define bytes number of configuration file buffer.
 * @retval OH_AI_Status
 * @else
 * @brief  从配置文件缓冲区初始化AI模块
 * @param  [in] config_data 定义从配置文件读取的缓冲区
 * @param  [in] data_size 定义配置文件缓冲区的字节数
 * @retval OH_AI_Status
 * @endif
 */
OH_AI_API OH_AI_Status OH_AI_Init(void* config_data, size_t data_size);

/**
 * @if Eng
 * @brief  Deinitialize the AI module
 * @retval OH_AI_Status
 * @else
 * @brief  去初始化AI模块
 * @retval OH_AI_Status
 */
OH_AI_API OH_AI_Status OH_AI_Deinit(void);

/**
 * @if Eng
 * @brief  Create a context object.
 * @retval Context object handle.
 * @else
 * @brief  创建一个上下文对象.
 * @retval 上下文对象句柄.
 * @endif
 */
OH_AI_API OH_AI_ContextHandle OH_AI_ContextCreate(void);

/**
 * @if Eng
 * @brief  Destroy the context object.
 * @param  [in] context Context object handle address.
 * @else
 * @brief  销毁上下文对象.
 * @param  [in] context 上下文对象指针.
 * @endif
 */
OH_AI_API void OH_AI_ContextDestroy(OH_AI_ContextHandle* context);

/**
 * @if Eng
 * @brief  Create a model object.
 * @retval Model object handle.
 * @else
 * @brief  创建一个模型对象.
 * @retval 模型对象句柄.
 * @endif
 */
OH_AI_API OH_AI_ModelHandle OH_AI_ModelCreate(void);

/**
 * @if Eng
 * @brief  Load and build the model from model path so that it can run on a device.
 * @param  [in] model Model object handle.
 * @param  [in] model_path Define the model file path.
 * @param  [in] model_context Define the context used to store options during execution.
 * @retval OH_AI_Status.
 * @else
 * @brief  从模型文件路径加载并构建模型，使其可以在设备上运行
 * @param  [in] model 模型对象句柄
 * @param  [in] model_path 定义模型文件路径
 * @param  [in] model_context 定义用于在执行期间存储选项的上下文句柄
 * @retval OH_AI_Status.
 * @endif
 */
OH_AI_API OH_AI_Status OH_AI_ModelBuildFromFile(
    OH_AI_ModelHandle model,
    const char* model_path,
    const OH_AI_ContextHandle model_context);

/**
 * @if Eng
 * @brief  Build the model from model file buffer so that it can run on a device.
 * @param  [in] model Model object handle.
 * @param  [in] model_data Define the buffer read from a model file.
 * @param  [in] data_size Define bytes number of model file buffer.
 * @param  [in] model_context Define the context used to store options during execution.
 * @retval OH_AI_Status.
 * @else
 * @brief  从模型文件缓冲区加载并构建模型，使其可以在设备上运行
 * @param  [in] model 模型对象句柄.
 * @param  [in] model_data 定义从模型文件读取的缓冲区
 * @param  [in] data_size 定义模型文件缓冲区的字节数
 * @param  [in] model_context 定义用于在执行期间存储选项的上下文句柄.
 * @retval OH_AI_Status.
 * @endif
 */
OH_AI_API OH_AI_Status OH_AI_ModelBuild(
    OH_AI_ModelHandle model,
    const void* model_data,
    size_t data_size,
    const OH_AI_ContextHandle model_context);

/**
 * @if Eng
 * @brief  Build the model from the name of the model so that it can run on a device.
 * @param  [in] model Model object handle.
 * @param  [in] model_name Define the name of the model.
 * @param  [in] model_context Define the context used to store options during execution.
 * @retval OH_AI_Status.
 * @else
 * @brief  依据模型名称构建模型，使其可以在设备上运行
 * @param  [in] model 模型对象句柄
 * @param  [in] model_name 定义从模型名称
 * @param  [in] model_context 定义用于在执行期间存储选项的上下文句柄
 * @retval OH_AI_Status.
 * @endif
 */
OH_AI_API OH_AI_Status OH_AI_ModelBuildFromName(
    OH_AI_ModelHandle model,
    const char* model_name,
    const OH_AI_ContextHandle model_context);

/**
 * @if Eng
 * @brief  Destroy the model object.
 * @param  [in] model Model object handle address.
 * @else
 * @brief  销毁模型对象
 * @param  [in] model 模型对象句柄指针
 * @endif
 */
OH_AI_API void OH_AI_ModelDestroy(OH_AI_ModelHandle* model);

/**
 * @if Eng
 * @brief  Obtains all input tensor handles of the model.
 * @param  [in] model Model object handle.
 * @retval The array that includes all input tensor handles.
 * @else
 * @brief  获取模型的所有输入张量句柄
 * @param  [in] model 模型对象句柄.
 * @retval 包含所有输入张量句柄的数组
 * @endif
 */
OH_AI_API OH_AI_TensorHandleArray OH_AI_ModelGetInputs(const OH_AI_ModelHandle model);

/**
 * @if Eng
 * @brief  Obtains all output tensor handles of the model.
 * @param  [in] model Model object handle.
 * @retval The array that includes all output tensor handles.
 * @else
 * @brief  获取模型的所有输出张量句柄
 * @param  [in] model 模型对象句柄
 * @retval 包含所有输出张量句柄的数组
 * @endif
 */
OH_AI_API OH_AI_TensorHandleArray OH_AI_ModelGetOutputs(const OH_AI_ModelHandle model);

/**
 * @if Eng
 * @brief  Inference model.
 * @param  [in] model Model object handle.
 * @param  [in] inputs The array that includes all input tensor handles.
 * @param  [in] outputs The array that includes all output tensor handles.
 * @retval OH_AI_Status.
 * @else
 * @brief  模型推理
 * @param  [in] model 模型对象句柄
 * @param  [in] inputs 包含所有输入张量句柄的数组
 * @param  [in] outputs 包含所有输出张量句柄的数组
 * @retval OH_AI_Status.
 * @endif
 */
OH_AI_API OH_AI_Status OH_AI_ModelPredict(
    OH_AI_ModelHandle model,
    const OH_AI_TensorHandleArray inputs,
    OH_AI_TensorHandleArray* outputs);

/**
 * @if Eng
 * @brief  Obtain the data size of the tensor.
 * @param  [in] tensor Tensor object handle.
 * @retval The data size of the tensor.
 * @else
 * @brief  获取张量的数据大小
 * @param  [in] tensor 张量对象句柄
 * @retval 张量的数据大小
 * @endif
 */
OH_AI_API size_t OH_AI_TensorGetDataSize(const OH_AI_TensorHandle tensor);

/**
 * @if Eng
 * @brief  Obtain the shape of the tensor.
 * @param  [in] tensor Tensor object handle.
 * @param  [out] shape_num Dimension of shape.
 * @retval The shape array of the tensor.
 * @else
 * @brief  获取张量的形状
 * @param  [in] tensor 张量对象的句柄
 * @param  [out] shape_num 张量形状的维度
 * @retval 张量的形状数组
 * @endif
 */
OH_AI_API const int64_t* OH_AI_TensorGetShape(const OH_AI_TensorHandle tensor, size_t* shape_num);


/**
 * @if Eng
 * @brief  Obtain the element number of the tensor.
 * @param  [in] tensor Tensor object handle.
 * @retval The element number of the tensor.
 * @else
 * @brief  获取张量中元素的个数
 * @param  [in] tensor 张量对象句柄
 * @retval 张量中元素的个数
 * @endif
 */
OH_AI_API int64_t OH_AI_TensorGetElementNum(const OH_AI_TensorHandle tensor);

/**
 * @if Eng
 * @brief  Obtain the data type of the tensor.
 * @param  [in] tensor Tensor object handle.
 * @retval The date type of the tensor.
 * @else
 * @brief  获取张量的数据类型
 * @param  [in] tensor 张量对象句柄
 * @retval 张量的数据类型
 * @endif
 */
OH_AI_API OH_AI_DataType OH_AI_TensorGetDataType(const OH_AI_TensorHandle tensor);

/**
 * @if Eng
 * @brief  Obtain the mutable data pointer of the tensor. If the internal data is empty, it will allocate memory.
 * @param  [in] tensor Tensor object handle.
 * @retval The data pointer of the tensor.
 * @else
 * @brief  获取张量的数据指针。如果内部数据为空，会分配内存
 * @param  [in] tensor 张量对象句柄.
 * @retval 张量的数据指针
 * @endif
 */
OH_AI_API void* OH_AI_TensorGetMutableData(const OH_AI_TensorHandle tensor);

/**
 * @if Eng
 * @brief  Obtain error code.
 * @retval Error code.
 * @else
 * @brief  获取详细错误码
 * @retval 详细错误码
 * @endif
 */
OH_AI_API int32_t OH_AI_GetErrorCode(void);

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif  /* __cplusplus */
#endif  /* __cplusplus */

#endif // AI_H_