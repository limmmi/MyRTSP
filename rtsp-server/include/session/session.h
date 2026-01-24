
#ifndef SESSION_H
#define SESSION_H

class Session {
    std::string session_id_;      // 唯一ID
    std::string resource_path_;   // 播放的资源路径,如 "/video"
    SessionState state_;          // INIT/READY/PLAYING/PAUSED/CLOSED
    
    // 端口分配
    int video_rtp_port_;          // 视频RTP端口
    int video_rtcp_port_;         // 视频RTCP端口
    int audio_rtp_port_;          // 音频RTP端口
    int audio_rtcp_port_;         // 音频RTCP端口
    
    // 客户端信息
    std::string client_ip_;       // 客户端IP
    uint16_t client_port_;        // 客户端端口
    
    time_t last_active_;          // 最后活跃时间(用于超时清理)
};

#endif
