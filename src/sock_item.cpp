#include "sock_item.h"
#include "log.h"
#include <unistd.h>

#define MAX_READ_SIZE 1024

void Sock_item::sock_read() {
    int n = read(fd_, buf.data() + buf_end_, MAX_READ_SIZE);
    if (n == -1) {
        DEBUG_LOG("read in fd " << fd_);
        // exit(1);
    } else if (n == 0) {
        close_flag_ = true;
    } else {
        buf_end_ += n;
        if (buf_end_ * 2 > max_buf_size_) {
            max_buf_size_ *= 2;
            buf.resize(max_buf_size_);
        }
    }
}

void Sock_item::sock_send(int tofd) {
    int len = 8 + get_head();
    int n = write(tofd, buf.data(), len);
    // DEBUG_LOG("write bytes " << n);
    if (n == -1) {
        // DEBUG_LOG("write in fd " << fd_);
        close_flag_ = true;
        buf_clear();
        return;
    }
    buf_end_ -= len;
    buf.erase(buf.begin(), buf.begin() + len);
    buf.resize(max_buf_size_);
}

void Sock_item::set_head(int head) { *(int *)&buf[4] = head; }

int Sock_item::get_head() {
    return buf_end_ < 8 ? 0 : *(int *)(buf.data() + 4);
}

int Sock_item::get_length() { return buf_end_; }

void Sock_item::pair_close() {
    char msg[] = "you pair has closed.";
    buf.insert(buf.begin(), 8, 0);
    buf.insert(buf.begin() + 8, msg, msg + sizeof(msg) - 1);
    set_head(sizeof(msg) - 1);
    sock_send(fd_);
    buf_clear();
}

void Sock_item::buf_clear() {
    buf.clear();
    buf.resize(max_buf_size_);
    buf_end_ = 0;
    // buf.insert(buf.begin(), 8, 0);
}