#include "TcpClient.h"
#include "log.h"
// #include "time.h"
#include <arpa/inet.h>
#include <cstdlib>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_EVENTS 10000

TcpClient::TcpClient(int n, int len)
    : conn_(n), msglen_(len + 8), send_turn(0), recv_num(0) {
    send_msg.insert(send_msg.begin(), 4, '0');
    char bytes[4];
    bytes[3] = (len >> 24) & 0xFF;
    bytes[2] = (len >> 16) & 0xFF;
    bytes[1] = (len >> 8) & 0xFF;
    bytes[0] = len & 0xFF;
    send_msg.insert(send_msg.end(), bytes, bytes + 4);
    send_msg.insert(send_msg.end(), len, 'x');
    // DEBUG_LOG(send_msg.size());
    // DEBUG_LOG(*(int *)(send_msg.data() + 4));
}

TcpClient::~TcpClient() {
    if (epfd)
        close(epfd);
}

void TcpClient::init() {
    epfd = epoll_create1(0);
    if (epfd == -1) {
        DEBUG_LOG("epoll_create");
        exit(1);
    }
    if (fcntl(epfd, F_SETFL, fcntl(epfd, F_GETFL, 0) | O_NONBLOCK) == -1) {
        DEBUG_LOG("epoll fcntl");
    }
    connect_n();
}

void TcpClient::loop() {
    struct epoll_event events[MAX_EVENTS];
    char abort[10008];
    // START_TIMER();
    timer.start();
    while (1) {
        // 发送消息
        if (send_turn < 300) {
            for (int i = 0; i < conn_; i += 2) {
                // DEBUG_LOG(send_msg.data());
                int n = write(fds[i], send_msg.data(), msglen_);
                if (n == -1) {
                    DEBUG_LOG("write");
                    exit(1);
                }
            }
            send_turn++;
        }

        // 接收消息
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        // DEBUG_LOG("nfds: " << nfds);
        if (nfds == -1) {
            DEBUG_LOG("epoll_wait");
            exit(1);
        }
        if (nfds == 0) {
            // STOP_TIMER("");
            break;
        }
        // DEBUG_LOG(nfds);
        for (int i = 0; i < nfds; ++i) {
            int fd = events[i].data.fd;
            int n = read(fd, abort, sizeof(abort));
            // recv_num++;
            // if(recv_num==100){
            //     timer.print("接收100条消息");
            //     return;
            // }
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
        // if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK) == -1) {
        //     DEBUG_LOG("fcntl");
        // }
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