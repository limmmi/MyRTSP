#include "session/port_pool.h"

#include <algorithm>
#include <stdexcept>

PortPool::PortPool(uint16_t min_port, uint16_t max_port)
    : port_min_(min_port),
      port_count_(max_port - min_port + 1),
      first_free_(0) {

    // 初始化位图
    bitmap_size_ = (port_count_ + 31) / 32;
    bitmap_.resize(bitmap_size_, 0);

    // 预计算全1掩码用于边界检查
    if (port_count_ % 32 != 0) {
        uint32_t last_mask = (1U << (port_count_ % 32)) - 1;
        last_word_mask_ = last_mask;
    } else {
        last_word_mask_ = 0xFFFFFFFF;
    }
}

uint16_t PortPool::AllocatePort() {
    std::lock_guard<std::mutex> lock(mutex_);

    // 尝试从first_free_开始快速查找
    int start = first_free_.load(std::memory_order_relaxed);
    int index = FindFreeSlot(start);

    if (index >= 0) {
        // 成功找到空闲端口
        SetUsed(index, true);
        UpdateFirstFree(index + 1);
        return static_cast<uint16_t>(index + port_min_);
    }

    // 从开头重新搜索
    index = FindFreeSlot(0);
    if (index >= 0) {
        SetUsed(index, true);
        UpdateFirstFree(index + 1);
        return static_cast<uint16_t>(index + port_min_);
    }

    return 0;  // 0表示没有可用端口
}

void PortPool::ReleasePort(uint16_t port) {
    std::lock_guard<std::mutex> lock(mutex_);

    int index = static_cast<int>(port) - port_min_;
    if (index >= 0 && index < port_count_ && IsUsed(index)) {
        SetUsed(index, false);

        // 如果释放的端口在first_free_之前，更新first_free_
        if (index < first_free_) {
            first_free_.store(index, std::memory_order_relaxed);
        }
    }
}

void PortPool::Reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::fill(bitmap_.begin(), bitmap_.end(), 0);
    first_free_.store(0, std::memory_order_relaxed);
}

int PortPool::FindFreeSlot(int start_index) {
    if (start_index >= port_count_) {
        return -1;
    }

    int start_word = start_index / 32;
    int start_bit = start_index % 32;

    // 处理第一个不完整的字
    uint32_t mask = ~(0xFFFFFFFFU << start_bit);
    uint32_t word = bitmap_[start_word];

    if (start_word == bitmap_size_ - 1) {
        // 最后一个字
        uint32_t available = (~word) & last_word_mask_;
        if (available & mask) {
            int bit = FindFirstZeroBit(available & mask);
            if (bit >= 0) {
                return start_word * 32 + bit;
            }
        }
    } else {
        // 普通字
        uint32_t available = ~word;
        if (available & mask) {
            int bit = FindFirstZeroBit(available & mask);
            if (bit >= 0) {
                return start_word * 32 + bit;
            }
        }
    }

    // 搜索剩余的字
    for (int i = start_word + 1; i < bitmap_size_; ++i) {
        uint32_t word = bitmap_[i];

        if (i == bitmap_size_ - 1) {
            // 最后一个字
            uint32_t available = (~word) & last_word_mask_;
            if (available != 0) {
                int bit = FindFirstZeroBit(available);
                if (bit >= 0) {
                    return i * 32 + bit;
                }
            }
        } else {
            // 普通字
            if (word != 0xFFFFFFFF) {
                int bit = FindFirstZeroBit(~word);
                if (bit >= 0) {
                    return i * 32 + bit;
                }
            }
        }
    }

    return -1;  // 没有找到空闲槽位
}

int PortPool::FindFirstZeroBit(uint32_t value) {
    if (value == 0) return -1;

    // 使用编译器内置函数
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_ctz(value);  // 计算尾随零的数量
#elif defined(_MSC_VER)
    unsigned long index;
    if (_BitScanForward(&index, value)) {
        return static_cast<int>(index);
    }
    return -1;
#else
    // 通用实现
    for (int i = 0; i < 32; ++i) {
        if (value & (1U << i)) {
            return i;
        }
    }
    return -1;
#endif
}

bool PortPool::IsUsed(int index) const {
    int word = index / 32;
    int bit = index % 32;
    return (bitmap_[word] >> bit) & 1U;
}

void PortPool::SetUsed(int index, bool used) {
    int word = index / 32;
    int bit = index % 32;

    if (used) {
        bitmap_[word] |= (1U << bit);
    } else {
        bitmap_[word] &= ~(1U << bit);
    }
}

void PortPool::UpdateFirstFree(int start) {
    for (int i = start; i < port_count_; ++i) {
        if (!IsUsed(i)) {
            first_free_.store(i, std::memory_order_relaxed);
            return;
        }
    }
    first_free_.store(port_count_, std::memory_order_relaxed);
}
