## janus-gateway

### 安装部署

+ [Ubuntu 下 Janus Server 搭建笔记](https://www.jianshu.com/p/760e2eb46a0e)
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
sudojanus apt-get install nodejs
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



