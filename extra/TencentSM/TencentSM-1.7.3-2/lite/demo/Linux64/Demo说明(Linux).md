
## 编译TencentSM

执行lite/build/目录下的脚本，生成对应平台的库文件，支持生成静态库和动态库。
例如，执行sh ./release4Linux_x86_64.sh，就在/lite/release/linux64/Release下生成静态库libTencentSM.a和动态库libTencentSM.so。

## 国密库Demo（Linux64）

1. **静态库Demo**，进入lite/demo/Linux64/static_library_demo目录，输入make，就生成了sm_test。执行sm_test，就显示SM2、SM3、SM4各个接口的调用示例。
2. **动态库Demo**，进入lite/demo/Linux64/shared_library_demo目录，输入make，就生成了sm_test。执行sm_test，就显示SM2、SM3、SM4各个接口的调用示例。
