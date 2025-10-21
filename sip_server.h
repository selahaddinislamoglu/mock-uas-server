/**
 * @file sip_server.h
 * @brief Header for SIP server functionalities, including message processing and queue management.
 */

#ifndef SIP_SERVER_H
#define SIP_SERVER_H

#include <pthread.h>
#include "message_queue.h"

/**
 * @struct worker_thread_t
 * @brief Structure for worker thread and its associated message queue.
 */
typedef struct
{
    message_queue_t queue;
    pthread_t thread;
} worker_thread_t;

void *process_sip_messages(void *arg);

#endif // SIP_SERVER_H
