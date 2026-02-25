#ifndef SESSION_MANAGER_H_
#define SESSION_MANAGER_H_

#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include "session.h"

/**
 * SessionManager 类：管理所有 RTSP 会话
 * 提供会话的创建、查找、删除和超时清理功能
 */
class SessionManager {
public:
    // 构造函数
    SessionManager();
    
    // 析构函数
    ~SessionManager();

    // 禁止拷贝构造和拷贝赋值
    SessionManager(const SessionManager&) = delete;
    SessionManager& operator=(const SessionManager&) = delete;

    // 创建新会话
    // @param session_id 会话ID
    // @param resource_path 资源路径
    // @param client_ip 客户端IP
    // @param client_port 客户端端口
    // @return 成功返回Session对象指针，失败返回nullptr
    SessionPtr CreateSession(const std::string& session_id, const std::string& resource_path,
                            const std::string& client_ip, uint16_t client_port);
    
    // 删除会话
    // @param session_id 要删除的会话ID
    // @return 成功返回true，失败返回false
    bool RemoveSession(const std::string& session_id);
    
    // 获取会话
    // @param session_id 会话ID
    // @return 成功返回Session对象指针，失败返回nullptr
    SessionPtr GetSession(const std::string& session_id);
    
    // 清理超时会话
    // @param timeout_sec 超时时间（秒），超过此时间的会话将被清理
    // @return 返回清理的会话数量
    int CleanupTimeoutSessions(int timeout_sec);
    
    // 获取当前会话总数
    // @return 返回会话数量
    size_t GetSessionCount() const;
    
    // 检查会话是否存在
    // @param session_id 会话ID
    // @return 存在返回true，否则返回false
    bool HasSession(const std::string& session_id) const;

private:
    // Session ID 到 Session 对象的映射表
    std::unordered_map<std::string, SessionPtr> sessions_;
    
    // 互斥锁，保证线程安全
    mutable std::mutex mutex_;
};

#endif // SESSION_MANAGER_H_
