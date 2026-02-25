#ifndef SOCKET_MANAGER_H
#define SOCKET_MANAGER_H

#include <string>
#include <memory>
#include "connection.h"

/**
 * SocketManager 类：管理网络套接字和监听端口
 * 负责创建、绑定和监听套接字，接受客户端连接
 */
class SocketManager {
public:
    // 构造函数
    SocketManager();
    
    // 析构函数：关闭所有资源
    ~SocketManager();
    
    // 禁止拷贝构造和拷贝赋值
    SocketManager(const SocketManager&) = delete;
    SocketManager& operator=(const SocketManager&) = delete;

    // 创建并绑定TCP套接字
    // @param port 要绑定的端口号
    // @param ip 要绑定的IP地址，默认为"0.0.0.0"表示监听所有网卡
    // @return 成功返回true，失败返回false
    bool bind_socket(int port, const std::string& ip = "0.0.0.0");
    
    // 开始监听连接请求
    // @param backlog 等待连接队列的最大长度，默认为128
    // @return 成功返回true，失败返回false
    bool listen_socket(int backlog = 128);
    
    // 接受一个新的客户端连接
    // @return 成功返回Connection对象的智能指针，失败返回nullptr
    std::shared_ptr<Connection> accept_connection();
    
    // 关闭监听套接字
    void close_socket();
    
    // 获取监听套接字的文件描述符
    // @return 返回文件描述符，未绑定返回-1
    int get_listen_fd() const;
    
    // 设置套接字为可复用地址（允许TIME_WAIT状态下绑定）
    // @param reuse 是否启用地址复用，默认为true
    // @return 成功返回true，失败返回false
    bool set_reuse_addr(bool reuse = true);

private:
    // 监听套接字的文件描述符
    int listen_fd_;
    
    // 监听的端口号
    int port_;
    
    // 监听的IP地址
    std::string ip_;
};

#endif
