#ifndef SOCK_ITEM_H
#define SOCK_ITEM_H

#include <vector>

#define HEAD_LEN 8

class SubReactor;

class Sock_item {
  public:
    Sock_item() = default;
    Sock_item(int fd)
        : fd_(fd), sendbuf_end_(0), recvbuf_end_(0), max_sendbuf_size_(4096),
          max_recvbuf_size_(4096), close_flag_(false), sendbuf(4096),
          recvbuf(4096) {}
    ~Sock_item(){};

    int fd() { return fd_; }
    bool is_closed() { return close_flag_; }

    std::vector<char> sendbuf;
    std::vector<char> recvbuf;

    void sock_read();
    void sock_send();

    void set_head(int head);
    int send_head();
    int recv_head();
    // int get_length();
    int send_len() const { return sendbuf_end_; };
    int recv_len() const { return recvbuf_end_; };
    void pair_close();
    void buf_clear();

    static void bufcopy(Sock_item *Recv, Sock_item *Send);

  private:
    int fd_;
    int sendbuf_end_;
    int recvbuf_end_;
    int max_sendbuf_size_;
    int max_recvbuf_size_;
    bool close_flag_;
};
#endif