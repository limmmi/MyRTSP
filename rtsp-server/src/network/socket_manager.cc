#include "network/socket_manager.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdexcept>
#include <cstring>

SocketManager::SocketManager() : listen_fd_(-1), port_(0), ip_("0.0.0.0") {
}

SocketManager::~SocketManager() {
    close_socket();
}

bool SocketManager::bind_socket(int port, const std::string& ip) {
    // 创建TCP套接字
    listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd_ < 0) {
        return false;
    }

    // 设置地址复用
    int reuse = 1;
    if (setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        close(listen_fd_);
        listen_fd_ = -1;
        return false;
    }

    // 绑定地址和端口
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    
    if (ip == "0.0.0.0" || ip.empty()) {
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    } else {
        if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0) {
            close(listen_fd_);
            listen_fd_ = -1;
            return false;
        }
    }

    if (bind(listen_fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(listen_fd_);
        listen_fd_ = -1;
        return false;
    }

    port_ = port;
    ip_ = ip;
    return true;
}

bool SocketManager::listen_socket(int backlog) {
    if (listen_fd_ < 0) {
        return false;
    }

    if (listen(listen_fd_, backlog) < 0) {
        return false;
    }

    return true;
}

std::shared_ptr<Connection> SocketManager::accept_connection() {
    if (listen_fd_ < 0) {
        return nullptr;
    }

    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    int client_fd = accept(listen_fd_, (struct sockaddr*)&client_addr, &addr_len);
    if (client_fd < 0) {
        return nullptr;
    }

    // 获取客户端IP和端口
    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, INET_ADDRSTRLEN);
    int port = ntohs(client_addr.sin_port);

    return std::make_shared<Connection>(client_fd, ip_str, port);
}

void SocketManager::close_socket() {
    if (listen_fd_ >= 0) {
        close(listen_fd_);
        listen_fd_ = -1;
    }
}

int SocketManager::get_listen_fd() const {
    return listen_fd_;
}

bool SocketManager::set_reuse_addr(bool reuse) {
    if (listen_fd_ < 0) {
        return false;
    }

    int opt = reuse ? 1 : 0;
    return setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == 0;
}
