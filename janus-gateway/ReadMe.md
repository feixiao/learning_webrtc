## janus-gateway

### 安装部署

+ [Ubuntu 下 Janus Server 搭建笔记](https://www.jianshu.com/p/760e2eb46a0e)

  ```ssh
  sudo apt-get install libmicrohttpd-dev libjansson-dev libnice-dev  
  sudo apt-get install libssl-dev libsrtp-dev libsofia-sip-ua-dev libglib2.0-dev
  sudo apt-get install libopus-dev libogg-dev libcurl4-openssl-dev pkg-config gengetopt 
  sudo apt-get install libtool automake

  sudo apt-get update && sudo apt-get upgrade
  ```

  ​

+ 依赖版本
  +  libwebsockets-2.4.2

  + libsrtp-2.1.0

  + janus-gateway-0.3.0

    ​

### [Frequently Asked Questions](https://janus.conf.meetecho.com/docs/FAQ)

### [Resources](https://janus.conf.meetecho.com/docs/resources.html)



### 运行Demo

#### janus-gateway

```
./bin/janus
```

#### nodejs

```
sudo apt-get install nodejs
sudo apt-get install npm
npm install http-server -g

#　进入相关目录
cd /home/frank/research/janus-gateway-0.3.1/html
#　启动httpserver
http-server 

# 浏览器访问地址
http://127.0.0.1:8080/
http://127.0.0.1:8080/echotest.html
```



###  源码结构

+ [源码文件功能分析](https://janus.conf.meetecho.com/docs/files.html)
+ [模块分析](https://janus.conf.meetecho.com/docs/modules.html)
+ [数据结构分析](https://janus.conf.meetecho.com/docs/annotated.html)

