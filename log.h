#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#define __USE_GNU
#include <sys/syscall.h>
#include <unistd.h>

#ifdef HIDE_LOGS
#define log(format, ...)
#else
#define log(format, ...) fprintf(stdout, "LOG: [tid:%d] [%s]" format "\n", gettid(), __FUNCTION__, ##__VA_ARGS__)
#endif

#define info(format, ...) fprintf(stdout, "INFO: [tid:%d] [%s] " format "\n", gettid(), __FUNCTION__, ##__VA_ARGS__)
#define error(format, ...) fprintf(stderr, "ERROR: [tid:%d] [%s] " format "\n", gettid(), __FUNCTION__, ##__VA_ARGS__)

#endif