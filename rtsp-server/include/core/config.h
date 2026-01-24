#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <cstdint>

class Config {
public:
    static Config& GetInstance();

    // RTSP服务配置
    uint16_t rtsp_port = 8554;
    std::string rtsp_host = "0.0.0.0";

    // 端口范围
    uint16_t port_min = 10000;
    uint16_t port_max = 20000;

    // 连接配置
    int max_connections = 1000;
    int session_timeout = 60;
    int buffer_size = 8192;

    // 认证配置
    bool auth_enabled = false;
    std::string auth_type = "jwt";
    std::string jwt_secret;
    int jwt_expire = 3600;
    bool jwt_refresh_enabled = true;
    int jwt_refresh_interval = 3600;

    // 日志配置
    bool log_enabled = true;
    bool log_to_file = false;
    bool log_to_console = true;
    std::string log_level = "info";
    std::string log_dir = "./logs";
    std::string log_file = "rtsp-server.log";
    int log_max_size = 100;
    int log_max_files = 10;
   

    // 媒体目录
    std::string video_dir = "./media/videos";
    std::string audio_dir = "./media/audios";

    // 线程池配置
    int thread_pool_size = 8;
    int worker_threads = 4;

    // 配置文件路径
    std::string config_file = "../config/server_config.json";

    // 从命令行解析参数
    void ParseCommandLine(int argc, char** argv);

    // 从JSON文件加载配置
    bool LoadFromFile(const std::string& filename);

    // 保存到JSON文件
    bool SaveToFile(const std::string& filename);

private:
    ServerConfig() = default;
    ~ServerConfig() = default;
    ServerConfig(const ServerConfig&) = delete;
    ServerConfig& operator=(const ServerConfig&) = delete;
};

#endif