#include "subreactor.h"
#include "log.h"
#include "sock_item.h"
#include <cstdio>
#include <cstdlib>
#include <sys/epoll.h>
#include <unistd.h>

#define MAX_EVENTS 10000
#define HEAD_LEN 8

SubReactor::SubReactor() : epfd(-1) { epfd = epoll_init(); }

SubReactor::~SubReactor() {
    if (epfd != -1)
        close(epfd);
}

int SubReactor::epoll_init() {
    int epfd = epoll_create1(0);
    if (epfd == -1) {
        perror("epoll_create");
        exit(1);
    }
    return epfd;
}

void SubReactor::add_relay_pair(int fd1, int fd2) {
    epoll_add(fd1);
    epoll_add(fd2);

    sock_map[fd1] = std::make_shared<Sock_item>(fd1);
    sock_map[fd2] = std::make_shared<Sock_item>(fd2);

    relay_map[fd1] = fd2;
    relay_map[fd2] = fd1;
}

void SubReactor::run() { relay_loop(); }

void SubReactor::relay_loop() {
    while (1) {
        struct epoll_event events[MAX_EVENTS];
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("sub_epoll_wait");
            exit(1);
        }

        for (int i = 0; i < nfds; i++) {
            int fd = events[i].data.fd;
            // 转发
            relay_event(fd);
        }
    }
}

void SubReactor::relay_event(int fd) {
    std::shared_ptr<Sock_item> sock = sock_map[fd];
    if (sock->is_closed()) {
        sock_map.erase(fd);
        epoll_del(fd);
        if (relay_map.find(fd) != relay_map.end()) {
            int tofd = relay_map[fd];
            relay_map.erase(fd);
            relay_map.erase(tofd);
        }
        return;
    }
    // 判断转发目标是否关闭
    if (relay_map.find(fd) == relay_map.end()) {
        char abort[10008];
        read(fd, &abort, sizeof(abort)); // 丢弃数据
        sock->pair_close();
        return;
    }
    // 将socket中的数据读取到buf中
    sock->sock_read();

    // 未读完报文头部
    if (sock->get_length() < HEAD_LEN) {
        DEBUG_LOG(sock->get_length());
        DEBUG_LOG("head not enough");
        return;
    }

    int head = sock->get_head();
    // DEBUG_LOG("head" << head);
    // DEBUG_LOG("buf_len" << sock->get_length());
    // assert(head != 0);

    // 已经读到完整的报文
    while (sock->get_length() >= HEAD_LEN + head) {
        int tofd = relay_map[fd];
        sock->sock_send(tofd);
        head = sock->get_head();
    }
}

void SubReactor::epoll_add(int fd) {
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = fd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        perror("epoll_ctl");
        exit(1);
    }
}

void SubReactor::epoll_del(int fd) {
    if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL) == -1) {
        DEBUG_LOG("epoll_ctl");
    }
}

void SubReactor::del_relay_pair(int fd) {
    if (fd) {
        close(fd);
        sock_map.erase(fd);
        relay_map.erase(fd);
    }
    if (relay_map.find(fd) != relay_map.end()) {
        int pair_fd = relay_map[fd];
        close(pair_fd);
        sock_map.erase(pair_fd);
        relay_map.erase(pair_fd);
    }
    return;
}