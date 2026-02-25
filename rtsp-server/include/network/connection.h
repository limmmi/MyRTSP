#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>
#include <memory>

/**
 * Connection 类：表示一个客户端连接
 * 管理单个客户端连接的状态、数据收发和生命周期
 */
class Connection {
public:
    // 构造函数：创建新的连接对象
    // @param fd 连接的文件描述符
    // @param ip 客户端IP地址
    // @param port 客户端端口号
    Connection(int fd, const std::string& ip, int port);
    
    // 析构函数：关闭连接，释放资源
    ~Connection();
    
    // 禁止拷贝构造和拷贝赋值
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    // 获取连接的文件描述符
    // @return 返回文件描述符
    int get_fd() const;
    
    // 获取客户端IP地址
    // @return 返回IP地址字符串
    const std::string& get_ip() const;
    
    // 获取客户端端口号
    // @return 返回端口号
    int get_port() const;
    
    // 从连接读取数据
    // @param buffer 接收数据的缓冲区
    // @param size 缓冲区大小
    // @return 实际读取的字节数，-1表示读取失败
    int read_data(char* buffer, size_t size);
    
    // 向连接写入数据
    // @param data 要发送的数据
    // @param size 数据大小
    // @return 实际写入的字节数，-1表示写入失败
    int write_data(const char* data, size_t size);
    
    // 关闭连接
    void close();
    
    // 检查连接是否有效
    // @return 连接有效返回true，否则返回false
    bool is_alive() const;

private:
    // 连接的文件描述符
    int fd_;
    
    // 客户端IP地址
    std::string ip_;
    
    // 客户端端口号
    int port_;
    
    // 连接是否有效
    bool alive_;
};

#endif
