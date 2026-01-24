#ifndef SERVER_H
#define SERVER_H

#include "config.h"
#include "network/connection.h"
#include "session/session_manager.h"
#include "session/port_pool.h"
#include <string>

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
    PortPool* m_tcp_port_pool;

    // 线程池相关
    threadpool<connection> *m_pool;
    int m_thread_num;

    // epoll相关
    int m_epollfd; 
    epoll_event events[MAX_EVENT_NUMBER];

    // 监听相关
    int m_listenfd;

    // 定时器相关
    int m_timer_fd;

    // 媒体目录
    std::string m_video_dir = "./media/videos";
    std::string m_audio_dir = "./media/audios";
};

#endif // SERVER_H
