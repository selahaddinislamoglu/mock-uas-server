#include "timer_manager.h"
#include "log.h"
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

typedef struct timer_s
{
    int tfd;
    timer_callback_t cb;
    void *user_data;
} timer_s;

static int epfd = -1;
static pthread_t timer_thread;
static int running = 0;

/**
 * @brief Timer thread function
 */
static void *timer_thread_func(void *arg)
{
    struct epoll_event events[32];
    while (running)
    {
        int n = epoll_wait(epfd, events, 32, 1000);
        if (n < 0)
            continue;

        for (int i = 0; i < n; i++)
        {
            timer_s *t = (timer_s *)events[i].data.ptr;
            uint64_t expirations;
            read(t->tfd, &expirations, sizeof(expirations));

            t->cb(t->user_data);

            epoll_ctl(epfd, EPOLL_CTL_DEL, t->tfd, NULL);
            close(t->tfd);
            free(t);
        }
    }
    return NULL;
}

/**
 * @brief Initializes the timer manager
 */
int timer_one_shot_init()
{
    epfd = epoll_create1(0);
    if (epfd < 0)
    {
        error("epoll_create1 failed: %s", strerror(errno));
        return -1;
    }
    running = 1;
    if (pthread_create(&timer_thread, NULL, timer_thread_func, NULL) != 0)
    {
        error("pthread_create failed: %s", strerror(errno));
        close(epfd);
        return -1;
    }
    return 0;
}

/**
 * @brief Shuts down the timer manager
 */
void timer_one_shot_shutdown()
{
    running = 0;
    pthread_join(timer_thread, NULL);
    close(epfd);
    epfd = -1;
}

/**
 * @brief Adds a one-shot timer to the timer manager
 */
int timer_one_shot_add(int initial_ms, timer_callback_t callback, void *user_data)
{
    if (epfd < 0)
        return -1;

    timer_s *t = malloc(sizeof(timer_s));
    if (!t)
        return -1;

    t->cb = callback;
    t->user_data = user_data;

    t->tfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (t->tfd < 0)
    {
        free(t);
        return -1;
    }

    struct itimerspec its;
    its.it_value.tv_sec = initial_ms / 1000;
    its.it_value.tv_nsec = (initial_ms % 1000) * 1000000;
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;

    if (timerfd_settime(t->tfd, 0, &its, NULL) < 0)
    {
        close(t->tfd);
        free(t);
        return -1;
    }

    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLIN;
    ev.data.ptr = t;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, t->tfd, &ev) < 0)
    {
        close(t->tfd);
        free(t);
        return -1;
    }

    return 0;
}
