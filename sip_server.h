/**
 * @file sip_server.h
 * @brief Header for SIP server functionalities, including message processing and queue management.
 */

#ifndef SIP_SERVER_H
#define SIP_SERVER_H

#include <pthread.h>
#include "sip_message.h"

#define MAX_THREADS 5
#define QUEUE_CAPACITY 10
#define SIP_PORT 5060

/**
 * @struct message_queue_t
 * @brief Structure for a thread-safe message queue used by the SIP server.
 */
typedef struct
{
    sip_message_t **messages;
    int capacity;
    int size;
    int front;
    int rear;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} message_queue_t;

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
void initialize_message_queue(message_queue_t *queue, int capacity);
void destroy_message_queue(message_queue_t *queue);
int enqueue_message(message_queue_t *queue, sip_message_t *message);
int dequeue_message(message_queue_t *queue, sip_message_t **message);

#endif // SIP_SERVER_H
