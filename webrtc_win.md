## Webrtc代码获取



#### [安装depot_tools](http://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools_tutorial.html#_setting_up) 

A tutorial introduction to the Chromium depot_tools git extensions.

+ [privoxy](http://www.privoxy.org/) socks代理转http&https

  配置文件中添加如下内容：

  ```
  forward-socks5 	 / 				 127.0.0.1:1080 .
  ```

+ 控制台设置代理

  ```
  set http_proxy=http://127.0.0.1:8118
  set https_proxy=http://127.0.0.1:8118
  ```

+ 为depot_tools 配置代理

  ```
  1：创建boto.cfg文件
  2：配置如下内容
  [Boto]
  proxy=127.0.0.1
  proxy_port = 8118
  3: 设置环境变量NO_AUTH_BOTO_CONFIG
  set NO_AUTH_BOTO_CONFIG=F:\webrtc_project\boto.cfg
  ```

#### 下载代码

```
mkdir webrtc && cd webrtc
set http_proxy=http://127.0.0.1:8118
set https_proxy=http://127.0.0.1:8118
git config --global http.proxy http://127.0.0.1:8118
git config --global https.proxy http://127.0.0.1:8118
set DEPOT_TOOLS_WIN_TOOLCHAIN 0
set NO_AUTH_BOTO_CONFIG=F:\webrtc_project\boto.cfg
set GYP_GENERATORS=msvs-ninja,ninja 
set GYP_MSVS_VERSION=2017
fetch webrtc
```

#### 编译代码

```
gclient sync
set GYP_MSVS_VERSION=2017

git branch -r
git checkout branch-heads/66
gclient runhooks

# 生成VS2017项目文件(推荐使用) 
gn gen out/Default –ide=vs2017 

# 生成
gn gen out/x64/Debug --ide="vs2017" --args="is_debug=true use_rtti=true target_cpu=\"x64\""
ninja -C out/x64/Debug 


set GYP_MSVS_VERSION = 2017
set DEPOT_TOOLS_WIN_TOOLCHAIN = 0

```



### 参考资料

+ [Building and Installing WebRTC on Windows](https://sourcey.com/building-and-installing-webrtc-on-windows/)
