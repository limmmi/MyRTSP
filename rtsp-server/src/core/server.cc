#include "core/server.h"
#include "network/socket_manager.h"
#include <stdexcept>
#include <cstring>
#include <signal.h>
#include <sys/timerfd.h>
#include <spdlog/spdlog.h>

Server::Server()
    : m_port(554), m_host("0.0.0.0"), m_port_min(10000), m_port_max(20000),
      m_session_manager(nullptr), m_udp_port_pool(nullptr),
      m_pool(nullptr), m_thread_num(4), m_epoll_manager(nullptr),
      m_rtsp_fd(-1), m_timer_fd(-1) {
}

Server::~Server() {
    if (m_epoll_manager) {
        delete m_epoll_manager;
    }
    if (m_session_manager) {
        delete m_session_manager;
    }
    if (m_udp_port_pool) {
        delete m_udp_port_pool;
    }
    if (m_pool) {
        delete m_pool;
    }
    if (m_rtsp_fd >= 0) {
        close(m_rtsp_fd);
    }
    if (m_timer_fd >= 0) {
        close(m_timer_fd);
    }
}

void Server::init(Config& config) {
    // 基础配置
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

    // 初始化会话管理器和端口池
    m_session_manager = new SessionManager();
    m_udp_port_pool = new PortPool(m_port_min, m_port_max);

    // 初始化线程池
    m_thread_num = config.thread_num;
    m_pool = new ThreadPool(m_thread_num);

    // 媒体目录
    m_video_dir = config.video_dir;
    m_audio_dir = config.audio_dir;

    spdlog::info("Server initialized on {}:{}", m_host, m_port);
}

void Server::eventListen() {
    try {
        // 创建 EpollManager
        m_epoll_manager = new EpollManager();
        
        // 创建 SocketManager
        SocketManager socket_manager;
        
        // 绑定并监听端口
        if (!socket_manager.bind_socket(m_port, m_host)) {
            throw std::runtime_error("Failed to bind socket on port " + std::to_string(m_port));
        }
        
        if (!socket_manager.listen_socket(128)) {
            throw std::runtime_error("Failed to listen socket");
        }
        
        m_rtsp_fd = socket_manager.get_listen_fd();
        
        // 添加监听fd到epoll
        m_epoll_manager->addfd(m_rtsp_fd, EPOLLIN);
        
        // 注册监听fd的事件处理器
        m_epoll_manager->add_event_handler(m_rtsp_fd, [this]() {
            this->handleNewConnection();
        });
        
        // 创建定时器fd用于会话超时清理
        m_timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
        if (m_timer_fd < 0) {
            spdlog::error("Failed to create timer fd");
            return;
        }
        
        // 设置定时器（每60秒清理一次超时会话）
        struct itimerspec timer_spec;
        timer_spec.it_interval.tv_sec = 60;
        timer_spec.it_interval.tv_nsec = 0;
        timer_spec.it_value.tv_sec = 60;
        timer_spec.it_value.tv_nsec = 0;
        
        if (timerfd_settime(m_timer_fd, 0, &timer_spec, nullptr) < 0) {
            spdlog::error("Failed to set timer");
            close(m_timer_fd);
            m_timer_fd = -1;
            return;
        }
        
        // 添加定时器fd到epoll
        m_epoll_manager->addfd(m_timer_fd, EPOLLIN);
        
        // 注册定时器的事件处理器
        m_epoll_manager->add_event_handler(m_timer_fd, [this]() {
            this->handleTimerEvent();
        });
        
        // 忽略 SIGPIPE 信号
        signal(SIGPIPE, SIG_IGN);
        
        spdlog::info("Server listening on port {}", m_port);
        
    } catch (const std::exception& e) {
        spdlog::error("Event listen failed: {}", e.what());
        throw;
    }
}

void Server::run() {
    spdlog::info("Server running...");
    
    // 进入epoll事件循环
    m_epoll_manager->loop();
}

void Server::handleNewConnection() {
    sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    
    int client_fd = accept(m_rtsp_fd, (sockaddr*)&client_addr, &addr_len);
    if (client_fd < 0) {
        spdlog::error("Failed to accept new connection");
        return;
    }
    
    // 设置为非阻塞
    int flags = fcntl(client_fd, F_GETFL, 0);
    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
    
    // 获取客户端IP
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    
    spdlog::info("New connection from {}:{}", client_ip, ntohs(client_addr.sin_port));
    
    // 创建Connection对象
    auto connection = std::make_shared<Connection>(client_fd, client_ip, ntohs(client_addr.sin_port));
    
    // 添加到epoll
    m_epoll_manager->addfd(client_fd, EPOLLIN | EPOLLET | EPOLLRDHUP);
    
    // 注册连接的事件处理器
    m_epoll_manager->add_event_handler(client_fd, [this, connection]() {
        this->handleClientRequest(connection);
    });
}

void Server::handleClientRequest(std::shared_ptr<Connection> connection) {
    char buffer[4096];
    int len = connection->read_data(buffer, sizeof(buffer) - 1);
    
    if (len <= 0) {
        // 连接关闭或出错
        if (len == 0) {
            spdlog::info("Client {}:{} disconnected", 
                        connection->get_ip(), connection->get_port());
        } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
            spdlog::error("Read error from client {}:{}", 
                         connection->get_ip(), connection->get_port());
        }
        connection->close();
        return;
    }
    
    buffer[len] = '\0';
    
    // 解析RTSP请求（TODO: 实现RTSP解析器）
    spdlog::debug("Received from {}:{}: {}", 
                 connection->get_ip(), connection->get_port(), buffer);
    
    // TODO: 调用RTSP解析器处理请求
    // std::string response = parseRtspRequest(buffer, connection);
    // connection->write_data(response.c_str(), response.size());
}

void Server::handleTimerEvent() {
    uint64_t expirations;
    ssize_t s = read(m_timer_fd, &expirations, sizeof(expirations));
    
    if (s != sizeof(expirations)) {
        return;
    }
    
    // 清理超时会话（超时时间：300秒）
    int timeout_sec = 300;
    int cleaned = m_session_manager->CleanupTimeoutSessions(timeout_sec);
    
    if (cleaned > 0) {
        spdlog::info("Cleaned {} timeout sessions", cleaned);
    }
}
