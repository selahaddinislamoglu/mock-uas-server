/**
 * @file message_queue.h
 * @brief Header for message queue structure and functions used in the SIP server.
 */

#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <pthread.h>

/**
 * @struct message_queue_t
 * @brief Structure for a thread-safe message queue used by the SIP server.
 */
typedef struct
{
    void **messages;
    int capacity;
    int size;
    int front;
    int rear;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} message_queue_t;

void initialize_message_queue(message_queue_t *queue, int capacity);
void destroy_message_queue(message_queue_t *queue);
int enqueue_message(message_queue_t *queue, void *message);
int dequeue_message(message_queue_t *queue, void **message);

#endif // MESSAGE_QUEUE_H