#include "../include/tcp_server.h"

//默认server自定义函数
std::function<void(tinyco::Socket*)> default_connection(
    [](tinyco::Socket* co_socket) {
      //管理生命周期
      std::unique_ptr<tinyco::Socket> connect_socket(co_socket);

      //设置缓冲区
      std::vector<char> buf;
      buf.resize(2048);
      while (1) {
        auto readNum = connect_socket->read((void*)&(buf[0]), buf.size());
        // std::string ok = "HTTP/1.0 200 OK\r\nServer:
        // minico/0.1.0\r\nContent-Type: text/html\r\n\r\n";
        if (readNum <= 0) {
          break;
        }
        connect_socket->send((void*)&(buf[0]), readNum);
      }
    });

void TcpServer::start_single(const char* ip, int port) {
  //用户没有注册自定义连接函数，使用默认的
  if (_on_server_connection == nullptr) {
    register_connection(default_connection);
  }

  //创建socket，进行服务器参数配置
  _single_listen_fd = new tinyco::Socket();
  if (_single_listen_fd->isUseful()) {
    _single_listen_fd->setTcpNoDelay(true);
    _single_listen_fd->setReuseAddr(true);
    _single_listen_fd->setReusePort(true);

    if (_single_listen_fd->bind(ip, port) < 0) {
      return;
    }

    //开始监听
    _single_listen_fd->listen();

    //保存服务器的ip和端口号
    if (ip != nullptr) {
      server_ip = ip;
    } else {
      server_ip = "any address";
    }
    server_port = port;
  }

  //开始运行server loop
  auto loop = std::bind(&TcpServer::single_server_loop, this);
  tinyco::co_go(loop);
  return;
}

void TcpServer::start_multi(const char* ip, int port) {
  int cnt = ::get_nprocs_conf();

  //用户没有注册自定义连接函数，使用默认的
  if (_on_server_connection == nullptr) {
    register_connection(default_connection);
  }

  //创建socket，进行服务器参数配置
  _multi_listen_fd = new tinyco::Socket[cnt];
  for (int i = 0; i < cnt; ++i) {
    if (_multi_listen_fd[i].isUseful()) {
      _multi_listen_fd[i].setTcpNoDelay(true);
      _multi_listen_fd[i].setReuseAddr(true);
      _multi_listen_fd[i].setReusePort(true);

      if (_multi_listen_fd[i].bind(ip, port) < 0) {
        return;
      }

      //开始监听
      _multi_listen_fd[i].listen();
    }

    //开始运行server loop
    auto loop = std::bind(&TcpServer::multi_server_loop, this, i);

    tinyco::co_go(loop, tinyco::parameter::coroutineStackSize, i);
  }
  return;
}

void TcpServer::single_server_loop() {
  while (true) {
    tinyco::Socket* conn = new tinyco::Socket(_single_listen_fd->accept());
    conn->setTcpNoDelay(true);

    //运行绑定的用户工作函数
    //为了防止内存泄漏,这里需要对conn进行管理 也就是用户自身进行管理
    auto user_connection = std::bind(*_on_server_connection, conn);
    tinyco::co_go(user_connection);
  }
  return;
}

void TcpServer::multi_server_loop(int thread_number) {
  while (true) {
    // conn即可以用来进行fd通信
    tinyco::Socket* conn =
        new tinyco::Socket(_multi_listen_fd[thread_number].accept());
    conn->setTcpNoDelay(true);

    //运行绑定的用户工作函数
    //为了防止内存泄漏,这里需要对conn进行管理 也就是用户自身进行管理
    auto user_connection = std::bind(*_on_server_connection, conn);
    tinyco::co_go(user_connection);
  }
  return;
}