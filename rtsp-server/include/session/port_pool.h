#ifndef PORT_POOL_H
#define PORT_POOL_H

#include <vector>
#include <mutex>
#include <atomic>
#include <cstdint>

/*
线程安全 端口存储池
*/
class PortPool {
public:
    PortPool(uint16_t min_port, uint16_t max_port);

    // 分配端口
    uint16_t AllocatePort();

    // 释放端口
    void ReleasePort(uint16_t port);

    // 重置所有端口为空闲
    void Reset();

private:
    std::vector<uint32_t> bitmap_;
    uint16_t port_min_;
    int port_count_;
    int bitmap_size_;
    uint32_t last_word_mask_;  // 最后一个字的掩码

    // 线程安全相关
    mutable std::mutex mutex_;
    std::atomic<int> first_free_;  // 使用原子操作提高性能

    // 查找空闲槽位
    int FindFreeSlot(int start_index);

    // 找到第一个0位的位置
    int FindFirstZeroBit(uint32_t value);

    // 检查端口是否被使用
    bool IsUsed(int index) const;

    // 设置端口使用状态
    void SetUsed(int index, bool used);

    // 更新first_free_指针
    void UpdateFirstFree(int start);

    // 禁止拷贝
    PortPool(const PortPool&) = delete;
    PortPool& operator=(const PortPool&) = delete;
};

#endif // PORT_POOL_H