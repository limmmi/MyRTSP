#ifndef SERVER_H
#define SERVER_H

#include "network/connection.h"
#include "network/epoll_manager.h"
#include "session/session_manager.h"
#include "session/port_pool.h"
#include "utils/thread_pool.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cassert>
#include <sys/epoll.h>
#include "core/config.h"
#include <thread>
#include <string>
#include <strings.h>

/* RTSP Server */
const int MAX_FD = 65536;           //最大文件描述符
const int MAX_EVENT_NUMBER = 10000; //最大事件数
const int TIMESLOT = 60;             //最小超时单位

class Server {
public:
    Server();
    ~Server();

    // 初始化 监听套接字, 线程池, 日志系统, 认证系统, 会话管理, 媒体目录
    void init(Config& config);

    // 创建监听套接字和epoll
    void eventListen();

    // 循环 处理事件
    void run();

private:
    // 处理新连接
    void handleNewConnection();
    
    // 处理客户端请求
    void handleClientRequest(std::shared_ptr<Connection> connection);
    
    // 处理定时器事件（清理超时会话）
    void handleTimerEvent();

public:
    // 基础 
    uint16_t m_port;
    std::string m_host;
    
    // 端口范围
    uint16_t m_port_min= 10000;
    uint16_t m_port_max = 20000;

    // 认证相关
    bool m_auth_enabled = false;
    std::string m_auth_type = "jwt";
    std::string m_jwt_secret;
    int m_jwt_expire = 3600;
    bool m_jwt_refresh_enabled = true;
    int m_jwt_refresh_interval;

    // 日志相关
    bool m_log_enabled = true;
    bool m_log_to_file = false;
    bool m_log_to_console = true;
    std::string m_log_dir = "./logs";
    std::string m_log_level = "info";
    std::string m_log_file = "rtsp.log";
    int m_log_max_size = 1024 * 1024 * 1024;
    int m_log_max_files = 10;

    // 会话管理, 端口池
    SessionManager* m_session_manager;
    PortPool* m_udp_port_pool;

    // 线程池相关
    ThreadPool* m_pool;
    int m_thread_num;

    // epoll相关
    EpollManager* m_epoll_manager;

    // 监听相关
    int m_rtsp_fd;

    // 定时器相关
    int m_timer_fd;

    // 媒体目录
    std::string m_video_dir = "./media/videos";
    std::string m_audio_dir = "./media/audios";
};

#endif // SERVER_H
