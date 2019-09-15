## MediaServer

#### 编译
#### Ubuntu 18.04 

+ 安装依赖

  ```shell
  sudo apt-get install libxmlrpc-c++8-dev
  sudo apt-get install libgsm1-dev
  sudo apt-get install libspeex-dev
  sudo apt-get install libopus-dev
  sudo apt-get install libavresample-dev
  sudo apt-get install libx264-dev
  sudo apt-get install libvpx-dev
  sudo apt-get install libswscale-dev
  sudo apt-get install libavformat-dev
  sudo apt-get install libmp4v2-dev
  sudo apt-get install libgcrypt11-dev
  sudo apt-get install libssl1.0-dev
  sudo apt-get install libjpeg-dev
  ```

+ 下载代码

  ```shell
  git clone https://github.com/medooze/media-server.git
  
  # 切换分支
  git checkout remotes/origin/optimizations
  ```

+ 修改config.mk

  ```shell
  SRCDIR		  = /home/frank/webrtc/media-server
  TARGET		  = /home/frank/webrtc/media
  ```

+ 编译common_audio

  ```shell
  sudo apt-get install ninja-build
  cd media-server/ext/out/Release
  ninja
  ```

+ 编译 media-server

  ```shell
  cd media-server
  make
  ```

  





#### 参考资料

+ [《为什么编译Medooze Server这么难？》](https://time.geekbang.org/column/article/135422)