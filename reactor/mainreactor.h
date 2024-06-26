#ifndef MAINREACTOR_H
#define MAINREACTOR_H
#include "subreactor.h"
#include <memory>
#include <thread>
#include <vector>

using std::shared_ptr;
using std::thread;
using std::vector;

class MainReactor {
  public:
    MainReactor(int n);
    ~MainReactor();
    void run();

  private:
    int thread_num;
    int epfd;
    int listenfd;

    vector<thread> threads;
    vector<shared_ptr<SubReactor>> subreactors;

    int epoll_init();
    int listensock_init();
    void thread_init();
    static void thread_func(shared_ptr<SubReactor>);

    void epoll_add_listen();

    void accept_loop();

    int fdpair[2];
    int fdflag;

    int add_fdpair(int fd);
    void dispatch();

    static void sigHandler(int sig);
};

#endif