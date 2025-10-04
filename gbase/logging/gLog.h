#if __cplusplus < 202302L
#warning This project requires C++23 or higher!
//TODO: need update logging for lower versions of cpp
#define GLOG(formatted_log, ...) // dull define
#define GLOG_ERROR(formatted_log, ...) // dull define
#else
#include <print>
#ifdef DEBUG_LOG
#define GLOG(formatted_log, ...)                                \
    {                                                           \
        std::println(formatted_log __VA_OPT__(, ) __VA_ARGS__); \
    }
#else
#define GLOG(formatted_log, ...) // do nothing
#endif

#define GLOG_ERROR(formatted_log, ...)                                                                \
    {                                                                                                 \
        std::println("ERROR at {}:{} " formatted_log, __FILE__, __LINE__ __VA_OPT__(, ) __VA_ARGS__); \
    }
#endif
