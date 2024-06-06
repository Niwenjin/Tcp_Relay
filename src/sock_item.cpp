#include "sock_item.h"
#include "log.h"
#include <algorithm>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_READ_SIZE 4096

void Sock_item::sock_read() {
    int n = recv(fd_, recvbuf.data() + recvbuf_end_, MAX_READ_SIZE, 0);
    if (n == -1) {
        DEBUG_LOG("read in fd " << fd_);
        // exit(1);
    } else if (n == 0) {
        close_flag_ = true;
    } else {
        recvbuf_end_ += n;
        if (recvbuf_end_ * 2 > max_recvbuf_size_) {
            max_recvbuf_size_ *= 2;
            recvbuf.resize(max_recvbuf_size_);
        }
    }
}

void Sock_item::sock_send() {
    int sendlen = send_head() + HEAD_LEN;
    int sendn = sendlen;
    while (sendn > 0) {
        int n = send(fd_, sendbuf.data(), sendn, 0);
        if (n == -1) {
            if (errno == EPIPE) {
                // 对端已关闭
                close_flag_ = true;
                return;
            }
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 写缓冲区已满
                break;
            } else {
                DEBUG_LOG("send in fd " << fd_ << " errno: " << errno);
                // exit(1);
            }
            sendn -= n;
        }
    }
    sendbuf.erase(sendbuf.begin(), sendbuf.begin() + sendlen);
    sendbuf.resize(max_sendbuf_size_);
}

void Sock_item::set_head(int head) { *(int *)&recvbuf[4] = head; }

int Sock_item::send_head() {
    return sendbuf_end_ < 8 ? 0 : *(int *)(sendbuf.data() + 4);
}

int Sock_item::recv_head() {
    return recvbuf_end_ < 8 ? 0 : *(int *)(recvbuf.data() + 4);
}

// int Sock_item::get_length() { return recvbuf_end_; }

void Sock_item::pair_close() {
    char msg[] = "you pair has closed.";
    // 对端关闭连接
}

void Sock_item::buf_clear() {
    // buf.clear();
    // buf.resize(max_buf_size_);
    // buf_end_ = 0;
    // buf.insert(buf.begin(), 8, 0);
}

void Sock_item::bufcopy(Sock_item *Recv, Sock_item *Send) {
    // 检测是否需要扩容
    if (Send->sendbuf_end_ + Recv->recvbuf_end_ > Send->max_sendbuf_size_) {
        Send->max_sendbuf_size_ *= 2;
        Send->sendbuf.resize(Send->max_sendbuf_size_);
    }
    // 将一个完整的报文复制到Send的缓冲区
    int copy_size = Recv->recv_head() + HEAD_LEN;
    std::copy(Recv->recvbuf.begin(), Recv->recvbuf.begin() + copy_size,
              Send->sendbuf.begin() + Send->sendbuf_end_);

    Send->sendbuf_end_ += copy_size;
    Recv->recvbuf_end_ -= copy_size;
    Recv->recvbuf.erase(Recv->recvbuf.begin(),
                        Recv->recvbuf.begin() + copy_size);
    Recv->recvbuf.resize(Recv->max_recvbuf_size_);
}