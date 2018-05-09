### EchoTestDemo分析

#### 创建transport会话

```
C --> S
{
	"janus": "create",            # create方法,Janus Protocol用于创建transport的会话
	"transaction": "1InYKSboDevo" # 表示请求ID，在异步处理过程中方便客户端区分
}

# 回复
{
   "janus": "success",             
   "transaction": "1InYKSboDevo",  # 对应请求的标识
   "data": {
      "id": 6816883350189335       # transaction SessionID
   }
}
```

#### 创建ICE对象

```
C --> S
{
	"janus": "attach",					 # attach方法，创建ICE对象
	"plugin": "janus.plugin.echotest",     # 向janus.plugin.echotest插件发送命令
	"opaque_id": "echotest-2plJY6YXG6Ji",
	"transaction": "0Of95YY5Ho9o",		  # 表示请求ID，在异步处理过程中方便客户端区分
	"session_id": 6816883350189335         # transaction SessionID(create方法中创建)
}
# 回复
{
	"janus": "success",
	"session_id": 6816883350189335,
	"transaction": "0Of95YY5Ho9o",
	"data": {
		"id": 4228050077170019 			# ICE对象ID
	}
}
```

#### 配置音视频通道

```
{
	"janus": "message",      # message消息会交给插件处理，属于插件自定义消息，前面的都是框架处理
	"body": {
		"audio": true,		# 开启音频
		"video": true		# 开启视频
	},
	"transaction": "gKxSwYr4SfnE",
	"session_id": 6816883350189335,  # SessionID
	"handle_id": 4228050077170019    # ICE HandleID
}
```

#### 上传SDP信息

