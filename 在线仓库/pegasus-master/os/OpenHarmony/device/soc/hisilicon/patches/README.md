# OpenHarmony通过打patch方式实现产品特性化修改

   -   [简介](#section01)
   -   [patch文件配置](#section02)
   -   [patch制作](#section03)

## 简介<a name="section01"></a>
基于OpenHarmony-v5.1.0-Release tag版本，在Hi3403V100和Hi3519AV200芯片平台上适配OpenHarmony，对OH原生代码的定制化化修改、由于无法上社区，因此采取打patch形式来处理。

## patch文件配置<a name="section02"></a>
OpenHarmony的编译构建子系统提供先打入patch、再开始编译构建的方案，产品编译打入patch，详情可参考build/hb/util/prebuild/patch_process.py，只需配置好产品的patch.yml文件。

添加os/OpenHarmony/vendor/hisilicon/hispark_aifly_linux/patch.yml和os/OpenHarmony/vendor/hisilicon/hispark_aiflylite_linux/patch.yml，文件中配置需要打入的patch，编译时会在vendor下加载读取该文件。配置格式如下：
```
OpenHarmony源码代码仓相对路径：
  - 对OpenHarmony源码定制化修改patch相对路径

applications/sample/camera/media:
applications/sample/camera/media:
  - device/soc/hisilicon/patches/applications/sample/camera/media/camera_media_001.patch
  - device/soc/hisilicon/patches/applications/sample/camera/media/camera_media_002.patch
  - ../../vendor/rkh/patches/applications/sample/camera/media/camera_media_003.patch
  - ../../vendor/rkh/patches/applications/sample/camera/media/camera_media_004.patch
......
```
>一个OpenHarmony源码代码仓下可以添加多个patch文件，不需要的patch可以注释或者删除掉

## patch制作<a name="section02"></a>
使用git比较OpenHarmony源码仓与Hispark_aifly产品定制化修改源码仓得到差异patch文件。

提供os/OpenHarmony/device/soc/hisilicon/patches/make_linux_patch.sh脚本文件

执行下述命令，即可完成对applications/sample/camera/media源代码仓对比差异，生成camera_media_001.patch文件
```
sh -x ./make_linux_patch.sh /pathto/OpenHarmony-v5.1.0-Release/OpenHarmony/applications/sample/camera/media /pathto/sig/master/test/applications/sample/camera/media  camera_media_001.patch
```
参数说明：
```
参数1：/pathto/OpenHarmony-v5.1.0-Release/OpenHarmony/applications/sample/camera/media  OH原生代码仓绝对路径
参数2：/pathto/sig/master/test/applications/sample/camera/media 产品化特性基于OH原生代码仓定制化修改的绝对路径
参数3：git对比生成差异后的patch文件名称
```

>单个patch文件不宜过大、若涉及改动量大，可以按照目录拆分多个patch文件；建议二进制和源码文件拆分开

>放置对开源组件定制化修改补丁
海思集成的补丁文件放置在os/OpenHarmony/device/soc/hisilicon/patches路径下
润开鸿集成的补丁文件放置在vendor/rkh/patches路径下
>配置patch.yml文件，指定需要打入的patch
Hi3403V100芯片平台的补丁放置在os/OpenHarmony/vendor/hisilicon/hispark_aifly_linux/patch.yml
Hi3519AV200芯片平台的补丁放置在os/OpenHarmony/vendor/hisilicon/hispark_aiflylite_linux/patch.yml