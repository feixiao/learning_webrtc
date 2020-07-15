## Ubuntu编译webrtc

#### 参考资料
+ [声网源](https://webrtc.org.cn/mirror/)

#### 整理库
```shell
mkdir -p ./dist/libs/linux64
find . -type f -name "*.a"  | xargs -i  cp -f {} ./dist/libs/linux64

# OSX
find . -type f -name "*.a"  | xargs  -n1 -I F cp "F"./dist/libs/osx
```

#### 整理头文件
```shell
find . -name "*.h" | xargs -i {} cp --parents {} ./dist/include
```

#### 打包
```shell
tar -zcvf webrtc_sw_201905.tar.gz ./dist
```
