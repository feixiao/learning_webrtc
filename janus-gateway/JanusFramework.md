## JanusFramework分析

### Janus交互方式

存在多种不同的方式与Janus实例交互包括：a [Plain HTTP REST Interface](https://janus.conf.meetecho.com/docs/rest.html#plainhttp) (the default), a [WebSockets Interface](https://janus.conf.meetecho.com/docs/rest.html#WS), a [RabbitMQ interface](https://janus.conf.meetecho.com/docs/rest.html#rabbit), [MQTT interface](https://janus.conf.meetecho.com/docs/FAQ.html#mqtt) and a [UnixSockets interface](https://janus.conf.meetecho.com/docs/rest.html#unix) 。他们都以插件的形式实现，我们也可以自定义自己的传输方式实现。

#### 支持传输协议

```shell
路径：janus-gateway-0.3.1/transports
目前支持下面五种传输方式的实现
-rw-rw-r-- 1 frank frank  70K 4月   4 23:03 janus_http.c
-rw-rw-r-- 1 frank frank  29K 4月   4 23:03 janus_mqtt.c
-rw-rw-r-- 1 frank frank  29K 4月   4 23:03 janus_pfunix.c
-rw-rw-r-- 1 frank frank  31K 4月   4 23:03 janus_rabbitmq.c
-rw-rw-r-- 1 frank frank  42K 4月   4 23:03 janus_websockets.c
```

#### 传输插件实现

```c
接口定义在 janus-gateway-0.3.1/transports/transport.h
struct janus_transport {
	/*! \brief 传输插件的构造函数
	 * @param[in] callback 网关的回调函数
	 * @param[in] config_path 配置文件路径获取传输插件的信息
	 * @returns 0表示成功，其他参考错误码 */
	int (* const init)(janus_transport_callbacks *callback, const char *config_path);
	/*! \brief 传输插件的析构函数 */
	void (* const destroy)(void);

	/*! \brief 获取插件的版本信息
	 *  \note 全部的插件必须全部实现该方法并且返回JANUS_TRANSPORT_API_VERSION */
	int (* const get_api_compatibility)(void);
	/*! \brief 获取传输插件的数字版本 */
	int (* const get_version)(void);
	/*! \brief 获取传输插件的字符串版本 */
	const char *(* const get_version_string)(void);
	/*! \brief 获取传输插件的描述信息 */
	const char *(* const get_description)(void);
	/*! \brief 获取传输插件的名字信息 */
	const char *(* const get_name)(void);
	/*! \brief 获取传输插件的作者信息 */
	const char *(* const get_author)(void);
	/*! \brief 获取传输插件的包信息 (用于web应用程序的引用) */
	const char *(* const get_package)(void);

	/*! \brief 检测Janus API 是否被支持*/
	gboolean (* const is_janus_api_enabled)(void);
	/*! \brief 检测Admin API 是否被支持 */·
	gboolean (* const is_admin_api_enabled)(void);

	/*! \brief 使用传输会话(transport session)给客户端发送信息。
	 * \note 传输插件需要释放该消息的资源，除非收到成功的消息否则不能保证消息被送达。
	 * @param[in] transport 指向transport session实例的指针
	 * @param[in] request_id 如果这是对之前请求的响应,request_id不能为NULL
	 * @param[in] admin 是否为an admin API 或者 a Janus API 信息
	 * @param[in] message json_t对象存储的消息内容
	 * @returns 0表示成功 */
	int (* const send_message)(void *transport, void *request_id, gboolean admin, json_t *message);
	/*! \brief 通知传输插件新的会话已经被建立
	 * @param[in] transport 指向transport session实例的指针
	 * @param[in] session_id 如果传输关心id，会话id会被创建 */
	void (* const session_created)(void *transport, guint64 session_id);
	/*! \brief 通知传输插件会话超时
	 * @param[in] 指向transport session实例的指针
	 * @param[in] 如果传输关心id，该会话id会被关闭
	 * @param[in] 会话关闭的原因是否是超时 */
	void (* const session_over)(void *transport, guint64 session_id, gboolean timeout);
};
```

#### 传输插件回调网关函数

```C
// 在传输插件的init方法中传入
struct janus_transport_callbacks {
	/*! \brief 回调函数通知有新的请求
	 * @param[in] plugin 
	 * @param[in] transport 指向transport session实例的指针用于获取事件
	 * @param[in] request_id Opaque pointer to a transport plugin specific value that identifies this request, so that an incoming response coming later can be matched
	 * @param[in] admin 是否为an admin API 或者 a Janus API 请求
	 * @param[in] message json_t对象存储的消息内容 */
	void (* const incoming_request)(janus_transport *plugin, void *transport, void *request_id, gboolean admin, json_t *message, json_error_t *error);
	/*! \brief 回调函数通知已经存在的传输通道离开
	 * @param[in] handle The transport session that went away
	 * @param[in] transport 指向离开会话的实例指针 */
	void (* const transport_gone)(janus_transport *plugin, void *transport);
	/*! \brief 回调函数检测核心代码，是否API秘钥必须被提供
	 * @returns TRUE if an API secret is needed, FALSE otherwise */
	gboolean (* const is_api_secret_needed)(janus_transport *plugin);
	/*! \brief Callback to check with the core if a provided API secret is valid
	 * @param[in] apisecret The API secret to validate
	 * @returns TRUE if the API secret is correct, FALSE otherwise */
	gboolean (* const is_api_secret_valid)(janus_transport *plugin, const char *apisecret);
	/*! \brief Callback to check with the core if an authentication token is needed
	 * @returns TRUE if an auth token is needed, FALSE otherwise */
	gboolean (* const is_auth_token_needed)(janus_transport *plugin);
	/*! \brief Callback to check with the core if a provided authentication token is valid
	 * @param[in] token The auth token to validate
	 * @returns TRUE if the auth token is valid, FALSE otherwise */
	gboolean (* const is_auth_token_valid)(janus_transport *plugin, const char *token);

	/*! \brief Callback to check whether the event handlers mechanism is enabled
	 * @returns TRUE if it is, FALSE if it isn't (which means notify_event should NOT be called) */
	gboolean (* const events_is_enabled)(void);
	/*! \brief Callback to notify an event to the registered and subscribed event handlers
	 * \note Don't unref the event object, the core will do that for you
	 * @param[in] plugin The transport originating the event
	 * @param[in] event The event to notify as a Jansson json_t object */
	void (* const notify_event)(janus_transport *plugin, void *transport, json_t *event);
};
```

#### 主程

```
// 启动分发传入请求的线程
requests = g_async_queue_new_full((GDestroyNotify) janus_request_destroy);
// 线程主要处理请求队列
GThread *requests_thread = g_thread_try_new("sessions requests", &janus_transport_requests, NULL, &error);


// 线程池主要处理任务队列
error = NULL;
tasks = g_thread_pool_new(janus_transport_task, NULL, -1, FALSE, &error);

其他操作都是围绕两个队列进行
```

##### 请求和任务的区别

+ 请求(request)
+ 任务(task)

### Janus业务处理

#### 支持的业务

```shell
路径：janus-gateway-0.3.1/plugins
目前官网提供如下的业务支持
-rw-rw-r-- 1 frank frank 177K 4月   4 23:03 janus_audiobridge.c
-rw-rw-r-- 1 frank frank  52K 4月   4 23:03 janus_echotest.c
-rw-rw-r-- 1 frank frank  83K 4月   4 23:03 janus_nosip.c
-rw-rw-r-- 1 frank frank  23K 4月   9 09:13 janus-plugin.o
-rw-rw-r-- 1 frank frank  84K 4月   4 23:03 janus_recordplay.c
-rw-rw-r-- 1 frank frank 175K 4月   4 23:03 janus_sip.c
-rw-rw-r-- 1 frank frank 178K 4月   4 23:03 janus_sipre.c
-rw-rw-r-- 1 frank frank 217K 4月   4 23:03 janus_streaming.c
-rw-rw-r-- 1 frank frank  80K 4月   4 23:03 janus_textroom.c
-rw-rw-r-- 1 frank frank  65K 4月   4 23:03 janus_videocall.c
-rw-rw-r-- 1 frank frank 246K 4月   4 23:03 janus_videoroom.c
-rw-rw-r-- 1 frank frank  34K 4月   4 23:03 janus_voicemail.c

```

#### 业务插件实现

```C
struct janus_plugin {
	/*! \brief 插件的构造函数
	 * @param[in] callback 插件使用的回调函数实例，与网关进行交互
	 * @param[in] config_path 获取插件的配置信息
	 * @returns 0表示成功，其他参考错误码 */
	int (* const init)(janus_callbacks *callback, const char *config_path);
	/*! \brief 插件的析构函数 */
	void (* const destroy)(void);

	/*! \brief 请教插件需要的版本信息*/
	int (* const get_api_compatibility)(void);
	/*! \brief 获取插件的数字版本 */
	int (* const get_version)(void);
	/*! \brief 获取插件的字符版本 */
	const char *(* const get_version_string)(void);
	/*! \brief 获取插件的描述 */
	const char *(* const get_description)(void);
	/*! \brief 获取插件的名字 */
	const char *(* const get_name)(void);
	/*! \brief 获取插件的作者名字 */
	const char *(* const get_author)(void);
	/*! \brief 获取传输插件的包信息 (用于web应用程序的引用)*/
	const char *(* const get_package)(void);

	/*! \brief 方法 为对象创建新的会话
	 * @param[in] handle 对端使用的The plugin/gateway session
	 * @param[out] error 错误信息 */
	void (* const create_session)(janus_plugin_session *handle, int *error);
	/*! \brief 处理对端发送过来的信息
	 * @param[in] handle 对端使用的The plugin/gateway session
	 * @param[in] transaction 标识message或者request
	 * @param[in] message 包含message/request的内容的json结构
	 * @param[in] jsep 如果存在包含了type/SDP
	 * @returns 返回janus_plugin_result结构，可能包含a response (for immediate/synchronous replies), an ack
	 * (for asynchronously managed requests) or an error */
	struct janus_plugin_result * (* const handle_message)(janus_plugin_session *handle, char *transaction, json_t *message, json_t *jsep);
	/*! \brief 回调函数当PeerConnection上线或者准备被使用的时候触发。
	 * @param[in] handle 对端使用的The plugin/gateway session */
	void (* const setup_media)(janus_plugin_session *handle);
	/*! \brief 方法用于处理介绍对端的rtp包
	 * @param[in] handle 对端使用的The plugin/gateway session
	 * @param[in] video 指示是否为视频数据
	 * @param[in] buf 数据存储内存
	 * @param[in] len 数据的长度 */
	void (* const incoming_rtp)(janus_plugin_session *handle, int video, char *buf, int len);
	/*! \brief 方法用于处理介绍对端的rtcp包
	 * @param[in] handle 对端使用的The plugin/gateway session
	 * @param[in] video 是否跟视频流相关
	 * @param[in] buf 消息内容存储内存
	 * @param[in] len 消息长度*/
	void (* const incoming_rtcp)(janus_plugin_session *handle, int video, char *buf, int len);
	/*! \brief 方法用于处理介绍对端 SCTP/DataChannel数据(text only, for the moment)*/
	void (* const incoming_data)(janus_plugin_session *handle, char *buf, int len);
	/*! \brief 当很多NACKS被发送或者接收到的时候被核心触发，对端被暴露在一个缓慢的或潜在不可靠的网络中
	 * \note 方法会多次被调用,
	 * @param[in] handle 对端使用的The plugin/gateway session
	 * @param[in] uplink 标明放心 uplink(Janus to peer) or downlink (peer to Janus)
	 * @param[in] video 指示是否为视频流 */
	void (* const slow_link)(janus_plugin_session *handle, int uplink, int video);
	/*! \brief 从对等点收到关于DTLS警报的通知 (i.e., the PeerConnection is not valid any more)
	 * @param[in] handle 对端使用的The plugin/gateway session */
	void (* const hangup_media)(janus_plugin_session *handle);
	/*! \brief Method to destroy a session/handle for a peer
	 * @param[in] handle The plugin/gateway session used for this peer
	 * @param[out] error An integer that may contain information about any error */
	void (* const destroy_session)(janus_plugin_session *handle, int *error);
	/*! \brief Method to get plugin-specific info of a session/handle
	 * @param[in] handle The plugin/gateway session used for this peer
	 * @returns A json_t object with the requested info */
	json_t *(* const query_session)(janus_plugin_session *handle);

};
```

