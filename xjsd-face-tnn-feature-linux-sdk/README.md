# xjsd-face-tnn

这是人脸算法TNN部署工程，基于cmake构建跨平台编译

## 工程构建

计划支持windows、linux和android平台的构建部署
### windows平台构建
1. 安装cmake>3.4版本
2. 安装visual studio 或者 clion

```PowerShell
mkdir build/win32 
cmake ./ -B build/win32 -A Win32 -G "Visual Studio 15 2017"
cd build/win32
cmake --build ./
```
### android平台构建
1. 安装cmake>3.4版本
2. 安装 ndk>r21

```PowerShell
mkdir build/armeabi-v7a
python tools/android_cmake_build.py -n NDK_PATH -a armeabi-v7a -i ./ -o build/armeabi-v7a
cd build/armeabi-v7a
cmake --build ./
```

## 工程介绍
* **android** android人脸aar和arr测试demo工程
* **data**  测试数据存放目录
* **docs**  文档目录
* **include**   对外输出的lib库头文件地址
* **modules**   功能模块目录，目前有检测、比较、活检、人脸对齐和人脸跟踪模块
* **test**  功能和单元测试目录，每一个功能模块都要有测试程序
* **third_part**    第三方库 opencv、TNN、libyuv等    
* **utility**   基础工具库,文件操作、定时器、log等
* **tools**     脚本、工具目录
* 
## 开发流程
工程主体c++开发方案，以方便做到跨平台部署的抽象，一般的开发流程是:
1. 在windows或linux上开发功能模块和单元测试程序。
2. 功能验证完成以后编译成android可执行程序，进行性能测试。
3. 功能性能测试都完成以后开发jni接口和android、服务器适配sdk。

内部功能模块的mat、rect、point这些参数我们尽量使用opencv和TNN的类型传递


## 测试流程
cmake中定义了一个TEST_DATA_PATH的宏，是我们测试数据的存放路径，windows平台是当前工程的data目录地址。android平台为"./data"，所以需要把data目录复制手机中并且和可执行文件同级目录。
### windows平台测试
直接运行visual studio的工程就读取模型和文件进行测试

### android平台测试
* 进入cmake android工程目录
* 编译android可执行文件
* 复制可执行文件和动态库到手机/data/local/tmp路径下
* 复制data中需要模型和测试文件到/data/local/tmp路径下
* 添加执行目录到LD_LIBRARY_PATH
* 运行可执行文件

**编译可执行文件**
```PowerShell
cd build/armeabi-v7a
cmake --build ./
adb push YoloTest /data/local/tmp/
adb push ../../data /data/local/tmp/data
```

**运行可执行文件**
```PowerShell
adb shell
cd /data/local/tmp
export LD_LIBRARY_PATH=./:$LD_LIBRARY_PATH
chmod +x YoloTest
./YoloTest
```

**输出结果**
```PowerShell
yolo_test
img 1200,800
[I][2022-08-22 13:46:45.018][WZT_TNN        ][proto_path=./data/Yolo/yolov5s_sim_opt.tnnproto]
[I][2022-08-22 13:46:45.018][WZT_TNN        ][model_path=./data/Yolo/yolov5s_sim_opt.tnnmodel]
I/tnn: tnn::Status tnn::OpenCLRuntime::Init() [File source/tnn/device/opencl/opencl_runtime.cc][Line 120] OpenCL version: CL_TARGET_OPENCL_VERSION 200   CL_HPP_TARGET_OPENCL_VERSION 110   CL_HPP_MINIMUM_OPENCL_VERSION 110
I/tnn: tnn::Status tnn::OpenCLRuntime::Init() [File source/tnn/device/opencl/opencl_runtime.cc][Line 157] Create common opencl context
[I][2022-08-22 13:46:46.677][WZT_TNN        ][===============> 1 14 20 255]
[I][2022-08-22 13:46:46.678][WZT_TNN        ][===============> 1 28 40 255]
[I][2022-08-22 13:46:46.682][WZT_TNN        ][===============> 1 56 80 255]
[I][2022-08-22 13:46:46.683][WZT_TNN        ][object size:2]
0,0.867205,(534.637,155.769),(1115.48,669.231)
27,0.831663,(693.101,371.28),(743.091,542.56)

terminating with uncaught exception of type cv::Exception: OpenCV(4.1.1) D:/source/opencv/opencv4.1.1/sources/modules/highgui/src/window.cpp:627: error: (-2:Unspecified error) The function is not implemented. Rebuild the library with Windows, GTK+ 2.x or Cocoa support. If you are on Ubuntu or Debian, install libgtk2.0-dev and pkg-config, then re-run cmake or configure script in function 'cvShowImage'

/buildbot/src/android/ndk-release-r21/external/libcxx/../../external/libcxxabi/src/abort_message.cpp:72: abort_message: assertion "terminating with uncaught exception of type cv::Exception: OpenCV(4.1.1) D:/source/opencv/opencv4.1.1/sources/modules/highgui/src/window.cpp:627: error: (-2:Unspecified error) The function is not implemented. Rebuild the library with Windows, GTK+ 2.x or Cocoa support. If you are on Ubuntu or Debian, install libgtk2.0-dev and pkg-config, then re-run cmake or configure script in function 'cvShowImage'
" failed
Aborted
```
**因为adb命令下没有gui支持，所以opencv imshow测试程序会报错，只看log输出是否正常就可以了**