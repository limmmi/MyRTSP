#ifndef EPOLL_MANAGER_H
#define EPOLL_MANAGER_H
#include <sys/epoll.h>
#include <unordered_map>
#include <functional>
#include <fcntl.h>
#include <vector>

class EpollManager {
public:
    // 构造函数：创建epoll文件描述符以及初始化事件数组
    EpollManager();
    
    // 析构函数：关闭epoll文件描述符，释放资源
    ~EpollManager();

    // epoll可监听的最大事件数量
    static const int MAX_EVENT_NUMBER = 1024;
    
    // epoll文件描述符
    int epoll_fd_;
    
    // epoll事件数组，用于存储epoll_wait返回的事件
    std::vector<struct epoll_event> events_;
    
    // 文件描述符到事件处理函数的映射表
    std::unordered_map<int, std::function<void()>> event_handlers_;

    // 向epoll实例添加文件描述符并注册监听事件
    // @param fd 要添加的文件描述符
    // @param events 要监听的事件类型（如EPOLLIN, EPOLLOUT等）
    void addfd(int fd, uint32_t events);
    
    // 为文件描述符注册事件处理回调函数
    // @param fd 文件描述符
    // @param handler 当对应文件描述符上有事件触发时执行的回调函数
    void add_event_handler(int fd, std::function<void()> handler);

    // 设置文件描述符为非阻塞模式
    // @param fd 要设置的文件描述符
    // @return 返回文件描述符的原始标志位
    int setnonblocking(int fd);
    
    // 进入epoll事件循环，持续监听并处理IO事件
    // 该函数会阻塞，在循环中调用epoll_wait等待事件，并触发对应的处理函数
    void loop();
};
const int EpollManager::MAX_EVENT_NUMBER = 1024;
#endif