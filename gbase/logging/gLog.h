#ifdef DEBUG_LOG
#include <iostream>
#define GLOG(formatted_log...) \
{                                      \
    printf(formatted_log);     \
}
#else
#define GLOG(formatted_log...) // do nothing
#endif