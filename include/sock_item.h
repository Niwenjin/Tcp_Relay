#ifndef SOCK_ITEM_H
#define SOCK_ITEM_H

#include <signal.h>
#include <vector>

class SubReactor;

class Sock_item {
  public:
    Sock_item() = default;
    Sock_item(int fd) : fd_(fd), close_flag_(false) {
        signal(SIGPIPE, SIG_IGN);
    }
    ~Sock_item(){};

    int fd() { return fd_; }
    bool is_closed() { return close_flag_; }

    std::vector<char> buf;

    void sock_read();
    void sock_send(int tofd);

    void set_head(int head);
    int get_head();
    int get_length();
    void pair_close();
    void buf_clear();

  private:
    int fd_;
    bool close_flag_;
};
#endif