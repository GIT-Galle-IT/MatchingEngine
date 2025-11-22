#pragma once
#if __cplusplus < 202302L
#warning This project requires C++23 or higher!
//TODO: need update logging for lower versions of cpp
#define GLOG(formatted_log, ...) // dull define
#define GLOG_ERROR(formatted_log, ...) // dull define
#else
#include <string>
#include <cstring>
constexpr const char *getFileName() {
    return (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__);
}

inline std::string FILENAME_ = getFileName();
#include <print>
#include <thread>
#ifdef DEBUG_LOG
#define GLOG_INFO(formatted_log, ...)                                   \
            {                                                           \
                std::println(formatted_log __VA_OPT__(, ) __VA_ARGS__); \
            }
#define GLOG_DEBUG_L1(formatted_log, ...)                                                 \
            {                                                                             \
                std::println("| {} | {}:{} |" formatted_log, std::this_thread::get_id(),  \
                            FILENAME_, __LINE__ __VA_OPT__(, ) __VA_ARGS__);           \
            }
#else
#define GLOG_INFO(formatted_log, ...) // do nothing
#define GLOG_DEBUG_L1(formatted_log, ...)
#endif
#define GLOG_ERROR(formatted_log, ...)                                                                          \
        {                                                                                                       \
            std::println("| ERROR | {}:{} |" formatted_log, FILENAME_, __LINE__ __VA_OPT__(, ) __VA_ARGS__); \
        }
#endif
