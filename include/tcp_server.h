#pragma once

#include <sys/sysinfo.h>

#include <functional>
#include <memory>

#include "socket.h"
#include "tinyco_api.h"

/**
 * @brief 基于协程的tcp服务器 一个连接对应一个协程
 * 是一个基类,可以被支持不同业务的服务器复用
 */
class TcpServer {
 public:
  //用户注册的服务器的连接回调函数
  using connection_callback = std::function<void(tinyco::Socket*)>;

  TcpServer()
      : _single_listen_fd(nullptr),
        _multi_listen_fd(nullptr),
        server_ip(nullptr),
        server_port(-1){};

  virtual ~TcpServer() {
    delete _single_listen_fd;
    _single_listen_fd = nullptr;
    delete _multi_listen_fd;
    _multi_listen_fd = nullptr;
    delete server_ip;
    server_ip = nullptr;
  }

  //单线程模式下开始服务器运行
  void start_single(const char* ip, int port);

  //多线程模式下开始服务器运行
  void start_multi(const char* ip, int port);

  //注册连接函数的接口，这里选择使用一层转发
  inline void register_connection(const connection_callback& func) {
    this->register_connection(connection_callback(func));
  }

  inline void register_connection(connection_callback&& func) {
    _on_server_connection.reset(new connection_callback(std::move(func)));
  }

 private:
  //服务器单核工作函数loop
  void single_server_loop();

  //服务器多核工作函数loop
  void multi_server_loop(int thread_number);

  //保存客户端连接时触发的回调函数 这里不要保存tcp连接
  std::shared_ptr<connection_callback> _on_server_connection;

  //单线程下用于监听的socket
  tinyco::Socket* _single_listen_fd;

  //多线程下监听的socket队列
  tinyco::Socket* _multi_listen_fd;

  // 服务器的初始参数
  const char* server_ip;
  int server_port;
};