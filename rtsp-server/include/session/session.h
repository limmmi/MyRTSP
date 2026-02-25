#ifndef SESSION_H
#define SESSION_H

#include <string>
#include <memory>
#include <ctime>

/**
 * 会话状态枚举
 */
enum class SessionState {
    INIT,      // 初始化状态
    READY,     // 准备状态
    PLAYING,   // 播放中
    PAUSED,    // 暂停
    CLOSED     // 已关闭
};

// 前向声明
class Session;

// Session 智能指针类型别名
using SessionPtr = std::shared_ptr<Session>;

/**
 * Session 类：表示一个 RTSP 会话
 * 管理单个客户端播放会话的状态、端口分配和生命周期
 */
class Session {
public:
    // 构造函数：创建新会话
    // @param session_id 会话唯一标识符
    // @param resource_path 资源路径（如 "/video"）
    // @param client_ip 客户端IP地址
    // @param client_port 客户端端口
    Session(const std::string& session_id, const std::string& resource_path,
            const std::string& client_ip, uint16_t client_port);
    
    // 析构函数：释放会话资源
    ~Session();

    // 禁止拷贝构造和拷贝赋值
    Session(const Session&) = delete;
    Session& operator=(const Session&) = delete;

    // 获取会话ID
    // @return 返回会话ID字符串
    const std::string& get_session_id() const;
    
    // 获取资源路径
    // @return 返回资源路径字符串
    const std::string& get_resource_path() const;
    
    // 获取会话状态
    // @return 返回当前会话状态
    SessionState get_state() const;
    
    // 设置会话状态
    // @param state 新的会话状态
    void set_state(SessionState state);
    
    // 获取客户端IP地址
    // @return 返回客户端IP字符串
    const std::string& get_client_ip() const;
    
    // 获取客户端端口
    // @return 返回客户端端口号
    uint16_t get_client_port() const;
    
    // 设置视频RTP端口
    // @param port 端口号
    void set_video_rtp_port(uint16_t port);
    
    // 获取视频RTP端口
    // @return 返回端口号，未分配返回0
    uint16_t get_video_rtp_port() const;
    
    // 设置视频RTCP端口
    // @param port 端口号
    void set_video_rtcp_port(uint16_t port);
    
    // 获取视频RTCP端口
    // @return 返回端口号，未分配返回0
    uint16_t get_video_rtcp_port() const;
    
    // 设置音频RTP端口
    // @param port 端口号
    void set_audio_rtp_port(uint16_t port);
    
    // 获取音频RTP端口
    // @return 返回端口号，未分配返回0
    uint16_t get_audio_rtp_port() const;
    
    // 设置音频RTCP端口
    // @param port 端口号
    void set_audio_rtcp_port(uint16_t port);
    
    // 获取音频RTCP端口
    // @return 返回端口号，未分配返回0
    uint16_t get_audio_rtcp_port() const;
    
    // 更新最后活跃时间
    void update_last_active();
    
    // 获取最后活跃时间
    // @return 返回最后活跃时间戳
    time_t get_last_active() const;
    
    // 检查会话是否超时
    // @param timeout_sec 超时时间（秒）
    // @return 超时返回true，否则返回false
    bool is_timeout(int timeout_sec) const;
    
    // 获取会话状态的字符串表示
    // @return 返回状态字符串
    std::string get_state_string() const;

private:
    // 会话唯一标识符
    std::string session_id_;
    
    // 播放的资源路径（如 "/video"）
    std::string resource_path_;
    
    // 会话状态
    SessionState state_;
    
    // 视频RTP端口
    uint16_t video_rtp_port_;
    
    // 视频RTCP端口
    uint16_t video_rtcp_port_;
    
    // 音频RTP端口
    uint16_t audio_rtp_port_;
    
    // 音频RTCP端口
    uint16_t audio_rtcp_port_;
    
    // 客户端IP地址
    std::string client_ip_;
    
    // 客户端端口
    uint16_t client_port_;
    
    // 最后活跃时间（用于超时清理）
    time_t last_active_;
};

#endif // SESSION_H
