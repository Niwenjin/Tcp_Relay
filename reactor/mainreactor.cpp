#include "mainreactor.h"
#include "log.h"
#include <arpa/inet.h>
#include <cstdlib>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_EVENTS 10000

volatile sig_atomic_t stop = 0;

void MainReactor::sigHandler(int sig) { stop = 1; }

MainReactor::MainReactor(int n)
    : thread_num(n), epfd(-1), listenfd(-1), fdflag(0) {
    listenfd = listensock_init();
    epfd = epoll_init();

    epoll_add_listen();

    signal(SIGINT, sigHandler);

    // 创建n个子线程
    thread_init();
}

MainReactor::~MainReactor() {
    if (epfd != -1)
        close(epfd);
    if (listenfd != -1)
        close(listenfd);
    for (int i = 0; i < thread_num; i++)
        threads[i].join();
}

int MainReactor::epoll_init() {
    int epfd = epoll_create1(0);
    if (epfd == -1) {
        DEBUG_LOG("epoll_create");
        exit(1);
    }
    return epfd;
}

void MainReactor::thread_init() {
    for (int i = 0; i < thread_num; i++) {
        subreactors.push_back(std::make_shared<SubReactor>());
        threads.push_back(thread(thread_func, subreactors[i]));
    }
}

void MainReactor::thread_func(shared_ptr<SubReactor> subreactor) {
    subreactor->run();
}

int MainReactor::listensock_init() {
    // 服务器监听的端口号
    const int serverPort = 6666;

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

void MainReactor::epoll_add_listen() {
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = listenfd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev) == -1) {
        DEBUG_LOG("epoll_ctl");
        exit(1);
    }
}

void MainReactor::accept_loop() {
    while (!stop) {
        struct epoll_event events[10];
        // std::cout << "main_epfd: " << epfd << std::endl;
        int nfds = epoll_wait(epfd, events, 10, -1);
        if (nfds == -1) {
            DEBUG_LOG("listen epoll_wait");
            // exit(1);
        }

        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == listenfd) {
                struct sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                int clientfd =
                    accept4(listenfd, (struct sockaddr *)&client_addr,
                            &client_len, SOCK_NONBLOCK);
                if (clientfd == -1) {
                    DEBUG_LOG("accept");
                    exit(1);
                }

                if (add_fdpair(clientfd)) {
                    dispatch();
                    fdflag = 0;
                }
            }
        }
    }
}

int MainReactor::add_fdpair(int fd) {
    if (fdflag == 0) {
        fdpair[0] = fd;
        fdflag = 1;
        return 0;
    } else {
        fdpair[1] = fd;
        fdflag = 0;
        return 1;
    }
}

void MainReactor::dispatch() {
    int i = fdpair[0] % thread_num;
    subreactors[i]->add_relay_pair(fdpair[0], fdpair[1]);
    // std::cout << "add pair " << fdpair[0] << " & " << fdpair[1] << std::endl;
}

void MainReactor::run() { accept_loop(); }