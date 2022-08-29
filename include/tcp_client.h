#pragma once

#include <sys/sysinfo.h>

#include <functional>
#include <memory>

#include "socket.h"
#include "tinyco_api.h"

/**
 * @brief: 客户端 一个客户端只用来保存一个连接 客户端必须在一个协程中运行
 */
class TcpClient {
 public:
  //进行socket的一系列初始化的工作
  TcpClient() : m_client_socket(new tinyco::Socket()) {}

  DISALLOW_COPY_MOVE_AND_ASSIGN(TcpClient);

  virtual ~TcpClient() {
    delete m_client_socket;
    m_client_socket = nullptr;
  }

  void connect(const char* ip, int port);

  // return 0 is success -1 is error
  int disconnect();
  size_t recv(void* buf, size_t count);
  size_t send(const void* buf, size_t count);

  inline int socket() const { return m_client_socket->fd(); }

 private:
  tinyco::Socket* m_client_socket;
};