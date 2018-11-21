## Webrtc代码获取

#### 配置git代理

```shell
# 设置代理
git config --global http.proxy 'socks5://172.28.101.219:1080'
git config --global https.proxy 'socks5://172.28.101.219:1080'

# 取消代理
git config --global --unset http.proxy
git config --global --unset https.proxy
```

#### 配置HTTP代理

```
sudo apt-get install privoxy
# 修改配置
sudo vim /etc/privoxy/config

#添加
forward-socks5   /               172.28.101.219:1080 .

#重启
sudo /etc/init.d/privoxy restart
```

#### [安装depot_tools](http://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools_tutorial.html#_setting_up) 

A tutorial introduction to the Chromium depot_tools git extensions.

```shell
# 下载
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
# 添加到环境变量
export PATH=$PATH:/home/frank/webrtc/depot_tools
```

#### 安装依赖项

```shell
export http_proxy=http://127.0.0.1:8118/
export https_proxy=http://127.0.0.1:8118/
wget https://cs.chromium.org/chromium/src/build/install-build-deps.sh
```

#### 下载代码

```
mkdir webrtc_android && cd webrtc_android
fetch --nohooks webrtc_android
gclient sync
```

#### 编译代码

```shell
gn gen out/Debug --args='target_os="android" target_cpu="arm"'
ninja -C out/Debug

#To build for ARM64: use target_cpu="arm64"
#To build for 32-bit x86: use target_cpu="x86"
#To build for 64-bit x64: use target_cpu="x64"
```

### 参考资料

+ [native code](https://webrtc.org/native-code/android/)