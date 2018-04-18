## JanusProtocol

存在多种不同的方式与Janus实例交互包括：a [Plain HTTP REST Interface](https://janus.conf.meetecho.com/docs/rest.html#plainhttp) (the default), a [WebSockets Interface](https://janus.conf.meetecho.com/docs/rest.html#WS), a [RabbitMQ interface](https://janus.conf.meetecho.com/docs/rest.html#rabbit), [MQTT interface](https://janus.conf.meetecho.com/docs/FAQ.html#mqtt) and a [UnixSockets interface](https://janus.conf.meetecho.com/docs/rest.html#unix)。全部的接口使用相同的消息，Janus的传输机制已经被模块化，这意味着其他传输方式在将来会变得可用。

### Plain HTTP REST Interface

用户碰到的存在三种不同类型的端点

##### [The gateway root](https://janus.conf.meetecho.com/docs/rest.html#root) 

默认路径**/janus**，但是可以配置，使用Post方式去创建一个会话。

##### [The session endpoint](https://janus.conf.meetecho.com/docs/rest.html#sessions)

类似**/janus/12345678**，使用标识符检索之前创建的会话，您可以发送GET命令 （用于事件和来自插件的消息的长轮询）或POST命令（用来创建插件句柄或操纵会话）。

##### [The plugin handle endpoint](https://janus.conf.meetecho.com/docs/rest.html#handles)

类似**/janus/12345678/98765432**，将句柄标识符附加到会话。只能发送POST命令(插件的消息/谈判，处理操作)。



#### 消息格式

一般来说，所有消息至少共享两个字段。

+ **janus** ： 请求和事件。
+ **transaction** ： 一个随机字符串，客户端可以使用它来匹配来自网关的传入消息。

##### Getting info about the Janus instance

+ http://yourserver:8088/janus/info

##### The gateway root

默认是/janus，可以通过janus.cfg配置文件配置。

+ 创建会话

  ```json
  {
          "janus" : "create",
          "transaction" : "<random alphanumeric string>"
  }
  ```

+ 成功的回复

  ```json
  {
          "janus" : "success",
          "transaction" : "<same as the request>",
          "data" : {
                  "id" : <unique integer session ID>
          }
  }
  ```

##### The session endpoint

一旦您创建了一个会话，您就可以使用在网关中创建的一个新端点。具体地说，新的端点是通过连接**The Gateway Root**和您从返回的会话标识符来构建的。

+ 这个端点可以用两种不同的方式来使用：

  - 使用无参数的GET请求到端点，您将发出一个长轮询请求，获取该会话的事件和传入消息的通知。


  - 使用POST请求发送JSON消息，您将与会话本身交互。

+ **Long-poll requests** 长轮询只会触发与你从插件中发送的消息相关的事件。

  格式如下：

  + **janus **： event
  + **sender**：唯一的插件句柄标识符
  + **transaction**:   这是可选的：它要么与你之前发送给插件的请求有关，要么它可能会丢失，以防这是插件在自己的账户上发送的事件
  + **plugindata**: 一个JSON对象，其中包含来自插件本身的信息
    + plugin: 插件独特的包名
    + data:
  + **jsep** : 一个可选的JSON对象，包含JSEP SDP（提供或回答），插件可以发送到与客户端协商WebRTC PeerConnection

  ```
  GET http://host:port/janus/<sessionid>?maxev=5   # maxev=5  最多返回5个事件
  ```

  ​

+ **Interacting with the session**

  附加到一个插件是为了利用它的特性，需要发attach消息到gateway。当然，你需要提供你想要附加到的插件的信息，通过plugin字段信息。

  attach

  ```
  {
          "janus" : "attach",
          "plugin" : "<the plugin's unique package name>",
          "transaction" : "<random string>"
  }
  ```

  destory

  ```
  # To destroy the current session
  {
          "janus" : "destroy",
          "transaction" : "<random string>"
  }
  ```

##### The plugin handle endpoint

一旦你创建了一个插件句柄，你就可以使用在网关中创建的一个新的端点。具体地说，新的端点是通过连接the gateway root、会话标识符和您已返回的新插件句柄标识符来构造的（比如`/janus/12345678/98765432`）。

你可以使用这个插件句柄来处理与插件通信相关的所有内容，给插件发送消息，商议Webrtc连接(attach)到该插件等等。

给插件发送消息/请求，你需要使用Post方法，数据指示为"message"。body字段将会包含特定的Json数据，如果消息还需要传达webrtc相关的谈判信息，那么JSEP字段中包含JSEP对象的json化版本也可以被附加。

+ **关闭声音的例子**

  ```json
  {
          "janus" : "message",
          "transaction" : "sBJNyUhH6Vc6",
          "body" : {
                  "audio" : false
          }
  }
  ```

+ **包含协商信息的例子**

  ```json
  {
          "janus" : "message",
          "transaction" : "sBJNyUhH6Vc6",
          "body" : {
                  "audio" : false
          },
          "jsep" : {
                  "type" : "offer",
                  "sdp" : "v=0\r\no=[..more sdp stuff..]"
          }
  }
  ```

  注意：出于任何理由你都不想使用the trickling of ICE candidates(这意味着这些都会包含在the SDP OFFER or ANSWER)，你必须添加额外的`"trickle" : false`到jsep对象，去显示的告知Janus不要发送任何的`trickle` candidate。

  **启用trickled的方式**

  ```json
  {
          "janus" : "trickle",
          "transaction" : "hehe83hd8dw12e",
          "candidates" : [ 
                  {
                          "sdpMid" : "video",
                          "sdpMLineIndex" : 1,
                          "candidate" : "..."
                  },
                  {
                          "sdpMid" : "video",
                          "sdpMLineIndex" : 1,
                          "candidate" : "..."
                  },
                  [..]
          ]
  }
  ```

  插件处理这种请求可能是同步也可以是异步。同步消息会立马返回，异步消息会返回ack发送给客户。

+ 销毁the plugin handle,可以发送detach消息

  ```json
  {
          "janus" : "detach",
          "transaction" : "<random string>"
  }
  ```

+ 如果想让保持handle存活，想要挂起相关的PeerConnection

  ```json
  {
          "janus" : "hangup",
          "transaction" : "<random string>"
  }
  ```

#### Webrtc相关事件

##### webrtcup 

ICE和DTLS成功了，因此Janus正确地建立了与用户/应用程序的PeerConnection。

```json
{
        "janus" : "webrtcup",
        session_id: <the session identifier>,
        sender: <the handle identifier>
}
```

#### media

Janus是否正在接受（`receiving:` `true/false`）音频/视频（`type:` `"audio/video"`）;

```json
{
        "janus" : "media",
        session_id: <the session identifier>,
        sender: <the handle identifier>,
        "type" : "audio",
        "receiving" : true
}
```

##### slowlink

Janus是否报告在这个PeerConnection发送/接收(上行:true / false)媒体数据存在麻烦;

```json
{
        "janus" : "slowlink",
        "session_id" : <the session identifier>,
        "sender" : <the handle identifier>
        "uplink" : true,
        "nacks" : <number of NACKs in the last second>
}
```

##### hangup

PeerConnection对象关闭

```json
{
        "janus" : "hangup",
        "session_id" : <the session identifier>,
        "sender" : <the handle identifier>,
        "reason" : "DTLS alert"
}
```



### 参考资料

+ [Janus Protocol](https://janus.conf.meetecho.com/docs/rest.html)
+ [JavaScript API](https://janus.conf.meetecho.com/docs/JS.html)   

