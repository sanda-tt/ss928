# HiSpark\_aiflylite介绍<a name="ZH-CN_TOPIC_0000001142448981"></a>

-   [简介](#section11660541593)
-   [目录](#section161941989596)
-   [使用说明](#section161941989598)
-   [构建说明](#section19369206113115)
-   [约束](#section119744591305)
-   [协议说明](#section1312121216216)
    -   [许可说明](#section129654513264)

-   [相关仓](#section1371113476307)

## 简介<a name="section11660541593"></a>

[开发板介绍](https://gitee.com/openharmony/docs/blob/master/zh-cn/device-dev/quick-start/quickstart-appendix-hi3516.md)

本目录主要用于放置hispark_aiflylite开发板相关内容，详情参见目录章节。

## 目录<a name="section161941989596"></a>

```
├── kernel    
│	├── BUILD.gn				#编译框架GN文件
│	├── kernel.mk				#用于配置内核编译交叉工具链、源码环境、defconfig等信息
│	└── kernel_module_build.sh	#内核编译入口shell脚本文件
├── linux
│   └── config.gni    #用于描述这个产品样例所使用的单板、内核、工具链、编译器等信息
```

## 使用说明<a name="section161941989598"></a>
1. 由于OpenHarmony工程的编译构建流程中会拷贝kernel/linux/linux-\*\.\*的代码至\$(OUT_DIR)/kernel/\${KERNEL_VERSION}后进行打补丁动作
    ```
    $(hide) rm -rf $(KERNEL_SRC_TMP_PATH);mkdir -p $(KERNEL_SRC_TMP_PATH);cp -arfL $(KERNEL_SRC_PATH)/* $(KERNEL_SRC_TMP_PATH)/
    ```

2. 合入HDF补丁
    按照drivers/hdf_core/adapter/khdf/linux/patch_hdf.sh中HDF补丁合入方法，应用kernel/linux/patches/\$(KERNEL_VERSION)/common_patch/hdf.patch，合入不同内核版本对应的HDF内核补丁

    ```
	$(hide) $(OHOS_BUILD_HOME)/drivers/hdf_core/adapter/khdf/linux/patch_hdf.sh $(OHOS_BUILD_HOME) $(KERNEL_SRC_TMP_PATH) $(KERNEL_PATCH_PATH) $(DEVICE_NAME)
	```
3. 若编译linux-6.6内核，则配套要修改bounds_checking_function/include/securec.h的stdarg.h
    ```
	ifeq ($(KERNEL_VERSION), linux-6.6)
	    sed -i 's/<stdarg.h>/<linux\/stdarg.h>/' $(KERNEL_SRC_TMP_PATH)/bounds_checking_function/include/securec.h
    endif
	```
4. 应用芯片特性的内核补丁和适配OpenHarmony内核的补丁，直接执行kernel/linux/patches/linux-6.6/\$(DEVICE_PATCH_DIR)/patch_$(DEVICE_NAME).sh即可
    ```
	$(hide) chmod 755 $(DEVICE_PATCH_DIR)/patch_$(DEVICE_NAME).sh
	$(hide) cd $(KERNEL_SRC_TMP_PATH);$(DEVICE_PATCH_DIR)/patch_$(DEVICE_NAME).sh $(DEVICE_PATCH_DIR)
    ```
5. 创建软链接到kernel/linux/common_modules目录下的软件源码
    ```
    $(hide) $(UNIFIED_COLLECTION_PATCH_FILE) $(OHOS_BUILD_HOME) $(KERNEL_SRC_TMP_PATH) $(DEVICE_NAME) $(KERNEL_VERSION)
    ...
    ```
6. 合并defconfig配置文件，将如下配置文件合并生成的defconfig拷贝至\${KERNEL_SRC_TMP_PATH}/arch/\$(KERNEL_ARCH)/configs/$(DEFCONFIG_FILE)使用，其中：  
    small_defconfig：小型系统常用配置文件  
    arm64_defconfig：芯片特性的内核配置文件  
    support_defconfig：适配OpenHarmony内核的专用配置文件
    base_defconfig：OpenHarmony特性依赖的内核必选模块以及安全红线特性等必须开启的配置项  
    ```
	$(hide) cp -rf $(KERNEL_CONFIG_PATH)/. $(KERNEL_SRC_TMP_PATH)/
	$(hide) bash $(KERNEL_SRC_TMP_PATH)/scripts/kconfig/merge_config.sh -O $(KERNEL_SRC_TMP_PATH)/arch/$(KERNEL_ARCH)/configs/ \
     -m $(KERNEL_SRC_TMP_PATH)/type/small_defconfig $(KERNEL_SRC_TMP_PATH)/$(DEVICE_NAME)/arch/arm64_defconfig \
     $(KERNEL_SRC_TMP_PATH)/$(DEVICE_NAME)/arch/support_defconfig $(KERNEL_SRC_TMP_PATH)/base_defconfig

	$(hide) mv ${KERNEL_SRC_TMP_PATH}/arch/$(KERNEL_ARCH)/configs/.config ${KERNEL_SRC_TMP_PATH}/arch/$(KERNEL_ARCH)/configs/$(DEFCONFIG_FILE)
	$(hide) $(KERNEL_MAKE) -C $(KERNEL_SRC_TMP_PATH) ARCH=$(KERNEL_ARCH) $(KERNEL_CROSS_COMPILE) distclean
	$(hide) $(KERNEL_MAKE) -C $(KERNEL_SRC_TMP_PATH) ARCH=$(KERNEL_ARCH) $(KERNEL_CROSS_COMPILE) $(DEFCONFIG_FILE)
    ```
## 构建说明<a name="section19369206113115"></a>

使用工程的全量编译命令，编译生成uImage内核镜像

```
./build.sh --product-name=ipcamera_hispark_aiflylite_linux --ccache --no-prebuilt-sdk --build-target build_kernel --gn-args linux_kernel_version=\"linux-6.6\"
```

```
--product-name ipcamera_hispark_aiflylite_linux  # 编译ipcamera_hispark_aiflylite_linux镜像
--build-target build_kernel                      # 编译hispark_aiflylite的uImage内核镜像
--gn-args linux_kernel_version=\"linux-6.6\"     # 编译指定内核版本，若不配置参数，则默认以config.json配置为准
--no-prebuilt-sdk                                # 跳过SDK子系统的编译
```

## 约束<a name="section119744591305"></a>

当前支持Hi3519AV200芯片。

## 协议说明<a name="section1312121216216"></a>

参见对应目录的LICENSE文件及代码声明


