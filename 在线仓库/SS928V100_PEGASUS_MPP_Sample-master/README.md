# SS928V100_PEGASUS_MPP_Sample

### 背景：

​	为降低代码维护复杂度，将SS928V100 SDK中的MPP Sample代码提取出来，单独进行维护，既方便客户熟悉代码，也方便集成代码到客户工程。

### 1.为减少代码仓大小，已将相关测试源文件删除，可从SDK中获取。

SDK目录：

```
SDK对应目录ss928v100_gcc/smp/a55_linux/mpp/sample/
```

涉及删除的文件和目录：

```
src/svp/dpu/data目录
src/svp/ive/data目录
src/svp/svp_npu/data目录
src/region/res目录
src/vgs/data目录
src/tde/res目录
src/hdmi/source_file目录
src/vdec/source_file目录
src/gfbg/res目录
```

### 2. Sample 说明