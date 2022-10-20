# 项目简介

## 背景
国密算法是国家密码管理局公布的国产密码算法标准，包括了对称加密，哈希算法，非对称加密，签名验签以及密钥交换算法。随后国家在各个领域推进国密算法的使用，并且出台了多项政策，推动国密算法在各个行业中的应用。目前各行业已经陆续开始使用国密算法，包括公安部、运输部、教育部，以及各大银行，安全厂商等都已启动。公司内部的工作中，也遇到了很多使用国密算法的场景；此外，随着《中华人民共和国网络安全法》的实施，在国家等保2.0标准推进中，国密算法将具备更加广泛的应用场景，国密库的使用也能让产品的满足政府合规性的要求。在此背景下，TencentSM国密库应运而生。

## 为什么自研？
随着密码算法国产化的推进，越来越多的团队需要接入使用国密库，然而在实际工作中，业界并没有一个严格遵循国密标准且易于接入的第三方库。从最初的国密库先行者GmSSL到最新的OpenSSL1.1.1虽然都已经实现了对国密库的支持，但是并没有提供一套易于接入的接口，在业务团队需要使用加解密功能时，往往学习和接入成本较高。另一方面，GmSSL作为OpenSSL的一个分支版本，它仍然具有OpenSSL的一些弊端，例如OpenSSL版本间的兼容性不足、不能平滑升级、API稳定性差以及安装包增量大等问题。在此背景下，我们团队自研了TencentSM国密库。TencentSM严格遵循密标委的国密标准，同时具有目前业界现存方案没有的优点，如性能优秀、兼容性友好、接口简化易于接入、安装包小等。

## 关于TencentSM
- TencentSM是TSM OTEAM开源协同团队自主研发的跨平台国密算法库。目标是为公司提供统一的国密解决方案，助力业务国密改造和密码合规要求。
- 目前拥有两个版本：内部开源版本与商密认证版本。其中商密认证版本在内部开源版本的基础上做了大量的优化和安全加固工作，具有优异的性能，尤其在信创环境上具有绝对优势。同时新增了国密证书与PKI体系、SM9等算法能力，并且取得到国家密码管理局的商用密码认证，可以满足强监管行业密码合规的监管要求。

**TencentSM支持的主要密码功能包括：**

- SM2：密钥对生成、加密、解密、签名、验签
- SM3：消息摘要（即，哈希算法）、消息验证码
- SM4：加密、解密(支持ECB/CBC/GCM模式)

**TencentSM密码算法实现遵循的标准包括：**

- GM/T0003-2012《SM2椭圆曲线公钥密码算法》
- GM/T0009-2012《SM2密码算法使用规范》
- GM/T0004-2012《SM3密码杂凑算法》
- GM/T0002-2012《SM4分组密码算法》
- NIST Special Publication 800-38D《Recommendation for Block. Cipher Modes of Operation: Galois/Counter Mode (GCM) and GMAC》

**TencentSM支持的接口语言包括：**

- C/C++
- Java
- Go
- Javascript

**TencentSM支持的运行环境包括：**

- Linux
- MacOS
- Windows
- iOS
- Android
- RTOS

## 商密认证版本

TencentSM提供商密认证版本，但由于国家密码管理局商用密码行业的监管及其他知识产权原因，目前商密认证版本并未开放源代码，如需使用商密认证版本，可联系接口人sunnycxie/medivhwu

### 商密认证版本有什么不同？

- **合规优势**：
商密认证版本正在各个平台陆续申请国家密码管理局颁发的商用密码产品认证，目前已获得移动终端(iOS/Android)的商用密码产品认证，服务端的商用密码产品认证预计2021上半年内可获得审批，目前TencentSM商密认证版本是腾讯唯一获得商用密码产品认证的密码模块。

- **性能优势**：
商密认证版本在SM2/SM3/SM4算法上，进行了多方面的性能优化，其中SM2的综合性能与开源版本相比有较大提升，SM4综合性能也有2~3倍的提升，且商密认证版本在信创环境，如linux arm64国产CPU上进行了针对性的优化，性能表现遥遥领先业界其他同类产品。

- **其他特性支持**：
商密认证版本在国密密钥交换、SM4白盒加密、国密证书及PKI体系相关功能上提供了额外接口支持。

