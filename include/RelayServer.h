#ifndef RELAY_SERVER_H
#define RELAY_SERVER_H
#include "sock_item.h"
// #include "timer.h"
#include <map>
#include <memory>

class RelayServer {
  public:
    RelayServer();
    RelayServer(int port);
    ~RelayServer();

    void loop();

  private:
    // init
    int listen_port;
    int listenfd;
    int epfd;
    int epoll_init();
    int listensock_init(int port);

    // epollctl
    void epoll_add(int fd, int events);
    void epoll_del(int fd, int events);

    // events_handler
    void listen_event();
    void read_event(int fd);
    void write_event(int fd);

    // pair
    int fdpair[2];
    int fdindex;
    void add_pair();
    void del_pair(int fd);
    std::map<int, std::shared_ptr<Sock_item>> sock_map;
    std::map<int, int> relay_map;

    // timer
    // Timer timer;
    // int relay_num = 0;
};
#endif