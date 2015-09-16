#pragma once

#include <sys/time.h>

inline unsigned long long GetSystemTimeMilis( ) {
    #ifdef __APPLE__
    struct timeval now;
    gettimeofday( &now, NULL );
    return
        (unsigned long long) now.tv_usec/1000 +
        (unsigned long long) now.tv_sec*1000;

    #else
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return
        (unsigned long long) now.tv_nsec/1000000. +
        (unsigned long long) now.tv_sec*1000;

    #endif
}
