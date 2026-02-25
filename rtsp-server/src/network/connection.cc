#include "network/connection.h"
#include <unistd.h>
#include <cstring>
#include <stdexcept>

Connection::Connection(int fd, const std::string& ip, int port)
    : fd_(fd), ip_(ip), port_(port), alive_(true) {
    if (fd_ < 0) {
        throw std::runtime_error("Invalid file descriptor");
    }
}

Connection::~Connection() {
    close();
}

int Connection::get_fd() const {
    return fd_;
}

const std::string& Connection::get_ip() const {
    return ip_;
}

int Connection::get_port() const {
    return port_;
}

int Connection::read_data(char* buffer, size_t size) {
    if (!alive_ || fd_ < 0) {
        return -1;
    }
    
    ssize_t bytes_read = read(fd_, buffer, size);
    if (bytes_read < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0;  // 非阻塞模式下暂时没有数据
        }
        return -1;
    } else if (bytes_read == 0) {
        alive_ = false;  // 连接已关闭
    }
    
    return static_cast<int>(bytes_read);
}

int Connection::write_data(const char* data, size_t size) {
    if (!alive_ || fd_ < 0) {
        return -1;
    }
    
    ssize_t bytes_written = write(fd_, data, size);
    if (bytes_written < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0;  // 非阻塞模式下发送缓冲区满
        }
        return -1;
    }
    
    return static_cast<int>(bytes_written);
}

void Connection::close() {
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
    alive_ = false;
}

bool Connection::is_alive() const {
    return alive_ && fd_ >= 0;
}
