#include "sock_item.h"
#include "log.h"
#include <cstdio>
#include <unistd.h>

void Sock_item::sock_read() {
    size_t to_read = 1024;
    size_t old_size = buf.size();
    buf.resize(old_size + to_read); // 确保有足够的空间来读取数据
    int n = read(fd_, buf.data() + old_size, to_read);
    if (n == -1) {
        DEBUG_LOG("read in fd " << fd_);
        // exit(1);
    } else if (n == 0) {
        close_flag_ = true;
    } else {
        buf.resize(old_size + n);
    }
}

void Sock_item::sock_send(int tofd) {
    int len = 8 + get_head();
    int n = write(tofd, buf.data(), len);
    if (n == -1) {
        DEBUG_LOG("write in fd " << fd_);
        close_flag_ = true;
        buf_clear();
        return;
        // exit(1);
    }
    if (buf.size() == len) {
        buf_clear();
    } else {
        buf.erase(buf.begin(), buf.begin() + len);
    }
}

void Sock_item::set_head(int head) { *(int *)&buf[4] = head; }

int Sock_item::get_head() { return buf.size() >= 8 ? 0 : *(int *)&buf[4]; }

int Sock_item::get_length() { return buf.size(); }

void Sock_item::pair_close() {
    char msg[] = "you pair has closed.";
    buf_clear();
    buf.insert(buf.begin(), 8, 0);
    buf.insert(buf.begin() + 8, msg, msg + sizeof(msg) - 1);
    set_head(sizeof(msg) - 1);
    sock_send(fd_);
    buf_clear();
}

void Sock_item::buf_clear() {
    buf.clear();
    // buf.insert(buf.begin(), 8, 0);
}