```
# offer操作
{
	"janus": "message",
	"body": {
		"audio": true,
		"video": true
	},
	"transaction": "4h7sptTFmZUh",
	"jsep": {
		"type": "offer",		# offer类型
		"sdp": "v=0\r\no=mozilla...THIS_IS_SDPARTA-59.0.3 1815213017397450941 0 IN IP4 0.0.0.0\r\ns=-\r\nt=0 0\r\na=fingerprint:sha-256 FA:5C:C0:13:A1:A4:F8:7B:C4:8A:88:DF:82:18:77:4E:27:21:B0:E7:02:B9:6B:EE:57:08:89:9A:80:23:B2:0E\r\na=group:BUNDLE sdparta_0 sdparta_1 sdparta_2\r\na=ice-options:trickle\r\na=msid-semantic:WMS *\r\nm=audio 9 UDP/TLS/RTP/SAVPF 109 9 0 8 101\r\nc=IN IP4 0.0.0.0\r\na=sendrecv\r\na=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level\r\na=extmap:2 urn:ietf:params:rtp-hdrext:sdes:mid\r\na=fmtp:109 maxplaybackrate=48000;stereo=1;useinbandfec=1\r\na=fmtp:101 0-15\r\na=ice-pwd:fce81d64f1d26d475024b43a01d32730\r\na=ice-ufrag:a387c063\r\na=mid:sdparta_0\r\na=msid:{1195e3e2-4586-4813-a250-b428a5dceff0} {02aa8736-ee44-425c-a7da-53e434dcf236}\r\na=rtcp-mux\r\na=rtpmap:109 opus/48000/2\r\na=rtpmap:9 G722/8000/1\r\na=rtpmap:0 PCMU/8000\r\na=rtpmap:8 PCMA/8000\r\na=rtpmap:101 telephone-event/8000/1\r\na=setup:actpass\r\na=ssrc:1351558007 cname:{436f99b2-d718-43f0-9c3b-47fd51935dff}\r\nm=video 9 UDP/TLS/RTP/SAVPF 120 121 126 97\r\nc=IN IP4 0.0.0.0\r\na=sendrecv\r\na=extmap:1 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time\r\na=extmap:2 urn:ietf:params:rtp-hdrext:toffset\r\na=extmap:3 urn:ietf:params:rtp-hdrext:sdes:mid\r\na=fmtp:126 profile-level-id=42e01f;level-asymmetry-allowed=1;packetization-mode=1\r\na=fmtp:97 profile-level-id=42e01f;level-asymmetry-allowed=1\r\na=fmtp:120 max-fs=12288;max-fr=60\r\na=fmtp:121 max-fs=12288;max-fr=60\r\na=ice-pwd:fce81d64f1d26d475024b43a01d32730\r\na=ice-ufrag:a387c063\r\na=mid:sdparta_1\r\na=msid:{1195e3e2-4586-4813-a250-b428a5dceff0} {5270ae02-5dd7-4cb7-8571-792dc3ed4d4a}\r\na=rtcp-fb:120 nack\r\na=rtcp-fb:120 nack pli\r\na=rtcp-fb:120 ccm fir\r\na=rtcp-fb:120 goog-remb\r\na=rtcp-fb:121 nack\r\na=rtcp-fb:121 nack pli\r\na=rtcp-fb:121 ccm fir\r\na=rtcp-fb:121 goog-remb\r\na=rtcp-fb:126 nack\r\na=rtcp-fb:126 nack pli\r\na=rtcp-fb:126 ccm fir\r\na=rtcp-fb:126 goog-remb\r\na=rtcp-fb:97 nack\r\na=rtcp-fb:97 nack pli\r\na=rtcp-fb:97 ccm fir\r\na=rtcp-fb:97 goog-remb\r\na=rtcp-mux\r\na=rtpmap:120 VP8/90000\r\na=rtpmap:121 VP9/90000\r\na=rtpmap:126 H264/90000\r\na=rtpmap:97 H264/90000\r\na=setup:actpass\r\na=ssrc:33853765 cname:{436f99b2-d718-43f0-9c3b-47fd51935dff}\r\nm=application 9 DTLS/SCTP 5000\r\nc=IN IP4 0.0.0.0\r\na=sendrecv\r\na=ice-pwd:fce81d64f1d26d475024b43a01d32730\r\na=ice-ufrag:a387c063\r\na=mid:sdparta_2\r\na=sctpmap:5000 webrtc-datachannel 256\r\na=setup:actpass\r\na=max-message-size:1073741823\r\n"
	},
	"session_id": 6816883350189335,
	"handle_id": 4228050077170019
}

# answer
{
	"janus": "event",
	"session_id": 6816883350189335,
	"sender": 4228050077170019,
	"transaction": "4h7sptTFmZUh",   # 对应offer的transaction
	"plugindata": {
		"plugin": "janus.plugin.echotest",
		"data": {
			"echotest": "event",
			"result": "ok"
		}
	},
	"jsep": {
		"type": "answer",   # answer操作
		"sdp": "v=0\r\no=mozilla...THIS_IS_SDPARTA-59.0.3 1525857525581093 1 IN IP4 172.28.104.225\r\ns=-\r\nt=0 0\r\na=group:BUNDLE sdparta_0 sdparta_1\r\na=msid-semantic: WMS janus\r\nm=audio 9 UDP/TLS/RTP/SAVPF 109\r\nc=IN IP4 172.28.104.225\r\na=sendrecv\r\na=mid:sdparta_0\r\na=rtcp-mux\r\na=ice-ufrag:REPb\r\na=ice-pwd:bk84wilJOcWfI3yke8+shT\r\na=ice-options:trickle\r\na=fingerprint:sha-256 D2:B9:31:8F:DF:24:D8:0E:ED:D2:EF:25:9E:AF:6F:B8:34:AE:53:9C:E6:F3:8F:F2:64:15:FA:E8:7F:53:2D:38\r\na=setup:active\r\na=rtpmap:109 opus/48000/2\r\na=ssrc:63327372 cname:janusaudio\r\na=ssrc:63327372 msid:janus janusa0\r\na=ssrc:63327372 mslabel:janus\r\na=ssrc:63327372 label:janusa0\r\na=candidate:1 1 udp 2013266431 172.28.104.225 57649 typ host\r\na=candidate:2 1 udp 2013266430 192.168.31.71 52127 typ host\r\na=end-of-candidates\r\nm=video 9 UDP/TLS/RTP/SAVPF 120\r\nc=IN IP4 172.28.104.225\r\na=sendrecv\r\na=mid:sdparta_1\r\na=rtcp-mux\r\na=ice-ufrag:REPb\r\na=ice-pwd:bk84wilJOcWfI3yke8+shT\r\na=ice-options:trickle\r\na=fingerprint:sha-256 D2:B9:31:8F:DF:24:D8:0E:ED:D2:EF:25:9E:AF:6F:B8:34:AE:53:9C:E6:F3:8F:F2:64:15:FA:E8:7F:53:2D:38\r\na=setup:active\r\na=rtpmap:120 VP8/90000\r\na=rtcp-fb:120 ccm fir\r\na=rtcp-fb:120 nack\r\na=rtcp-fb:120 nack pli\r\na=rtcp-fb:120 goog-remb\r\na=rtcp-fb:120 transport-cc\r\na=ssrc:1503870278 cname:janusvideo\r\na=ssrc:1503870278 msid:janus janusv0\r\na=ssrc:1503870278 mslabel:janus\r\na=ssrc:1503870278 label:janusv0\r\na=candidate:1 1 udp 2013266431 172.28.104.225 57649 typ host\r\na=candidate:2 1 udp 2013266430 192.168.31.71 52127 typ host\r\na=end-of-candidates\r\nm=application 0 DTLS/SCTP 0\r\nc=IN IP4 172.28.104.225\r\na=inactive\r\n"
	}
}
```

#### ICE相关操作

```
{
	"janus": "trickle",
	"candidate": {
		"candidate": "candidate:0 1 UDP 2122252543 172.28.101.219 49186 typ host",
		"sdpMid": "sdparta_0",
		"sdpMLineIndex": 0
	},
	"transaction": "BYle7CVX1vDy",
	"session_id": 6816883350189335,
	"handle_id": 4228050077170019
}

{
	"janus": "trickle",
	"candidate": {
		"candidate": "candidate:2 1 TCP 2105524479 172.28.101.219 9 typ host tcptype active",
		"sdpMid": "sdparta_0",
		"sdpMLineIndex": 0
	},
	"transaction": "a2PcVS1DO8fq",
	"session_id": 6816883350189335,
	"handle_id": 4228050077170019
}
...... 
```

#### ICE和DTLS成功

```
C --> S
{
   "janus": "webrtcup",
   "session_id": 6816883350189335,
   "sender": 4228050077170019
}
```

#### 准备传输媒体数据

```
{
	"janus": "media",
	"session_id": 6816883350189335,
	"sender": 4228050077170019,
	"type": "audio",
	"receiving": true
}

{
	"janus": "media",
	"session_id": 6816883350189335,
	"sender": 4228050077170019,
	"type": "video",
	"receiving": true
}
```

