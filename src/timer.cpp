#include "timer.h"
#include <iostream>

void Timer::start() {
    start_ = std::chrono::high_resolution_clock::now();
}

void Timer::print(const char *msg) {
    stop_ = std::chrono::high_resolution_clock::now();
    duration_ = std::chrono::duration_cast<std::chrono::milliseconds>(stop_ - start_);
    std::cerr << msg << " 用时: " << duration_.count() << " 毫秒\n";
}