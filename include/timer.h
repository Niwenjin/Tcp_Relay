#ifndef TIMER_H
#define TIMER_H
#include <chrono>

class Timer {
  public:
    Timer() = default;
    ~Timer(){};
    void start();
    void print(const char *msg);

  private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
    std::chrono::time_point<std::chrono::high_resolution_clock> stop_;
    std::chrono::milliseconds duration_;
};
#endif