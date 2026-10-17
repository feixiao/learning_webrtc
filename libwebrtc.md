# 跨平台 WebRTC C++ 二进制封装库：webrtc-sdk/libwebrtc 深度解析

[webrtc-sdk/libwebrtc](https://github.com/webrtc-sdk/libwebrtc) 是由开源社区（Flutter-WebRTC / CloudWebRTC / LiveKit 核心团队成员主导）维护的一个现代化 **WebRTC C++ 二进制封装层（Binary Wrapper）**。

其核心目标是解决 Google 官方 WebRTC 原生库直接在第三方 C++ 项目中集成时面临的**构建极其臃肿、头文件与依赖项污染严重、ABI 极易冲突**等痛点，对外提供干净、标准的 C++ 抽象接口以及开箱即用的跨平台预编译二进制产物（`.dll`、`.so`、`.xcframework`、`.aar`）。

---

## 一、为什么需要 libwebrtc？

直接使用 Google 官方原生 WebRTC（`libjingle_peerconnection`）存在以下显著的工程挑战：

1. **构建依赖极度笨重**：WebRTC 依赖 Chromium 的 `depot_tools`、GN 和 Ninja 构建系统，拉取完整源码动辄数十 GB，编译门槛极高；
2. **符号与头文件污染**：原生头文件大量暴露 Chromium 内部基础库（Abseil-cpp、BoringSSL、内部线程模型及内存管理），极易与宿主工程现有依赖发生符号冲突；
3. **C++ 标准库与 ABI 不兼容**：WebRTC 默认使用 Chromium 自带的定制版 `libc++`，与系统的 GCC/Clang `libstdc++` 或 MSVC CRT 混用时，极易引发难以排查的跨运行时崩溃；
4. **缺少轻量级 C++ 包装**：官方原生的 C++ API 主要是内部实现接口，长期演进中经常变动（例如 Unified Plan 重构、AudioTransport 变更等），缺少稳定外露的动态链接 ABI。

**`libwebrtc` 的解决方案：**
- **统一接口封装**：采用面向接口编程（抽象基类 + `scoped_refptr` 引用计数智能指针），头文件完全与 Chromium / Abseil 解耦，仅暴露标准的 C++ 接口；
- **动态库二进制交付**：官方 CI 自动化产出 Windows、Linux、macOS、iOS、Android、嵌入式等多平台的动态链接库与头文件；
- **生态扩展打补丁**：内置自定义音频注入（`CustomAudioTransport`）、高保真采样率支持、Windows 硬件编解码（WMF / Intel MSDK）、Wayland PipeWire 桌面捕获优化、以及 SFrame/E2EE 端到端帧加密等能力。

---

## 二、支持的操作系统与架构矩阵

`libwebrtc` 覆盖了从桌面端、移动端到嵌入式设备的广泛平台支持：

| 操作系统 / 平台 | 目标架构 | 交付产物格式 | 官方 CI 自动化构建 | 图形与多媒体底层支撑 | 典型应用场景 |
| :--- | :--- | :--- | :---: | :--- | :--- |
| **Windows** | `x64` (AMD64)<br>`arm64` (WoA)<br>`x86` (32位旧版) | `libwebrtc.dll`<br>`libwebrtc.dll.lib`<br>C++ 头文件 | ✅ (x64, arm64) | Core Audio, WASAPI, DirectShow, D3D11, DXGI, WMF | Windows 桌面客户端 (Qt, MFC, Electron, Flutter Desktop) |
| **Linux (桌面/服务器)** | `x64` (x86_64)<br>`arm64` (aarch64) | `libwebrtc.so`<br>C++ 头文件 | ✅ (x64, arm64) | ALSA, PulseAudio, X11 (Xlib), Wayland (PipeWire + DMA-BUF) | Linux 原生 GUI 软件、无头机器人、媒体录制网关 |
| **Embedded Linux** | `armv7` (ARM 32-bit)<br>`arm64` (ARM 64-bit) | `libwebrtc.so`<br>C++ 头文件 | 需交叉编译 (提供完整脚本) | V4L2, ALSA, 嵌入式轻量级渲染 | 树莓派 (Raspberry Pi)、RK3588/RK3568 工控机、车载终端、智能音箱 |
| **Apple macOS** | `x86_64` + `arm64`<br>(Universal 二进制) | `WebRTC.xcframework` 或 `libwebrtc.dylib` | ✅ (xcframework) | CoreAudio, AVFoundation, Metal, ScreenCaptureKit | macOS 桌面 App、Mac Catalyst 混合应用 |
| **Apple iOS** | `arm64` (真机)<br>`x86_64` / `arm64` (模拟器) | `WebRTC.xcframework` | ✅ (xcframework) | CoreAudio, VideoToolbox, Metal | iPhone / iPad 原生或混合应用 |
| **Apple tvOS / visionOS** | `arm64` (真机)<br>`arm64` (模拟器) | `WebRTC.xcframework` | ✅ (xcframework) | Apple TV 与 Apple Vision Pro (xrOS) 音视频栈 | 空间计算 (Vision Pro) 实时通信、大屏终端会议 |
| **Android** | `armeabi-v7a`<br>`arm64-v8a`<br>`x86`<br>`x86_64` | `libwebrtc.aar` | ✅ (多架构合体 AAR) | OpenSL ES, AAudio, Camera2, MediaCodec | Android 手机/平板、车机、智能大屏、定制 Android 设备 |

> **提示**：GitHub Releases 页面中，每个正式版本标签（如 `libwebrtc.m150.7871.xx`）都会自动挂载 Windows、Linux 的 Release Zip 压缩包，以及多架构合一的 `libwebrtc-android-release.aar` 与 `WebRTC.xcframework.zip`。

---

## 三、支持的功能与特性全景

`libwebrtc` 对 WebRTC 规范及工业落地常用需求进行了深度封装，功能覆盖以下九大模块：

```
+-----------------------------------------------------------------------------------+
|                            libwebrtc 核心功能架构                                 |
+-----------------------------------------------------------------------------------+
|  [核心管道] RTCPeerConnectionFactory / RTCPeerConnection / Unified Plan / SDP      |
|  [媒体流与轨道] RTCMediaStream / RTCAudioTrack / RTCVideoTrack / Transceiver API  |
|  [数据通道] RTCDataChannel (SCTP 基于流的文本/二进制可靠与不可靠传输)              |
|  [音频系统] 设备枚举 / 3A 算法 / CustomAudioTransport 自定义音频注入 / 192kHz 采样 |
|  [视频系统] 摄像头设备 / CustomVideoSource 自定义外置视频帧注入 / 像素转换 (ARGB)   |
|  [屏幕共享] 桌面/窗口采集 / 实时 JPEG 缩略图 / Linux PipeWire & X11 双协议支持      |
|  [编解码加速] VP8, VP9, AV1(dav1d), H.264, H.265 / WMF & Intel MSDK 硬件加速编解码|
|  [端到端加密] SFrame 规范 RTCFrameCryptor (AES-GCM/CBC) / 密钥棘轮滚动 / 数据包加密|
|  [诊断与调优] 全局 Field Trials 动态配置 / RTCLogging 统一日志拦截 / 异步 GetStats |
+-----------------------------------------------------------------------------------+
```

### 1. 核心 PeerConnection 与传输规范
- **自动化三线程模型**：封装并由 `LibWebRTC::Initialize()` 统一初始化 `worker_thread`、`signaling_thread`、`network_thread` 与 SSL 环境，进程退出时 `Terminate()` 自动安全释放，规避多线程析构死锁；
- **对齐标准 W3C 规范**：完全支持 `CreateOffer`、`CreateAnswer`、`SetLocalDescription`、`SetRemoteDescription` 异步流程；
- **Trickle ICE 与候选者管理**：`AddCandidate`、`RestartIce`，支持完整的 ICE 收集状态（`RTCIceGatheringState`）与连接状态（`RTCIceConnectionState`）回调；
- **Unified Plan 语义**：提供 `AddTransceiver`、`AddTrack`、`RemoveTrack`，精细控制 RTP 方向（`SendRecv`、`SendOnly`、`RecvOnly`、`Inactive`）；
- **实时统计报告**：提供异步 `GetStats` 接口，支持按 Sender / Receiver 过滤，统计对象内置 `ToJson()` 可快速序列化上报。

### 2. 数据通道 (RTCDataChannel)
- 基于 SCTP 协议的低延迟数据链路；
- 支持有序（Ordered）与无序传输，支持设置最大重传次数（`maxPacketLifeTime` / `maxRetransmits`）；
- 支持 UTF-8 字符串与二进制数据块（`uint8_t*`）的双向收发；
- 具备完整的生命周期回调（`OnStateChange`、`OnMessage`）。

### 3. 音频系统与定制化传输
- **设备枚举与热插拔**：`RTCAudioDevice` 提供录音与播放设备数量枚举、名称/GUID 获取、设备切换以及系统音频设备变动的监听通知（`OnDeviceChange`）；
- **音量控制**：支持动态设置和读取麦克风与扬声器音量；
- **高保真音频采样支持**：打入了 Windows Core Audio 补丁（`allow_176k4_192k_shared_mode_formats.patch`），支持高达 176.4kHz 和 192kHz 采样率的专业音频设备；
- **自定义音频注入 (`CustomAudioTransport`)**：
  - 通过 `custom_audio_source_m150.patch` 等补丁，允许开发者重载 `AudioTransport`；
  - 绕过硬件麦克风/扬声器，支持将外部合成音频、伴音、网络拉流 PCM 直接塞入 WebRTC 管道；
- **音频前/后处理扩展 (`RTCAudioProcessing`)**：
  - 继承 WebRTC 原生优秀的 3A 算法（AEC 回声消除、ANS 降噪、AGC 自动增益）；
  - 提供 `CustomProcessing` 回调钩子，允许挂载**采集后（Capture Post-Processing）**与**渲染前（Render Pre-Processing）**处理，方便接入第三方 AI 降噪、美声变声算法。

### 4. 视频系统与自定义视频流
- **摄像头采集**：`RTCVideoDevice` 支持物理摄像头枚举与 `VCMCapturer` 启动捕获；
- **自定义视频源 (`CreateCustomVideoSource`)**：
  - 支持创建不依赖任何物理摄像头的虚拟视频源；
  - 允许上层程序随时通过 `RTCVideoSource::OnCapturedFrame(scoped_refptr<RTCVideoFrame>)` 直接推入自定义生成的每一帧画面；
  - 典型应用场景：OpenCV 算法处理后画面、屏幕/窗口截图推送、游戏画面渲染、AI 虚拟人画面注入；
- **像素格式与色彩空间转换**：
  - 原生 `RTCVideoFrame` 支持直接访问 Y、U、V 各分量内存指针（`DataY`、`DataU`、`DataV`）及其行跨距（`Stride`）；
  - 内置高性能转码函数 `ConvertToARGB(Type type, ...)`，支持快速将 YUV 数据转换为 `kARGB`、`kBGRA`、`kABGR`、`kRGBA`；
- **通用渲染器抽象 (`RTCVideoRenderer<T>`)**：接口极简，接收到视频帧后可零拷贝转换为 Qt `QImage`、wxWidgets `wxImage`、SDL 纹理或 Direct3D / OpenGL 纹理。

### 5. 桌面共享与窗口抓取 (Desktop Capture)
- **多屏与窗口遍历**：`RTCDesktopMediaList` 能够枚举当前系统所有显示器屏幕（Screen）或应用窗口（Window）；
- **内置缩略图生成器**：内部集成 TurboJPEG，可直接抓取并输出各个屏幕/窗口的实时 JPEG 缩略图字节流，供 UI 绘制选择面板；
- **精细化抓屏控制**：`RTCDesktopCapturer` 支持设定目标帧率（fps）、任意局部矩形裁剪区域（x, y, w, h），支持配置是否包含鼠标指针（`showCursor`）；
- **Linux 现代图形栈兼容**：
  - 支持传统 X11 协议截屏；
  - 原生支持现代 Wayland 桌面下通过 **PipeWire + DMA-BUF** 进行高效、无权限弹窗阻断的屏幕共享（打入 `fix_desktop_capture_compile.patch` 保证稳定编译）。

### 6. 编解码器与硬件加速
- **全格式编解码支持**：
  - **VP8 / VP9**：内置原生 Google `libvpx`；
  - **AV1**：内置 `libaom` 编码器与新一代超轻量高性能解码器 `dav1d`；
  - **H.264**：开启 `rtc_use_h264=true` 并集成 Chrome 级别的 FFmpeg H.264 解码器；
  - **H.265 (HEVC)**：构建支持 `rtc_use_h265=true`；
- **Windows 硬件编解码加速扩展**：
  - **Windows Media Foundation (WMF)**：提供专用的 WMF H.265 硬件编码器与解码器（`wmf_h265_encoder.cc`、`wmf_h265_decoder.cc`），调用 GPU 编解码引擎大幅减轻 CPU 负荷；
  - **Intel Media SDK (MSDK)**：支持可选集成 Intel 硬件加速管线（D3D11 / DXGI 纹理与零拷贝），支持硬件硬解/硬编。

### 7. 端到端加密 E2EE (SFrame / FrameCryptor)
`libwebrtc` 内置了现代 WebRTC 工业界标准的媒体帧级端到端加密体系（主要参考 SFrame 规范与 LiveKit / Flutter-WebRTC E2EE 实现）：
- **帧级透明加解密 (`RTCFrameCryptor`)**：
  - 挂载在 `RTCRtpSender` 与 `RTCRtpReceiver` 上，在 RTP 打包发送前与解包接收后对媒体负载（Payload）进行加解密；
  - 即使媒体经过不可信的第三方 SFU 服务器转发，中间节点也无法偷窥音视频内容；
  - 支持 `AES-GCM` 与 `AES-CBC` 密码算法；
- **密钥派生与棘轮密钥轮换 (`KeyProvider`)**：
  - 支持 `PBKDF2` 与 `HKDF` 密钥派生算法；
  - 支持密钥棘轮轮换（Key Ratchet）、设置密钥环（KeyRing，最大 255 个密钥）、未加密魔法字节与 SIF 挂载；
  - 具备状态感知回调（`OnFrameCryptionStateChanged`，感知密钥缺失、解密失败、内部异常等）；
- **数据通道加密 (`RTCDataPacketCryptor`)**：扩展支持对 DataChannel 数据包进行独立加解密保护。

### 8. 运行时微调与日志体系
- **全局 Field Trials 动态生效 (`RTCFieldTrials`)**：
  - 允许在调用 `LibWebRTC::Initialize()` 之前注入自定义开关，例如开启 `WebRTC-FlexFEC-03/Enabled/`、`WebRTC-H264HighProfile/Enabled/` 等；
  - 无需为调整实验参数反复重新编译巨大的二进制库；
- **统一日志过滤与拦截 (`RTCLogging`)**：支持全局调节 WebRTC 内部日志的详细等级（Verbose, Info, Warning, Error）。

---

## 四、核心接口调用示例（C++ 代码骨架）

以下演示如何在 C++ 工程中使用 `libwebrtc` 创建 PeerConnection 并使用自定义视频源：

```cpp
#include <iostream>
#include "libwebrtc.h"

using namespace libwebrtc;

// 1. 实现 PeerConnection 状态监听器
class SimplePCObserver : public RTCPeerConnectionObserver {
public:
    void OnSignalingState(RTCSignalingState state) override {
        std::cout << "[PC] Signaling State Changed: " << state << std::endl;
    }
    void OnIceGatheringState(RTCIceGatheringState state) override {}
    void OnIceConnectionState(RTCIceConnectionState state) override {
        std::cout << "[PC] ICE Connection State: " << state << std::endl;
    }
    void OnIceCandidate(scoped_refptr<RTCIceCandidate> candidate) override {
        std::cout << "[PC] New Candidate: " << candidate->candidate() << std::endl;
    }
    void OnTrack(scoped_refptr<RTCRtpTransceiver> transceiver) override {}
    void OnDataChannel(scoped_refptr<RTCDataChannel> data_channel) override {}
    void OnRenegotiationNeeded() override {}
    void OnAddStream(scoped_refptr<RTCMediaStream> stream) override {}
    void OnRemoveStream(scoped_refptr<RTCMediaStream> stream) override {}
    void OnAddTrack(vector<scoped_refptr<RTCMediaStream>> streams,
                    scoped_refptr<RTCRtpReceiver> receiver) override {}
    void OnRemoveTrack(scoped_refptr<RTCRtpReceiver> receiver) override {}
};

int main() {
    // 2. 初始化环境（自动拉起 worker/signaling/network 线程池与 SSL）
    if (!LibWebRTC::Initialize()) {
        std::cerr << "Failed to initialize LibWebRTC!" << std::endl;
        return -1;
    }

    // 3. 创建 PeerConnection 工厂
    scoped_refptr<RTCPeerConnectionFactory> factory =
        LibWebRTC::CreateRTCPeerConnectionFactory();
    factory->Initialize();

    // 4. 配置 STUN / TURN 服务器
    RTCConfiguration config;
    IceServer stun_server;
    stun_server.uri = "stun:stun.l.google.com:19302";
    config.ice_servers.push_back(stun_server);

    // 5. 创建 PeerConnection 实例
    SimplePCObserver observer;
    scoped_refptr<RTCMediaConstraints> constraints = RTCMediaConstraints::Create();
    scoped_refptr<RTCPeerConnection> pc = factory->Create(config, constraints);
    pc->RegisterRTCPeerConnectionObserver(&observer);

    // 6. 创建自定义视频源与轨道（无需硬件摄像头）
    scoped_refptr<RTCVideoSource> video_source =
        factory->CreateCustomVideoSource("custom_video_track", constraints);
    scoped_refptr<RTCVideoTrack> video_track =
        factory->CreateVideoTrack(video_source, "custom_video_track");

    // 添加轨道到 PeerConnection
    std::vector<std::string> stream_ids = {"stream_0"};
    pc->AddTrack(video_track, stream_ids);

    // 7. 向视频源持续灌入自定义 YUV/ARGB 画面（外部渲染线程循环执行）
    // int w = 640, h = 480;
    // auto frame = RTCVideoFrame::Create(w, h, y_ptr, stride_y, u_ptr, stride_u, v_ptr, stride_v);
    // video_source->OnCapturedFrame(frame);

    // 8. 创建 Offer 进行 SDP 协商
    pc->CreateOffer(
        [](const std::string sdp, const std::string type) {
            std::cout << "CreateOffer Success:\n" << sdp << std::endl;
        },
        [](const char* error) {
            std::cerr << "CreateOffer Error: " << error << std::endl;
        },
        constraints);

    // 9. 资源销毁与退出
    // pc->Close();
    // factory->Terminate();
    // LibWebRTC::Terminate();
    return 0;
}
```

---

## 五、获取与集成方式

### 方式一：直接使用 GitHub 预编译二进制包（推荐）
在项目 [Releases 页面](https://github.com/webrtc-sdk/libwebrtc/releases) 下载对应操作系统和架构的包：
- Windows: `libwebrtc-win-{x64,arm64}-release.zip`（内含 `libwebrtc.dll`、`libwebrtc.dll.lib` 及完整的 `include/` 头文件目录）
- Linux: `libwebrtc-linux-{x64,arm64}-release.zip`（内含 `libwebrtc.so` 及 `include/`）
- macOS / iOS: `WebRTC.xcframework.zip`
- Android: `libwebrtc-android-release.aar`

在你的 `CMakeLists.txt` 中直接链接：
```cmake
# 指定头文件目录
include_directories("${LIBWEBRTC_ROOT}/include")

# 链接预编译动态库
if (WIN32)
    target_link_libraries(your_app PRIVATE "${LIBWEBRTC_ROOT}/lib/libwebrtc.dll.lib")
elseif (UNIX AND NOT APPLE)
    target_link_libraries(your_app PRIVATE "${LIBWEBRTC_ROOT}/lib/libwebrtc.so")
endif ()
```

### 方式二：使用 Conan 包管理器
仓库根目录包含 `conanfile.py`，支持通过 Conan 自动引入与构建依赖包。

### 方式三：从源码完整构建
若需要修改源码、调整 GN 参数或集成自定义编解码器，构建流程如下：

1. **安装 Google depot_tools** 并将其加入系统 `PATH`；
2. **拉取指定 Release 分支**（例如 `m150_release` 或 `m144_release`）：
   ```bash
   mkdir libwebrtc_build && cd libwebrtc_build
   # 配置 .gclient 指向 https://github.com/webrtc-sdk/webrtc.git
   gclient sync
   ```
3. **拉取 wrapper 源码并打上扩展补丁**：
   ```bash
   cd src
   git clone https://github.com/webrtc-sdk/libwebrtc
   git apply libwebrtc/patches/custom_audio_source_m150.patch
   git apply libwebrtc/patches/add_libwebrtc_build_target.patch
   ```
4. **生成工程配置并编译**：
   - **Linux**:
     ```bash
     gn gen out/Default --args='target_os="linux" target_cpu="x64" is_debug=false rtc_use_h264=true rtc_use_h265=true rtc_use_pipewire=true is_component_build=false use_rtti=true'
     ninja -C out/Default libwebrtc
     ```
   - **Windows**:
     ```bash
     gn gen out/Default --args="target_os=\"win\" target_cpu=\"x64\" is_clang=true is_debug=false rtc_use_h264=true rtc_use_h265=true is_component_build=false"
     ninja -C out/Default libwebrtc
     ```

---

## 六、总结与技术选型建议

| 需求场景 | 推荐选型 | 理由与考量 |
| :--- | :--- | :--- |
| **Flutter 桌面端音视频开发** | **libwebrtc** (首选) | `flutter-webrtc` 官方 Windows/Linux 底座，无需任何额外适配，直接集成即可拥有完整媒体能力 |
| **原生 C++ 桌面客户端 (Qt / wxWidgets)** | **libwebrtc** (首选) | 免去维护数十 GB WebRTC 源码的成本，头文件与 ABI 干净稳定，开箱即用支持多屏共享与自定义音视频注入 |
| **嵌入式 Linux 设备 (树莓派/RK3588/车机)** | **libwebrtc** (强烈推荐) | 官方具备成熟的 `armv7` 与 `arm64` 裁剪编译流程，比直接拉官方全量源码交叉编译简单数倍 |
| **纯 Web 浏览器端** | 原生 `window.RTCPeerConnection` | 浏览器自带成熟标准 API，无需 Native 库 |
| **Go 语言微服务 / SFU 服务端** | Pion WebRTC | 纯 Go 编写，无 CGO 依赖，高并发轻量化，运维简单 |
| **深度定制 WebRTC 内部算法的超大型自研团队** | Google 原生 WebRTC 源码树 | 需要频繁改动底层 GCC/BWE/NetEQ 算法细节，直接在官方主干维护 Patch 树 |
