#pragma once
#ifdef DEBUG
#include <stdio.h>
#define GLOG_DEBUG(log, ...) \
    printf(log __VA_OPT__(, ) __VA_ARGS__);
#else
#define GLOG_DEBUG(log, ...)
#endif