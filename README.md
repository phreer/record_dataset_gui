# 人体运动数据采集界面程序
本应用以 Qt5 为平台, 用于采集人体手势数据. 调用的外设有普通摄像头, Intel Realsense F200, 基于 Android 的智能手表, Myo.

## 环境
Windows 10 64bit
Qt Creator 4.5.2
Qt5
Intel Realsense DCM F200 1.3.27.52404
Intel Realsense SDK
Android Studio 2.3
Android SDK API Level 25

Realsense SDK 可以在[这个地址](https://pan.baidu.com/s/1ufNhnLiya_17Mp17_p514w)下载.
OpenCV 可以在[这个地址](https://pan.baidu.com/s/11nCLgG5aUTlNQXdQmWc2DA)下载.
myo的SDK可以在[这个地址](https://developer.thalmic.com/downloads)
关于 OpenCV 和 Realsense SDK 的配置可以参考[这篇文章]()
关于Myo的API Reference 可以参照[这个地址](https://developer.thalmic.com/docs/api_reference/platform/index.html)

## 关于 smart watch
我们所使用的 smart watch 基于 Android 平台, 因此采用 WiFi 进行通信. 基本的思路类似于 FTP, 使用两个套接字, 一个用于发送控制命令(开始记录, 结束记录, 传输数据等), PC 作为 Client, smart watch 为 Server. 另一个用于接收数据, PC 为 Server 接受 smart watch 的请求, 该部分使用一个单独的线程一直运行.

## 文件结构
- **record_v0_1.pro** 包含 include path, lib path, 以配置 winsock, opencv, realsense sdk等.
- **mainWindow** 统筹各个部分, 按下 START 键时开始记录, 按下 STOP 键停止记录, 并接受 smart watch 的数据.
- **tcp_controller** 包含 winsock2 网络编程需要的头文件, 声明数据结构, 缓冲区等.
- **tcp_reciever** 包含一个线程类(继承自 QThread), 用于启动一个接收套接字, 接收来自 smart watch 的文件.

## 其他说明
网络编程部分使用 winsock2 api, 可能需要安装 Visual Studio 才能使用, 我不太确定, 我使用的是 VS2015.
其实 Qt 本身提供了网络编程 api, 或许更容易使用, 但因为我之前是在 VS 上面写的, 所以用了 winsock.
