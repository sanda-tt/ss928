一、与PQT版本包位置对应关系
1、将版本包中的configs文件夹，config.cfg、Makefile、readme.txt、test_control.c、test_main.c拷贝到smp/a55_linux/mpp/sample/pqtool目录下
2、将版本包中release目录下的lib_pqcontrol.a、lib_pqstream.a拷贝到smp/a55_linux/mpp/out/lib目录下

二、与PQT版本包有差异的文件
1、Makefile中SRC_DIR、COMMON_INCLUDE变量和lib_pqcontrol.a、lib_pqstream.a的链接方式有差异
2、readme.txt中第3步拷贝有差异
3、test_control.c、test_main.c文件注释有差异

三、使用方式
进到sample目录
1. 按照实际的 编译环境 修改 第 1 行 SDK_DIR。
2. 执行make 编译生成 test_pqt(这个可执行程序名称可按照需要进行修改)。
3. 将test_pqt、config.cfg、configs拷贝至板端，同级放置。将sdk out目录下的lib改名为libs拷贝至板端，同级放置。
4. 打开config.cfg文件，修改UseLibrary下AE、AWB参数，确认设置的路径在test_pqt程序运行时能够访问。
5. 加载ko和设置访问SDK动态库的环境变量命令如下：
./load_ss928v100_user -i

export LD_LIBRARY_PATH=./libs/:$LD
export LD_LIBRARY_PATH=./libs/:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=./libs/extdrv:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=./libs/svp_npu:$LD_LIBRARY_PATH
export LD_PRELOAD=./libs/libss_ae.so:./libs/libss_awb.so:./libs/libot_mpi_isp.so:./libs/libsecurec.so
6. 运行程序：./test_pqt
注意：跑哪个ini业务在test_main.c中改参数"hy_s0603", "0"
如果仅需要跑control业务，编译时不需要链接lib_pqstream.a，可参考test_control.c适配