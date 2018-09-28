## medooze/sfu部署

#### Node安装

参考[media-server-node-demo部署](./media-server-node.md)



#### 下载代码

```shell
git clone https://github.com/medooze/sfu.git
```



#### 修改代码

```shell
package.json

"medooze-media-server": "^0.27.2",
修改为
"medooze-media-server": "^0.43.1",
```



#### 安装依赖项

```shell
npm install  
#或者
npm install --unsafe-perm

# 秘钥文件生成
openssl req -sha256 -days 3650 -newkey rsa:1024 -nodes -new -x509 -keyout server.key -out server.cert

# 启动服务,ip设置为客户端都能访问到的ip，不能为0.0.0.0
node index.js [ip]

```



#### 客户端访问

```
https://[ip]:8084/index.html
```

