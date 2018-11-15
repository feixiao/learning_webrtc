## media-server-node-demo部署

安装环境Windows 10 WSL

#### 安装Node

```shell
apt install nodejs-legacy
apt install npm

# 查看node版本
➜  ~ node -v
v4.2.6

# node升级
npm install -g n  	# node版本管理工具 n
n stable          	# 获取稳定版本

#  获取更新之后的版本
➜  ~ node -v
v10.10.0
```

#### 获取代码

```shell
git clone https://github.com/medooze/media-server-demo-node.git
cd  media-server-demo-node
npm install  # 安装依赖项   npm install --unsafe-perm (出错情况使用这个)


node index.js 0.0.0.0    # 默认端口8000

浏览器访问 https://localhost:8000/svc/     # 最后的"/"不要忘记
```

