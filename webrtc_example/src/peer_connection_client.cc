/*
 *  Copyright 2012 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "peer_connection_client.h"
#include "defaults.h"


#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "rtc_base/nethelpers.h"
#include "rtc_base/stringutils.h"

#ifdef WIN32
#include "rtc_base/win32socketserver.h"
#endif

using rtc::sprintfn;

namespace {

// This is our magical hangup signal.
const char kByeMessage[] = "BYE";
// Delay between server connection retries, in milliseconds
const int kReconnectDelay = 2000;

rtc::AsyncSocket* CreateClientSocket(int family) {
#ifdef WIN32
  rtc::Win32Socket* sock = new rtc::Win32Socket();
  sock->CreateT(family, SOCK_STREAM);
  return sock;
#elif defined(WEBRTC_POSIX)
  rtc::Thread* thread = rtc::Thread::Current();
  RTC_DCHECK(thread != NULL);
  return thread->socketserver()->CreateAsyncSocket(family, SOCK_STREAM);
#else
#error Platform not supported.
#endif
}

}  // namespace

PeerConnectionClient::PeerConnectionClient()
  : callback_(NULL),
    resolver_(NULL),
    state_(NOT_CONNECTED),
    my_id_(-1) {
}

PeerConnectionClient::~PeerConnectionClient() {
}


// 事件处理函数连接操作
void PeerConnectionClient::InitSocketSignals() {
  RTC_DCHECK(control_socket_.get() != NULL);
  RTC_DCHECK(hanging_get_.get() != NULL);

  // 关联处理函数

  // 关闭事件
  control_socket_->SignalCloseEvent.connect(this,
      &PeerConnectionClient::OnClose);
  hanging_get_->SignalCloseEvent.connect(this,
      &PeerConnectionClient::OnClose);

  // 连接事件触发的时候调用
  control_socket_->SignalConnectEvent.connect(this,
      &PeerConnectionClient::OnConnect);
  hanging_get_->SignalConnectEvent.connect(this,
      &PeerConnectionClient::OnHangingGetConnect);

  // 读事件
  control_socket_->SignalReadEvent.connect(this,
      &PeerConnectionClient::OnRead);
  hanging_get_->SignalReadEvent.connect(this,
      &PeerConnectionClient::OnHangingGetRead);
}

int PeerConnectionClient::id() const {
  return my_id_;
}

bool PeerConnectionClient::is_connected() const {
  return my_id_ != -1;
}

const Peers& PeerConnectionClient::peers() const {
  return peers_;
}

// 注册观察者对象
void PeerConnectionClient::RegisterObserver(
    PeerConnectionClientObserver* callback) {
  RTC_DCHECK(!callback_);
  callback_ = callback;
}


// 发起连接操作
void PeerConnectionClient::Connect(const std::string& server, int port,
                                   const std::string& client_name) {
  RTC_DCHECK(!server.empty());
  RTC_DCHECK(!client_name.empty());

  // 状态检查
  if (state_ != NOT_CONNECTED) {
    RTC_LOG(WARNING)
        << "The client must not be connected before you can call Connect()";
    callback_->OnServerConnectionFailure();
    return;
  }

  // 参数检查
  if (server.empty() || client_name.empty()) {
    callback_->OnServerConnectionFailure();
    return;
  }

  // 参数修正
  if (port <= 0)
    port = kDefaultServerPort;

  server_address_.SetIP(server);
  server_address_.SetPort(port);
  client_name_ = client_name;

  // 如果ip地址的格式没有解析正常就先解析
  if (server_address_.IsUnresolvedIP()) {
    state_ = RESOLVING;
    resolver_ = new rtc::AsyncResolver();
    // 调用PeerConnectionClient::OnResolveResult进行
    resolver_->SignalDone.connect(this, &PeerConnectionClient::OnResolveResult);
    resolver_->Start(server_address_);
  } else {
	// 当解析成功，调用DoConnect执行连接操作, 该函数对socket做一些相关的初始化工作
    DoConnect();
  }
}

void PeerConnectionClient::OnResolveResult(
    rtc::AsyncResolverInterface* resolver) {
  if (resolver_->GetError() != 0) {
    callback_->OnServerConnectionFailure();
    resolver_->Destroy(false);
    resolver_ = NULL;
    state_ = NOT_CONNECTED;
  } else {
	// 获取域名解析之后的地址
    server_address_ = resolver_->address();
    DoConnect();
  }
}

//　执行连接操作
void PeerConnectionClient::DoConnect() {
  // unique_ptr::reset 取得指针参数的所有权，然后删除原始存储的指针。 
  // 如果新指针与原始存储指针相同，reset将删除指针并将存储的指针设置为nullptr。  
  control_socket_.reset(CreateClientSocket(server_address_.ipaddr().family()));
  hanging_get_.reset(CreateClientSocket(server_address_.ipaddr().family()));

  // Socket相关事件的关联
  InitSocketSignals();
  
  // 构建请求
  char buffer[1024];
  sprintfn(buffer, sizeof(buffer),
           "GET /sign_in?%s HTTP/1.0\r\n\r\n", client_name_.c_str());
  onconnect_data_ = buffer;

  bool ret = ConnectControlSocket();
  if (ret)
    state_ = SIGNING_IN;
  if (!ret) {
    callback_->OnServerConnectionFailure();
  }
}

// 给对端发送信息(通过服务器转发)
bool PeerConnectionClient::SendToPeer(int peer_id, const std::string& message) {
  if (state_ != CONNECTED)
    return false;

  RTC_DCHECK(is_connected());
  RTC_DCHECK(control_socket_->GetState() == rtc::Socket::CS_CLOSED);
  if (!is_connected() || peer_id == -1)
    return false;

  char headers[1024];
  sprintfn(headers, sizeof(headers),
      "POST /message?peer_id=%i&to=%i HTTP/1.0\r\n"
      "Content-Length: %i\r\n"
      "Content-Type: text/plain\r\n"
      "\r\n",
      my_id_, peer_id, message.length());
  onconnect_data_ = headers;
  onconnect_data_ += message;
  return ConnectControlSocket();
}

bool PeerConnectionClient::SendHangUp(int peer_id) {
  return SendToPeer(peer_id, kByeMessage);
}

bool PeerConnectionClient::IsSendingMessage() {
  return state_ == CONNECTED &&
         control_socket_->GetState() != rtc::Socket::CS_CLOSED;
}

bool PeerConnectionClient::SignOut() {
  // 状态检查
  if (state_ == NOT_CONNECTED || state_ == SIGNING_OUT)
    return true;

  // socket状态检查
  if (hanging_get_->GetState() != rtc::Socket::CS_CLOSED)
    hanging_get_->Close();

  if (control_socket_->GetState() == rtc::Socket::CS_CLOSED) {
    state_ = SIGNING_OUT;

    if (my_id_ != -1) {
      char buffer[1024];
      sprintfn(buffer, sizeof(buffer),
          "GET /sign_out?peer_id=%i HTTP/1.0\r\n\r\n", my_id_);
      onconnect_data_ = buffer;
      return ConnectControlSocket();
    } else {
      // Can occur if the app is closed before we finish connecting.
      return true;
    }
  } else {
    state_ = SIGNING_OUT_WAITING;
  }

  return true;
}

void PeerConnectionClient::Close() {
  control_socket_->Close();
  hanging_get_->Close();
  onconnect_data_.clear();
  peers_.clear();
  if (resolver_ != NULL) {
    resolver_->Destroy(false);
    resolver_ = NULL;
  }
  my_id_ = -1;
  state_ = NOT_CONNECTED;
}

// 发起连接请求
bool PeerConnectionClient::ConnectControlSocket() {
  RTC_DCHECK(control_socket_->GetState() == rtc::Socket::CS_CLOSED);
  int err = control_socket_->Connect(server_address_);
  if (err == SOCKET_ERROR) {
    Close();
    return false;
  }
  return true;
}

// TCP连接完成发生登入请求
void PeerConnectionClient::OnConnect(rtc::AsyncSocket* socket) {
  RTC_DCHECK(!onconnect_data_.empty());
  size_t sent = socket->Send(onconnect_data_.c_str(), onconnect_data_.length());
  RTC_DCHECK(sent == onconnect_data_.length());
  onconnect_data_.clear();
}

void PeerConnectionClient::OnHangingGetConnect(rtc::AsyncSocket* socket) {
  char buffer[1024];
  sprintfn(buffer, sizeof(buffer),
           "GET /wait?peer_id=%i HTTP/1.0\r\n\r\n", my_id_);
  int len = static_cast<int>(strlen(buffer));
  int sent = socket->Send(buffer, len);
  RTC_DCHECK(sent == len);
}

void PeerConnectionClient::OnMessageFromPeer(int peer_id,
                                             const std::string& message) {
  if (message.length() == (sizeof(kByeMessage) - 1) &&
      message.compare(kByeMessage) == 0) {
    callback_->OnPeerDisconnected(peer_id);
  } else {
    callback_->OnMessageFromPeer(peer_id, message);
  }
}

bool PeerConnectionClient::GetHeaderValue(const std::string& data,
                                          size_t eoh,
                                          const char* header_pattern,
                                          size_t* value) {
  RTC_DCHECK(value != NULL);
  size_t found = data.find(header_pattern);
  if (found != std::string::npos && found < eoh) {
    *value = atoi(&data[found + strlen(header_pattern)]);
    return true;
  }
  return false;
}

bool PeerConnectionClient::GetHeaderValue(const std::string& data, size_t eoh,
                                          const char* header_pattern,
                                          std::string* value) {
  RTC_DCHECK(value != NULL);
  size_t found = data.find(header_pattern);
  if (found != std::string::npos && found < eoh) {
    size_t begin = found + strlen(header_pattern);
    size_t end = data.find("\r\n", begin);
    if (end == std::string::npos)
      end = eoh;
    value->assign(data.substr(begin, end - begin));
    return true;
  }
  return false;
}

// 获取数据和content_length
bool PeerConnectionClient::ReadIntoBuffer(rtc::AsyncSocket* socket,
                                          std::string* data,
                                          size_t* content_length) {
  char buffer[0xffff];
  //　读完socket缓存中的数据
  do {
    int bytes = socket->Recv(buffer, sizeof(buffer), nullptr);
    if (bytes <= 0)
      break;
    data->append(buffer, bytes);
  } while (true);

  bool ret = false;
  size_t i = data->find("\r\n\r\n");
  if (i != std::string::npos) {
    RTC_LOG(INFO) << "Headers received";
    if (GetHeaderValue(*data, i, "\r\nContent-Length: ", content_length)) {
      size_t total_response_size = (i + 4) + *content_length;
      if (data->length() >= total_response_size) {
        ret = true;
        std::string should_close;
        const char kConnection[] = "\r\nConnection: ";
        if (GetHeaderValue(*data, i, kConnection, &should_close) &&
            should_close.compare("close") == 0) {
          socket->Close();
          // Since we closed the socket, there was no notification delivered
          // to us.  Compensate by letting ourselves know.
          OnClose(socket, 0);
        }
      } else {
        // We haven't received everything.  Just continue to accept data.
      }
    } else {
      RTC_LOG(LS_ERROR) << "No content length field specified by the server.";
    }
  }
  return ret;
}

// 处理服务端发送过来的数据
void PeerConnectionClient::OnRead(rtc::AsyncSocket* socket) {
  size_t content_length = 0;
  if (ReadIntoBuffer(socket, &control_data_, &content_length)) {
    size_t peer_id = 0, eoh = 0;
    bool ok = ParseServerResponse(control_data_, content_length, &peer_id,
                                  &eoh);
    if (ok) {
      if (my_id_ == -1) {
        // First response.  Let's store our server assigned ID.
        RTC_DCHECK(state_ == SIGNING_IN);
        my_id_ = static_cast<int>(peer_id);
        RTC_DCHECK(my_id_ != -1);

        // The body of the response will be a list of already connected peers.
        if (content_length) {
          size_t pos = eoh + 4;
          while (pos < control_data_.size()) {
            size_t eol = control_data_.find('\n', pos);
            if (eol == std::string::npos)
              break;
            int id = 0;
            std::string name;
            bool connected;
            if (ParseEntry(control_data_.substr(pos, eol - pos), &name, &id,
                           &connected) && id != my_id_) {
              peers_[id] = name;

			  // 获取已经连接到服务器端的客户端信息
              callback_->OnPeerConnected(id, name);
            }
            pos = eol + 1;
          }
        }
        RTC_DCHECK(is_connected());
        callback_->OnSignedIn();			// 回调通知SignedIn完成
      } else if (state_ == SIGNING_OUT) {
        Close();
        callback_->OnDisconnected();
      } else if (state_ == SIGNING_OUT_WAITING) {
        SignOut();
      }
    }

    control_data_.clear();

    if (state_ == SIGNING_IN) {
      RTC_DCHECK(hanging_get_->GetState() == rtc::Socket::CS_CLOSED);
      state_ = CONNECTED;
      hanging_get_->Connect(server_address_);
    }
  }
}

void PeerConnectionClient::OnHangingGetRead(rtc::AsyncSocket* socket) {
  RTC_LOG(INFO) << __FUNCTION__;
  size_t content_length = 0;
  if (ReadIntoBuffer(socket, &notification_data_, &content_length)) {
    size_t peer_id = 0, eoh = 0;
    bool ok = ParseServerResponse(notification_data_, content_length,
                                  &peer_id, &eoh);

    if (ok) {
      // Store the position where the body begins.
      size_t pos = eoh + 4;

      if (my_id_ == static_cast<int>(peer_id)) {
        // A notification about a new member or a member that just
        // disconnected.
        int id = 0;
        std::string name;
        bool connected = false;
        if (ParseEntry(notification_data_.substr(pos), &name, &id,
                       &connected)) {
          if (connected) {
            peers_[id] = name;
            //　客户端连接
            callback_->OnPeerConnected(id, name);
          } else {
            peers_.erase(id);
            // 客户端断开
            callback_->OnPeerDisconnected(id);
          }
        }
      } else {
		// 
        OnMessageFromPeer(static_cast<int>(peer_id),
                          notification_data_.substr(pos));
      }
    }

    notification_data_.clear();
  }

  if (hanging_get_->GetState() == rtc::Socket::CS_CLOSED &&
      state_ == CONNECTED) {
    hanging_get_->Connect(server_address_);
  }
}

bool PeerConnectionClient::ParseEntry(const std::string& entry,
                                      std::string* name,
                                      int* id,
                                      bool* connected) {
  RTC_DCHECK(name != NULL);
  RTC_DCHECK(id != NULL);
  RTC_DCHECK(connected != NULL);
  RTC_DCHECK(!entry.empty());

  *connected = false;
  size_t separator = entry.find(',');
  if (separator != std::string::npos) {
    *id = atoi(&entry[separator + 1]);
    name->assign(entry.substr(0, separator));
    separator = entry.find(',', separator + 1);
    if (separator != std::string::npos) {
      *connected = atoi(&entry[separator + 1]) ? true : false;
    }
  }
  return !name->empty();
}

int PeerConnectionClient::GetResponseStatus(const std::string& response) {
  int status = -1;
  size_t pos = response.find(' ');
  if (pos != std::string::npos)
    status = atoi(&response[pos + 1]);
  return status;
}


//　解析数据，获取peer_id和"\r\n\r\n"的位置
bool PeerConnectionClient::ParseServerResponse(const std::string& response,
                                               size_t content_length,
                                               size_t* peer_id,
                                               size_t* eoh) {
  
  // 返回信息不是200就直接断开连接
  int status = GetResponseStatus(response.c_str());
  if (status != 200) {
    RTC_LOG(LS_ERROR) << "Received error from server";
    Close();
    callback_->OnDisconnected();
    return false;
  }

  *eoh = response.find("\r\n\r\n");
  RTC_DCHECK(*eoh != std::string::npos);
  if (*eoh == std::string::npos)
    return false;

  *peer_id = -1;

  // See comment in peer_channel.cc for why we use the Pragma header and
  // not e.g. "X-Peer-Id".
  GetHeaderValue(response, *eoh, "\r\nPragma: ", peer_id);

  return true;
}

void PeerConnectionClient::OnClose(rtc::AsyncSocket* socket, int err) {
  RTC_LOG(INFO) << __FUNCTION__;

  socket->Close();

#ifdef WIN32
  if (err != WSAECONNREFUSED) {
#else
  if (err != ECONNREFUSED) {
#endif
    if (socket == hanging_get_.get()) {
      if (state_ == CONNECTED) {
        hanging_get_->Close();
        hanging_get_->Connect(server_address_);
      }
    } else {
      callback_->OnMessageSent(err);
    }
  } else {
    if (socket == control_socket_.get()) {
      RTC_LOG(WARNING) << "Connection refused; retrying in 2 seconds";
      rtc::Thread::Current()->PostDelayed(RTC_FROM_HERE, kReconnectDelay, this,
                                          0);
    } else {
      Close();
      callback_->OnDisconnected();
    }
  }
}

void PeerConnectionClient::OnMessage(rtc::Message* msg) {
  // ignore msg; there is currently only one supported message ("retry")
  DoConnect();
}
