#include "../include/tcp_client.h"

//客户端的核心连接函数 需要用一个协程去连接 防止本函数阻塞
void TcpClient::connect(const char* ip, int port) {
  //调用client_socket的连接函数
  return m_client_socket->connect(ip, port);
}

int TcpClient::disconnect() {
  /** 关闭写 如果读到了0 就关掉*/
  return m_client_socket->shutdownWrite();
}

size_t TcpClient::recv(void* buf, size_t count) {
  return m_client_socket->read(buf, count);
}

size_t TcpClient::send(const void* buf, size_t count) {
  return m_client_socket->send(buf, count);
}