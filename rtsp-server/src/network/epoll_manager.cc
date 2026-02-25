#include "network/epoll_manager.h"
#include <stdexcept>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
EpollManager::EpollManager() : epoll_fd_(-1), events_(MAX_EVENT_NUMBER) {
    epoll_fd_ = epoll_create1(0);
    if (epoll_fd_ == -1) {
        throw std::runtime_error("Failed to create epoll fd");
    }
}

EpollManager::~EpollManager() {
    if (epoll_fd_ != -1) {
        close(epoll_fd_);
    }
}

void EpollManager::addfd(int fd, uint32_t events) {
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
        throw std::runtime_error("Failed to add fd to epoll");
    }
    setnonblocking(fd);
}

void EpollManager::add_event_handler(int fd, std::function<void()> handler) {
    event_handlers_[fd] = handler;
}

//对文件描述符设置非阻塞
int EpollManager::setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

void EpollManager::loop(){
    while (true) {
        int number = epoll_wait(epoll_fd_, events_.data(), MAX_EVENT_NUMBER, -1);
        if (number < 0 && errno != EINTR) {
            continue;
        }

        for (int i = 0; i < number; i++) {
            int sockfd = events_[i].data.fd;

            auto it = event_handlers_.find(sockfd);
            if (it != event_handlers_.end()) {
                it->second();
            }
        }
    }
}
