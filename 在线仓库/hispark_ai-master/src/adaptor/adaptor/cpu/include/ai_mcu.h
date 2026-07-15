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
#ifndef MS_INCLUDE_AI_MCU_H
#define MS_INCLUDE_AI_MCU_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @defgroup middleware_utils_ai_api AI
 * @ingroup middleware_utils
 * @{
 */

/**
 * @if Eng
 * @brief  default compiler feature type of AI API.
 * @else
 * @brief  AI接口默认编译器特性
 * @endif
 */
#ifndef MS_API
#ifdef _WIN32
#define MS_API __declspec(dllexport)
#else
#define MS_API __attribute__((visibility("default")))
#endif
#endif

/**
 * @if Eng
 * @brief  MS component type
 * @else
 * @brief  MS组件类型
 * @endif
 */
enum MSCompCode {
    kMSCompCodeCore = 0x00000000u,       /*!< @if Eng MS AICore type component.
                                              @else   MS AICore组件 @endif */
    kMSCompCodeLite = 0xF0000000u,       /*!< @if Eng  MS Lite type component.
                                              @else   MS Lite 组件 @endif */
};

/**
 * @if Eng
 * @brief  MS return status
 * @else
 * @brief  MS返回状态码
 * @endif
 */
typedef enum MSStatus {
    kMSStatusSuccess = 0,                                                   /*!< @if Eng MS success status.
                                                                                 @else   MS 成功状态 @endif */
    kMSStatusCoreFailed = kMSCompCodeCore | 0x1,                            /*!< @if Eng MS aicore failed.
                                                                                 @else   MS AICore组件失败 @endif */
    /* Common error code, range: [-1, -100) */
    kMSStatusLiteError = kMSCompCodeLite | (0x0FFFFFFF & -1),               /*!< @if Eng common error.
                                                                                 @else   组件通用运行错误 @endif */
    kMSStatusLiteNullptr = kMSCompCodeLite | (0x0FFFFFFF & -2),             /*!< @if Eng nullptr failed.
                                                                                 @else   空指针运行失败 @endif */
    kMSStatusLiteParamInvalid = kMSCompCodeLite | (0x0FFFFFFF & -3),        /*!< @if Eng no change failed.
                                                                                 @else   未修改失败 @endif */
    kMSStatusLiteNoChange = kMSCompCodeLite | (0x0FFFFFFF & -4),            /*!< @if Eng invalid param failed.
                                                                                 @else   非法参数失败 @endif */
    kMSStatusLiteSuccessExit = kMSCompCodeLite | (0x0FFFFFFF & -5),         /*!< @if Eng no error but exit.
                                                                                 @else   正常退出 @endif */
    kMSStatusLiteMemoryFailed = kMSCompCodeLite | (0x0FFFFFFF & -6),        /*!< @if Eng create mem failed.
                                                                                 @else   创建内存失败 @endif */
    kMSStatusLiteNotSupport = kMSCompCodeLite | (0x0FFFFFFF & -7),          /*!< @if Eng support failed.
                                                                                 @else   支持失败 @endif */
    kMSStatusLiteThreadPoolError = kMSCompCodeLite | (0x0FFFFFFF & -8),     /*!< @if Eng error occur in thread pool.
                                                                                 @else   线程池错误 @endif */
    kMSStatusLiteUninitializedObj = kMSCompCodeLite | (0x0FFFFFFF & -9),    /*!< @if Eng object is not initialized.
                                                                                 @else   对象未初始化 @endif */
    kMSStatusLiteFileError = kMSCompCodeLite | (0x0FFFFFFF & -10),          /*!< @if Eng invalid file failed.
                                                                                 @else   文件非法 @endif */
    kMSStatusLiteServiceDeny = kMSCompCodeLite | (0x0FFFFFFF & -11),        /*!< @if Eng denial of service.
                                                                                 @else   服务拒绝 @endif */
    kMSStatusLiteModelRebuild = kMSCompCodeLite | (0x0FFFFFFF & -12),       /*!< @if Eng model has been built.
                                                                                 @else   模型未构建失败 @endif */
    /* Executor error code, range: [-100,-200) */
    kMSStatusLiteOutOfTensorRange = kMSCompCodeLite | (0x0FFFFFFF & -100),  /*!< @if Eng failed to check range.
                                                                                 @else 张量越界 @endif */
    kMSStatusLiteInputTensorError = kMSCompCodeLite | (0x0FFFFFFF & -101),  /*!< @if Eng failed to check input tensor.
                                                                                 @else 检查输入张量失败 @endif */
    kMSStatusLiteReentrantError = kMSCompCodeLite | (0x0FFFFFFF & -102),    /*!< @if Eng exist executor running.
                                                                                 @else 系统已存在执行机正在执行中 @endif */
    /* Graph error code, range: [-200,-300) */
    kMSStatusLiteGraphFileError = kMSCompCodeLite | (0x0FFFFFFF & -200),    /*!< @if Eng failed to verify graph file.
                                                                                 @else 检查图文件失败 @endif */
    /* Node error code, range: [-300,-400) */
    kMSStatusLiteNotFindOp = kMSCompCodeLite | (0x0FFFFFFF & -300),         /*!< @if Eng failed to find operator.
                                                                                 @else 算子未找到 @endif */
    kMSStatusLiteInvalidOpName = kMSCompCodeLite | (0x0FFFFFFF & -301),     /*!< @if Eng invalid operator name.
                                                                                 @else 算子名称非法 @endif */
    kMSStatusLiteInvalidOpAttr = kMSCompCodeLite | (0x0FFFFFFF & -302),     /*!< @if Eng invalid operator attr.
                                                                                 @else 算子属性非法 @endif */
    kMSStatusLiteOpExecuteFailure = kMSCompCodeLite | (0x0FFFFFFF & -303),  /*!< @if Eng execution operator failed.
                                                                                 @else 算子执行 @endif */
    /* Tensor error code, range: [-400,-500) */
    kMSStatusLiteFormatError = kMSCompCodeLite | (0x0FFFFFFF & -400),       /*!< @if Eng check tensor format fail.
                                                                                 @else 检查张量格式失败 @endif */
    /* InferShape error code, range: [-500,-600) */
    kMSStatusLiteInferError = kMSCompCodeLite | (0x0FFFFFFF & -500),        /*!< @if Eng failed to infer shape.
                                                                                 @else 推理Shape失败 @endif */
    kMSStatusLiteInferInvalid = kMSCompCodeLite | (0x0FFFFFFF & -501),      /*!< @if Eng invalid pre-infer shape.
                                                                                 @else 执行前已shape推理 @endif */
    /* User input param error code, range: [-600, 700) */
    kMSStatusLiteInputParamInvalid = kMSCompCodeLite | (0x0FFFFFFF & -600)  /*!< @if Eng invalid input param by user.
                                                                                 @else 用户输入参数非法 @endif */
} MSStatus;

/**
 * @if Eng
 * @brief  MS data format
 * @else
 * @brief  MS 数据格式
 * @endif
 */
typedef enum MSFormat {
    kMSFormatNCHW = 0,    /*!< @if Eng HCHW data format.
                               @else   HCHW格式排布. @endif */
    kMSFormatNHWC = 1,    /*!< @if Eng NHWC data format.
                               @else   NHWC格式排布. @endif */
    kMSFormatNHWC4 = 2,   /*!< @if Eng NHWC4 data format.
                               @else   NHWC4格式排布. @endif */
    kMSFormatHWKC = 3,    /*!< @if Eng HWKC data format.
                               @else   HWKC格式排布. @endif */
    kMSFormatHWCK = 4,    /*!< @if Eng HWCK data format.
                               @else   HWCK格式排布. @endif */
    kMSFormatKCHW = 5,    /*!< @if Eng KCHW data format.
                               @else   KCHW格式排布. @endif */
    kMSFormatCKHW = 6,    /*!< @if Eng CKHW data format.
                               @else   CKHW格式排布. @endif */
    kMSFormatKHWC = 7,    /*!< @if Eng KHWC data format.
                               @else   KHWC格式排布. @endif */
    kMSFormatCHWK = 8,    /*!< @if Eng CHWK data format.
                               @else   CHWK格式排布. @endif */
    kMSFormatHW = 9,      /*!< @if Eng HW data format.
                               @else   HW格式排布. @endif */
    kMSFormatHW4 = 10,    /*!< @if Eng HW4 data format.
                               @else   HW4格式排布. @endif */
    kMSFormatNC = 11,     /*!< @if Eng NC data format.
                               @else   NC格式排布. @endif */
    kMSFormatNC4 = 12,    /*!< @if Eng NC4 data format.
                               @else   NC4格式排布. @endif */
    kMSFormatNC4HW4 = 13, /*!< @if Eng NC4HW4 data format.
                               @else   NC4HW4格式排布. @endif */
    kMSFormatNCDHW = 15,  /*!< @if Eng NCDHW data format.
                               @else   NCDHW格式排布. @endif */
    kMSFormatNWC = 16,    /*!< @if Eng NWC data format.
                               @else   NWC格式排布. @endif */
    kMSFormatNCW = 17     /*!< @if Eng NCW data format.
                               @else   NCW格式排布. @endif */
} MSFormat;

/**
 * @if Eng
 * @brief  MS model type
 * @else
 * @brief  MS 模型格式
 * @endif
 */
typedef enum MSModelType {
    kMSModelTypeMindIR = 0,          /*!< @if Eng MindIR model type.
                                          @else   MindIR类型. @endif */
    kMSModelTypeInvalid = 0xFFFFFFFF /*!< @if Eng invalid model type.
                                          @else   模型类型非法. @endif */
} MSModelType;

/**
 * @if Eng
 * @brief  MS data type
 * @else
 * @brief  MS 数据类型
 * @endif
 */
typedef enum MSDataType {
    kMSDataTypeUnknown = 0,            /*!< @if Eng data unknown .
                                            @else   MindIR类型. @endif */
    kMSDataTypeObjectTypeString = 12,  /*!< @if Eng string type data.
                                            @else   string类型数据 @endif */
    kMSDataTypeObjectTypeList = 13,    /*!< @if Eng list type data.
                                            @else   列表类型数据. @endif */
    kMSDataTypeObjectTypeTuple = 14,   /*!< @if Eng tuple type data.
                                            @else   元组类型数据. @endif */
    kMSDataTypeObjectTypeTensor = 17,  /*!< @if Eng tensor type data.
                                            @else   张量类型数据. @endif */
    kMSDataTypeNumberTypeBegin = 29,   /*!< @if Eng begin type data.
                                            @else   Begin类型数据. @endif */
    kMSDataTypeNumberTypeBool = 30,    /*!< @if Eng bool type data.
                                            @else   bool类型数据. @endif */
    kMSDataTypeNumberTypeInt8 = 32,    /*!< @if Eng int8 type data.
                                            @else   int8类型数据. @endif */
    kMSDataTypeNumberTypeInt16 = 33,   /*!< @if Eng int16 type data.
                                            @else   int16类型数据. @endif */
    kMSDataTypeNumberTypeInt32 = 34,   /*!< @if Eng int32 type data.
                                            @else   int32类型数据. @endif */
    kMSDataTypeNumberTypeInt64 = 35,   /*!< @if Eng int64 type data.
                                            @else   int64类型数据. @endif */
    kMSDataTypeNumberTypeUInt8 = 37,   /*!< @if Eng uint8 type data.
                                            @else   uint8类型数据. @endif */
    kMSDataTypeNumberTypeUInt16 = 38,  /*!< @if Eng uint16 type data.
                                            @else   uint16类型数据. @endif */
    kMSDataTypeNumberTypeUInt32 = 39,  /*!< @if Eng uint32 type data.
                                            @else   uint32类型数据. @endif */
    kMSDataTypeNumberTypeUInt64 = 40,  /*!< @if Eng uint64 type data.
                                            @else   uint64类型数据. @endif */
    kMSDataTypeNumberTypeFloat16 = 42, /*!< @if Eng float16 type data.
                                            @else   float16类型数据. @endif */
    kMSDataTypeNumberTypeFloat32 = 43, /*!< @if Eng float32 type data.
                                            @else   float32类型数据. @endif */
    kMSDataTypeNumberTypeFloat64 = 44, /*!< @if Eng float64 type data.
                                            @else   float64类型数据. @endif */
    kMSDataTypeNumberTypeEnd = 46,     /*!< @if Eng end type data.
                                            @else   end类型数据. @endif */
    kMSDataTypeInvalid = INT32_MAX,    /*!< @if Eng invalid data type.
                                            @else   数据类型非法. @endif */
} MSDataType;

/**
 * @if Eng
 * @brief  MS model handle
 * @else
 * @brief  MS 模型对象
 * @endif
 */
typedef void *MSModelHandle;

/**
 * @if Eng
 * @brief  MS context handle
 * @else
 * @brief  MS 上下文对象
 * @endif
 */
typedef void *MSContextHandle;

/**
 * @if Eng
 * @brief  MS tensor handle
 * @else
 * @brief  MS 张量对象
 * @endif
 */
typedef void *MSTensorHandle;

/**
 * @if Eng
 * @brief  MS tensor handle array
 * @else
 * @brief  MS 张量对象数组
 * @endif
 */
typedef struct MSTensorHandleArray {
    size_t handle_num;
    MSTensorHandle *handle_list;
} MSTensorHandleArray;

/**
 * @if Eng
 * @brief  MS max shape dim num
 * @else
 * @brief  MS 最大shape支持维度数量
 * @endif
 */
#define MS_MAX_SHAPE_NUM 32

/**
 * @if Eng
 * @brief  MS tensor handle array
 * @else
 * @brief  MS 数据形状信息
 * @endif
 */
typedef struct MSShapeInfo {
    size_t shape_num;
    int64_t shape[MS_MAX_SHAPE_NUM];
} MSShapeInfo;

/**
 * @if Eng
 * @brief  MS callback param
 * @else
 * @brief  MS 回调函数参数 (Micro不支持)
 * @endif
 */
typedef struct MSCallBackParamC {
    char *node_name;
    char *node_type;
} MSCallBackParamC;

/**
 * @if Eng
 * @brief  MS callback function
 * @else
 * @brief  MS 回调函数
 * @endif
 */
typedef bool (*MSKernelCallBackC)(const MSTensorHandleArray inputs, const MSTensorHandleArray outputs,
    const MSCallBackParamC kernel_Info);

/**
 * @if Eng
 * @brief  Create a model object.
 * @retval MSModelHandle Object     Success.
 * @retval NULL                     Nullptr Failed.
 * @else
 * @brief  创建一个模型对象。
 * @retval MSModelHandle Object     成功返回#MSModelHandle。
 * @retval NULL                     失败返回#空指针。
 * @endif
 */
MS_API MSModelHandle MSModelCreate(void);

/**
 * @if Eng
 * @brief  Destroy a model object.
 * @param  [in]  model              input MSModelHandle object to destroy.
 * @retval void                     void Return.
 * @else
 * @brief  销毁一个模型对象。
 * @param  [in]  model              待销毁的MSModelHandle对象.
 * @retval void                     无对应返回值#void
 * @endif
 */
MS_API void MSModelDestroy(MSModelHandle *model);

/**
 * @if Eng
 * @brief  Create a context object.
 * @retval MSContextHandle Object    Success.
 * @retval NULL                     Nullptr Failed.
 * @else
 * @brief  创建一个上下文对象。
 * @retval MSContextHandle Object   成功返回#MSContextHandle对象。
 * @retval NULL                     失败返回#空指针。
 * @endif
 */
MS_API MSContextHandle MSContextCreate(void);

/**
 * @if Eng
 * @brief  Destroy the context object.
 * @param  [in]  context            input MSContextHandle object to destroy.
 * @retval void                     void Return.
 * @else
 * @brief  销毁一个上下文对象。
 * @param  [in]  context              待销毁的MSContextHandle对象.
 * @retval void                       无对应返回值#void
 * @endif
 */
MS_API void MSContextDestroy(MSContextHandle *context);

/**
 * @if Eng
 * @brief  Obtain all input tensor handles of the model.
 * @param  [in]  model                   Model object handle.
 * @retval MSTensorHandleArray Object    Success.
 * @retval NULL                          Nullptr Failed.
 * @else
 * @brief  获取模型对应的输入张量对象列表。
 * @param  [in]  model                    输入的模型对象。
 * @retval MSTensorHandleArray对象        成功返回#MSTensorHandleArray对象列表。
 * @retval NULL                           失败返回#空指针。
 * @endif
 */
MS_API MSTensorHandleArray MSModelGetInputs(const MSModelHandle model);

/**
 * @if Eng
 * @brief  Obtain all output tensor handles of the model.
 * @param  [in]  model                   Model object handle.
 * @retval MSTensorHandleArray Object    Success.
 * @retval NULL                          Nullptr Failed.
 * @else
 * @brief  获取模型对应的输出张量对象列表。
 * @param  [in]  model                    输入的模型对象。
 * @retval MSTensorHandleArray对象        成功返回#MSTensorHandleArray对象列表。
 * @retval NULL                           失败返回#空指针。
 * @endif
 */
MS_API MSTensorHandleArray MSModelGetOutputs(const MSModelHandle model);

/**
 * @if Eng
 * @brief  Obtain the data size of the tensor.
 * @param  [in]  tensor                   Model object handle.
 * @retval data size                   Success.
 * @else
 * @brief  表示获得Tensor的数据大小。
 * @param  [in]  tensor                    输入的张量对象。
 * @retval 张量对应的数据大小               成功返回#数据大小。
 * @endif
 */
MS_API size_t MSTensorGetDataSize(const MSTensorHandle tensor);


MS_API const int64_t *MSTensorGetShape(const MSTensorHandle tensor, size_t *shape_num);

/**
 * @if Eng
 * @brief  Obtain the element number of the tensor.
 * @param  [in]  tensor                   Model object handle.
 * @retval element num                   Success.
 * @else
 * @brief  创建一个输出张量对象列表。
 * @param  [in]  tensor                    输入的张量对象。
 * @retval 张量对应的元素个数               成功返回#元素个数。
 * @endif
 */
MS_API int64_t MSTensorGetElementNum(const MSTensorHandle tensor);

/**
 * @if Eng
 * @brief  Obtain the data type of the tensor.
 * @param  [in]  tensor                   Model object handle.
 * @retval data type                   Success.
 * @else
 * @brief  表示获得Tensor的数据类型。
 * @param  [in]  tensor                    输入的张量对象。
 * @retval 张量对应的数据类型               成功返回#数据类型。
 * @endif
 */
MS_API MSDataType MSTensorGetDataType(const MSTensorHandle tensor);

/**
 * @if Eng
 * @brief  Obtain the mutable data pointer of the tensor. If the internal data is empty, it will allocate memory.
 * @param  [in]  tensor                   Model object handle.
 * @retval void*                   Success.
 * @retval NULL                    Nullptr Failed.
 * @else
 * @brief  获取张量数据指针。如果数据为空，也会分配内存。
 * @param  [in]  tensor                    输入的张量对象。
 * @retval void* ptr              成功返回#void类型的指针。
 * @retval NULL                   失败返回#空指针。
 * @endif
 */
MS_API void *MSTensorGetMutableData(const MSTensorHandle tensor);

/**
 * @if Eng
 * @brief  Build the model from model file buffer so that it can run on a device.
 * @param  [in]  model                    Model object handle.
 * @param  [in]  model_data               Define the buffer read from a model file.
 * @param  [in]  data_size                Define bytes number of model file buffer.
 * @param  [in]  model_type               Define The type of model file.
 * @param  [in]  model_context            Define the context used to store options during execution.
 * @retval kMSStatusSuccess        Success.
 * @retval other                   Failed.
 * @else
 * @brief  创建一个模型对象。
 * @param  [in]  model                    模型对象.
 * @param  [in]  model_data               模型数据只读数组.
 * @param  [in]  data_size                模型数据大小.
 * @param  [in]  model_type               模型类型.
 * @param  [in]  model_context            对应的上下文对象用于存储选项.
 * @retval kMSStatusSuccess        成功返回#kMSStatusSuccess状态。
 * @retval 其他                    失败返回#其他状态码。
 * @endif
 */
MS_API MSStatus MSModelBuild(MSModelHandle model, const void *model_data, size_t data_size, MSModelType model_type,
    const MSContextHandle model_context);

/**
 * @if Eng
 * @brief  Inference model.
 * @param  [in]  model                    Model object handle.
 * @param  [in]  inputs                   The array that includes all input tensor handles.
 * @param  [in]  outputs                  The array that includes all output tensor handles.
 * @param  [in]  before                   CallBack before predict.
 * @param  [in]  after                    CallBack after predict.
 * @retval kMSStatusSuccess               Success.
 * @retval other                          Failed.
 * @else
 * @brief  完成模型推理。
 * @param  [in]  model                    模型对象
 * @param  [in]  inputs                   输入张量对象的列表
 * @param  [in]  outputs                  输出张量对象的列表
 * @param  [in]  before                   推理之后的回调函数
 * @param  [in]  after                    推理之前的回调函数
 * @retval kMSStatusSuccess         成功返回#kMSStatusSuccess状态。
 * @retval 其他                     失败返回#其他状态码。
 * @endif
 */
MS_API MSStatus MSModelPredict(MSModelHandle model, const MSTensorHandleArray inputs, MSTensorHandleArray *outputs,
    const MSKernelCallBackC before, const MSKernelCallBackC after);

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* MS_INCLUDE_AI_MCU_H */
