#include "RelayServer.h"
#include "log.h"
#include "sock_item.h"
#include <arpa/inet.h>
#include <cstdlib>
#include <fcntl.h>
#include <signal.h>
#include <sys/epoll.h>
#include <unistd.h>

#define MAX_EVENTS 30000

volatile sig_atomic_t stop = 0;

void sigHandler(int sig) { stop = 1; }

RelayServer::RelayServer()
    : listenfd(-1), epfd(-1), listen_port(6666), fdindex(0) {
    listenfd = listensock_init(listen_port);
    epfd = epoll_init();
    epoll_add(listenfd, EPOLLIN);
    signal(SIGINT, sigHandler);
    signal(SIGPIPE, SIG_IGN);
}

RelayServer::RelayServer(int port)
    : listenfd(-1), epfd(-1), listen_port(port), fdindex(0) {
    listenfd = listensock_init(listen_port);
    epfd = epoll_init();
    epoll_add(listenfd, EPOLLIN);
    signal(SIGINT, sigHandler);
    signal(SIGPIPE, SIG_IGN);
}

RelayServer::~RelayServer() {
    if (epfd != -1)
        close(epfd);
    if (listenfd != -1)
        close(listenfd);
}

void RelayServer::loop() {
    while (!stop) {
        struct epoll_event events[MAX_EVENTS];
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            DEBUG_LOG("epoll_wait");
            return;
        }
        for (int i = 0; i < nfds; i++) {
            if (events[i].events & EPOLLIN) {
                if (events[i].data.fd == listenfd) {
                    listen_event();
                } else {
                    read_event(events[i].data.fd);
                }
            } else if (events[i].events & EPOLLOUT) {
                write_event(events[i].data.fd);
            }
        }
    }
}

int RelayServer::listensock_init(int port) {
    // 服务器监听的端口号
    const int serverPort = port;

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        DEBUG_LOG("listen fd create");
    }

    int flags = fcntl(listenfd, F_GETFL, 0);
    if (flags == -1) {
        DEBUG_LOG("fcntl");
    }
    if (fcntl(listenfd, F_SETFL, flags | O_NONBLOCK) == -1) {
        DEBUG_LOG("fcntl");
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(serverPort);
    addr.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) ==
        -1) {
        DEBUG_LOG("setsockopt");
    }

    // bind
    if (bind(listenfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        DEBUG_LOG("bind");
        exit(1);
    }
    // listen
    if (listen(listenfd, 10) == -1) {
        DEBUG_LOG("listen");
        exit(1);
    }

    return listenfd;
}

int RelayServer::epoll_init() {
    int epfd = epoll_create1(0);
    if (epfd == -1) {
        DEBUG_LOG("epoll_create");
        exit(1);
    }
    return epfd;
}

void RelayServer::epoll_add(int fd, int events) {
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;

    // 读取当前的事件
    struct epoll_event current_ev;
    if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &current_ev) == 0) {
        ev.events |= current_ev.events;
    } else {
        if (errno == ENOENT) {
            // 说明该文件描述符还没有被添加到epoll中
            if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
                DEBUG_LOG("epoll_add");
            }
        } else {
            DEBUG_LOG("epoll_mod");
        }
    }
}

void RelayServer::epoll_del(int fd, int events) {
    if (events == 0) {
        // 完全移除文件描述符
        if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr) == -1) {
            DEBUG_LOG("epoll_del");
        }
    } else {
        // 获取当前的事件
        struct epoll_event current_ev;
        if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &current_ev) == 0) {
            // 移除指定的事件
            current_ev.events &= ~events;

            if (current_ev.events == 0) {
                // 如果移除之后没有事件需要监听，删除文件描述符
                if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr) == -1) {
                    DEBUG_LOG("epoll_del");
                }
            } else {
                // 否则，更新当前的事件
                if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &current_ev) == -1) {
                    DEBUG_LOG("epoll_mod");
                }
            }
        } else {
            if (errno != ENOENT) {
                DEBUG_LOG("epoll_mod");
            }
        }
    }
}

void RelayServer::listen_event() {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int clientfd = accept4(listenfd, (struct sockaddr *)&client_addr,
                           &client_len, SOCK_NONBLOCK);
    if (clientfd == -1) {
        DEBUG_LOG("accept");
        return;
    }

    fdpair[fdindex++] = clientfd;
    if (fdindex == 2) {
        add_pair();
        // DEBUG_LOG("add pair");
        fdindex = 0;
    }
}

void RelayServer::read_event(int fd) {
    if (sock_map.find(fd) == sock_map.end()) {
        return;
    }
    auto recvsock = sock_map[fd];
    if (recvsock->is_closed()) {
        // sock_map.erase(fd);
        epoll_del(fd, 0);
        if (relay_map.find(fd) != relay_map.end()) {
            int tofd = relay_map[fd];
            // 一对连接都已关闭
            if (sock_map.find(tofd) != sock_map.end() &&
                sock_map[tofd]->is_closed()) {
                del_pair(fd);
            }
            // relay_map.erase(fd);
            // relay_map.erase(tofd);
        }
        return;
    }
    // 判断转发目标是否关闭
    if (relay_map.find(fd) == relay_map.end()) {
        char abort[10008];
        read(fd, &abort, sizeof(abort)); // 丢弃数据
        recvsock->pair_close();
        return;
    }
    // 将socket中的数据读取到buf中
    recvsock->sock_read();

    // 未读完报文头部
    if (recvsock->recv_len() < HEAD_LEN) {
        return;
    }

    // 已经读到完整的报文
    // while (sock->get_length() >= HEAD_LEN + sock->get_head()) {
    //     int tofd = relay_map[fd];
    //     sock->sock_send(tofd);
    //     // relay_num++;
    // }
    while (recvsock->recv_len() >= HEAD_LEN + recvsock->recv_head()) {
        int tofd = relay_map[fd];
        auto sendsock = sock_map[tofd];
        // 将一个完整的包转移到发送缓冲区
        Sock_item::bufcopy(recvsock.get(), sendsock.get());
        // 关注发送fd的写事件
        epoll_add(tofd, EPOLLOUT);
    }
}

void RelayServer::write_event(int fd) {
    auto sendsock = sock_map[fd];
    while (sendsock->send_len() >= HEAD_LEN + sendsock->send_head()) {
        // 每次转发一条完整的报文
        sendsock->sock_send();
    }
    // 关闭可写事件
    epoll_del(fd, EPOLLOUT);
}

void RelayServer::add_pair() {
    int fd1 = fdpair[0], fd2 = fdpair[1];

    epoll_add(fd1, EPOLLIN);
    epoll_add(fd2, EPOLLIN);

    sock_map[fd1] = std::make_shared<Sock_item>(fd1);
    sock_map[fd2] = std::make_shared<Sock_item>(fd2);

    relay_map[fd1] = fd2;
    relay_map[fd2] = fd1;
}

void RelayServer::del_pair(int fd) {
    int tofd = relay_map[fd];
    relay_map.erase(fd);
    relay_map.erase(tofd);
    sock_map.erase(fd);
    sock_map.erase(tofd);
}