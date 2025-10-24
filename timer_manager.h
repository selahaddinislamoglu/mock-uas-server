#ifndef TIMER_MANAGER_H
#define TIMER_MANAGER_H

typedef void (*timer_callback_t)(void *user_data);

int timer_one_shot_init();
void timer_one_shot_shutdown();
int timer_one_shot_add(int initial_ms, timer_callback_t callback, void *user_data);

#endif
