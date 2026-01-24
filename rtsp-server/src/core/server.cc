#include "core/server.h"
#include "core/config.h"
#include "session/port_pool.h"
#include <thread>
#include <string>

// 初始化 监听套接字, 线程池, 日志系统, 认证系统, 会话管理, 媒体目录
void init(Config& config)
{
    // 基础
    m_port = config.rtsp_port;
    m_host = config.rtsp_host;
    
    // RTP(RTCP)端口范围
    m_port_min = config.port_min;
    m_port_max = config.port_max;

    // 认证相关
    m_auth_enabled = config.auth_enabled;
    m_auth_type = config.auth_type;
    m_jwt_secret = config.jwt_secret;
    m_jwt_expire = config.jwt_expire;
    m_jwt_refresh_enabled = config.jwt_refresh_enabled;
    m_jwt_refresh_interval = config.jwt_refresh_interval;

    // 日志相关
    m_log_enabled = config.log_enabled;
    m_log_to_file = config.log_to_file;
    m_log_to_console = config.log_to_console;
    m_log_dir = config.log_dir;
    m_log_level = config.log_level;
    m_log_file = config.log_file;
    m_log_max_size = config.log_max_size;
    m_log_max_files = config.log_max_files;

    // 会话管理, 端口池
    m_session_manager = new session_manager();
    m_udp_port_pool = new udp_port_pool(m_port_min, m_port_max);
    m_tcp_port_pool = new tcp_port_pool(m_port_min, m_port_max);

    // 线程池相关
    m_thread_num = config.thread_num;
    m_pool = new threadpool<connection>(m_thread_num);

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

}