#ifdef DEBUG_LOG
#include <print>
#define GLOG(formatted_log, ...) \
{                                      \
    std::println(formatted_log __VA_OPT__(,) __VA_ARGS__);     \
}
#else
#define GLOG(formatted_log, ...) // do nothing
#endif