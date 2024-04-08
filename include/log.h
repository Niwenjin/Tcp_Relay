#ifndef __LOG_H__
#define __LOG_H__
#include <iostream>
#include <chrono>

// #define NDEBUG
// 如果定义了EN_DEBUG，则开启日志和计时
#ifndef NDEBUG
#define EN_LOG
// #define EN_TIMER
#endif

// 如果定义了DEBUG_LOG，则输出DEBUG_LOG信息
#ifdef EN_LOG
#define DEBUG_LOG(msg) std::cerr << "DEBUG: " << msg << '\n'
#else
#define DEBUG_LOG(msg)
#endif

// 如果定义了EN_TIMER，则输出DEBUG_LOG信息
#ifdef EN_TIMER
#define START_TIMER() auto start = std::chrono::high_resolution_clock::now()
#define STOP_TIMER(msg)                                                      \
    auto stop = std::chrono::high_resolution_clock::now();                   \
    auto duration =                                                          \
        std::chrono::duration_cast<std::chrono::milliseconds>(stop - start); \
    if (duration.count() >= 15)                                              \
    std::cerr << msg << " 用时: " << duration.count() << " 毫秒\n"

#else
#define START_TIMER()
#define STOP_TIMER(msg)
#endif

#endif