#include "TcpClient.h"
#include "log.h"
#include <arpa/inet.h>
#include <cstdlib>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_EVENTS 10000

TcpClient::TcpClient(int n, int len)
    : conn_(n), msglen_(len + 8), send_flag(true) {
    send_msg.insert(send_msg.begin(), 4, 0);
    char len_bytes[4];
    len_bytes[0] = (len >> 24) & 0xFF;
    len_bytes[1] = (len >> 16) & 0xFF;
    len_bytes[2] = (len >> 8) & 0xFF;
    len_bytes[3] = len & 0xFF;
    send_msg.insert(send_msg.begin() + 4, len_bytes, len_bytes + 4);
    send_msg.insert(send_msg.begin() + 8, len, 'x');
}

TcpClient::~TcpClient() {
    if (epfd)
        close(epfd);
}

void TcpClient::init() {
    epfd = epoll_create(1);
    if (epfd == -1) {
        DEBUG_LOG("epoll_create");
        exit(1);
    }
    connect_n();
}

void TcpClient::loop() {
    struct epoll_event events[MAX_EVENTS];
    char abort[10008];
    while (1) {
        // 发送消息
        if (send_flag) {
            for (int i = 0; i < conn_; i+=2) {
                int n = write(fds[i], send_msg.data(), msglen_);
                if (n == -1) {
                    DEBUG_LOG("write");
                    exit(1);
                }
            }
        }
        // 接收消息
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            DEBUG_LOG("epoll_wait");
            exit(1);
        }
        for (int i = 0; i < nfds; ++i) {
            // TODO: send & recv message
            int fd = events[i].data.fd;
            int n = read(fd, abort, sizeof(abort));
        }
    }
}

void TcpClient::connect_n() {
    for (int i = 0; i < conn_; ++i) {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd == -1) {
            DEBUG_LOG("socket");
            exit(1);
        }
        fds.push_back(fd);
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(6666);
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
            DEBUG_LOG("connect");
            exit(1);
        }
        struct epoll_event ev;
        ev.events = EPOLLIN;
        ev.data.fd = fd;
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
            DEBUG_LOG("epoll_ctl");
            exit(1);
        }
    }
}