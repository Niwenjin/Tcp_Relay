#ifndef SUBREACTOR_H
#define SUBREACTOR_H
#include "sock_item.h"
#include <memory>
#include <unordered_map>

class SubReactor {
  public:
    SubReactor();
    ~SubReactor();
    void add_relay_pair(int fd1, int fd2);
    void run();

  private:
    int epfd;

    void relay_loop();
    void relay_event(int fd);

    int epoll_init();
    void epoll_add(int fd);
    void epoll_del(int fd);

    void del_relay_pair(int fd);

    std::unordered_map<int, std::shared_ptr<Sock_item>> sock_map;
    std::unordered_map<int, int> relay_map;
};
#endif