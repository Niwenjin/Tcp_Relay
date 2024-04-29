#ifndef SOCK_ITEM_H
#define SOCK_ITEM_H

#include <signal.h>
#include <vector>

#define HEAD_LEN 8

class SubReactor;

class Sock_item {
  public:
    Sock_item() = default;
    Sock_item(int fd)
        : fd_(fd), buf_end_(0), max_buf_size_(2048), close_flag_(false) {
        signal(SIGPIPE, SIG_IGN);
        buf.resize(max_buf_size_);
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
    int buf_end_;
    int max_buf_size_;
    bool close_flag_;
};
#endif