## 开源版本

TencentSM提供公司内部开源版本

### 使用须知

- git仓库地址为：http://git.code.oa.com/SM-OpenSource/TencentSM.git

- 使用开源版本请在在线文档进行登记，以便开源协同团队了解业务接入方信息，登记地址：https://docs.qq.com/sheet/DV0J2UGJ5YXJZc2pG?tab=BB08J2

- 该版本为公司内部开源版本，并未对外开源，请勿对外公开源代码

- 该版本如需向公司外提供不含源代码的发布包，请与开源协同团队接口人medivhwu联系确认

### 如何编译

如需自行编译，可参考以下编译流程

#### Linux平台编译
> Step1:cd ./TencentSM/lite/build目录下

> Step2:sh ./build4Linux.sh

> Step3:根据提示输入static或者shared来编译静态库或者动态库

> Step4:根据控制台输出目录，获得.a文件或者.so文件

#### MacOS平台编译

> Step1:cd ./TencentSM/lite/build目录下

> Step2:sh ./build4MacOS.sh

> Step3:根据提示输入static或者shared来编译静态库或者动态库

> Step4:根据控制台输出目录，获得.a文件或者.dylib文件

### iOS平台编译

> Step1:cd ./TencentSM/lite/build目录下

> Step2:sh ./build4iOS.sh

> Step3:等待编译完成，根据控制台信息，定位到输出目录，获得不同架构下的.a以及universal目录下的全架构.a

#### Android平台编译

> Step1:cd ./TencentSM/lite/build目录下

> Step2:修改buildcmake.bat脚本中的ndk路径

> Step3:执行buildcmake.bat编译脚本

#### JNA接口编译

- JNA接口位于lite/jna目录下，是一个maven工程
- 在JNA接口编译前，可先编译相应平台架构下的so，以获得最新版本并替换jna目录中test/main子目录中的resources

> Step1:cd ./TencentSM/lite/jna目录下

> Step2:mvn clean

> Step3:mvn package即可完成jar打包

#### Go接口编译

- Go接口位于lite/go目录下
- 在Go接口编译前，可先编译相应平台架构下的.a，以获得最新版本

> Step1:cd ./TencentSM/lite/build

> Step2:sh sh release4Golang.sh x （x为版本子编号，可为任意数字或字符串）

> Step3:在/TencentSM/lite/release/go下获取最新包

其他平台可参考linux64的Makefile。如需使用动态链接so或改动库、投文件的存储位置，则需要相应地改动build.go文件头

#### Windows平台编译

- 安装msys2及mingw64工具链
> 1. 下载msys2并安装
> 2. 在msys2 shell中运行：
  pacman -Syu(安装完后强制结束)
  pacman -Syu
  pacman -S mingw-w64-i686-toolchain
  pacman -S mingw-w64-x86_64-toolchain
> 3. 把mingw64和mingw32目录下的mingw32-make.exe命令复制一份到同一个目录，命名为make.exe。

- 在msys2环境编译
> 1. 打开相应的MSYS2 MINGW环境shell(在MSYS2 MINGW64环境编译的是win64，在MSYS2 MINGW32环境编译的是win32)
> 2. 为了方便， 第一次需要用mount命令把工程目录映射到用户~目录下：
     mount D:\msys_project ~/project
    也可以在~/目录下自己创建或拷贝过来
> 3.  用env命令检查下path中是否有cmake路径，没有的话加一下:
    export PATH=$PATH:'/E/Program Files/CMake/bin'
> 4.  到lite/build目录下运行./buildwin.sh
> 5. 文件生成到lite/release/win目录下

### 如何调用

- 请参考《算法接口使用说明.md》

## 在tlinux上快速使用

目前TencentSM已集成到tlinux系统，可实现开箱即用，使用步骤如下：

##### 1.安装TencentSM包
sudo yum makecache && sudo yum install -y tencentsm

##### 2.业务在xxxx.c调用TencentSM接口
- 包含头文件 #include <tencentsm/sm.h>
- 若以动态库方式调用TencentSM，示例：
 gcc -o test xxxx.c -lTencentSM
- 若以静态库方式调用TencentSM，示例：
 gcc -o test xxxx.c /lib64/libTencentSM.a
