
#ifndef SESSION_MANAGER_H_
#define SESSION_MANAGER_H_

class SessionManager {
    std::unordered_map<std::string, SessionPtr> sessions_;  // Session ID -> Session
    std::mutex mutex_;                                     // 线程安全
    
    SessionPtr CreateSession(const std::string& resource_path);
    bool RemoveSession(const std::string& session_id);
    SessionPtr GetSession(const std::string& session_id);
    void CleanupTimeoutSessions(int timeout_sec);  // 定时清理超时会话
};

#endif