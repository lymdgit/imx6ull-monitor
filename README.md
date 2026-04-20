# 基于V4L2与Qt的嵌入式视频监控系统 (IMX6ULL)

这是一个专为嵌入式Linux平台（基于IMX6ULL）设计的轻量级视频画面采集与流媒体分发项目。本项目使用 C++ 和 Qt 框架开发，底层基于 V4L2 (Video for Linux 2) 接口进行摄像头硬件的图像采集，支持本地屏幕实时预览与 RTSP 将视频流推送到云端或本地服务器。

## 🌟 核心特性 (Features)

本项目支持两种核心运行模式，可根据实际应用场景灵活切换：

1. **本地显示模式 (Local Display)**
   - 树莓派/IMX6ULL外接摄像头后，通过 V4L2 抓取视频帧。
   - 使用 Qt 框架将经过处理后的图像实时渲染并显示在开发板自带的 LCD 屏幕上。
   - 适合需要现场实时监控的边缘终端设备。

2. **RTSP 网络推流模式 (RTSP Streaming)**
   - 开发板抓取视频帧后，将其作为视频源，通过网络通信协议推送到远端的 RTSP 流媒体服务器。
   - 完美兼容 **mediamtx** 轻量级流媒体服务器。
   - 局域网/广域网内的其他设备（如 PC、手机）可通过 **VLC 播放器** 通过拉取 RTSP 流，实现低延迟的远程视频监控。

## 🛠️ 硬件与技术栈 (Tech Stack & Hardware)

- **主控硬件配置**：NXP i.MX6ULL 开发板（韦东山开发板，自带 LCD 显示屏）
- **外设**：免驱 USB 摄像头 (UVC) 或定制的 CSI 摄像头模块
- **GUI & 应用程序框架**：Qt 
- **视频采集组件**：V4L2 (Video for Linux 2)
- **流媒体服务器**：[mediamtx](https://github.com/bluenviron/mediamtx) (部署运行在 Ubuntu 系统上)
- **测试/客户端工具**：VLC Media Player

## ⚙️ 系统架构图简述 (Architecture)

```text
[ USB/CSI Camera ] ---V4L2---> [ IMX6ULL (Qt App) ] 
                                     |
                                     |---> 1. LCD Screen (本地画面显示)
                                     |
                                     |---> 2. RTSP Push ---> [ Ubuntu (mediamtx) ] ---> [ VLC Player (拉流端) ]
```

## 🚀 快速上手 (Getting Started)

### 环境准备
1. 确保 IMX6ULL 开发板已烧录带有 Qt 库的 Linux 根文件系统。
2. 确保 Ubuntu 服务器上已下载并运行了 `mediamtx`，处于监听状态。

### 编译与运行
*(开发者可以在此补充具体的交叉编译命令和开发板上的启动命令，例如：)*
```bash
# 在 Ubuntu 主机上使用交叉编译工具链进行编译
qmake
make

# 将编译好的可执行文件拷贝到开发板，并执行
./v4l2_qt_camera
```


