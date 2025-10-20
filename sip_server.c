/**
 * @file sip_server.c
 * @brief Implementation of SIP server functionalities, including message processing and queue management.
 */

#include "sip_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Initializes a message queue.
 * @param queue Pointer to the message queue to initialize.
 * @param capacity The maximum number of messages the queue can hold.
 */
void initialize_message_queue(message_queue_t *queue, int capacity) {
    queue->messages = malloc(sizeof(sip_message_t*) * capacity);
    queue->capacity = capacity;
    queue->size = 0;
    queue->front = 0;
    queue->rear = -1;
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->cond, NULL);
}

/**
 * @brief Destroys a message queue, freeing its resources.
 * @param queue Pointer to the message queue to destroy.
 */
void destroy_message_queue(message_queue_t *queue) {
    for (int i = 0; i < queue->size; i++) {
        free(queue->messages[(queue->front + i) % queue->capacity]);
    }
    free(queue->messages);
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->cond);
}

/**
 * @brief Enqueues a message into the queue.
 * @param queue Pointer to the message queue where the message will be enqueued.
 * @param message Pointer to the message to enqueue.
 * @return 1 on success, 0 if the queue is full.
 */
int enqueue_message(message_queue_t *queue, sip_message_t *message) {
    pthread_mutex_lock(&queue->mutex);
    if (queue->size == queue->capacity) {
        pthread_mutex_unlock(&queue->mutex);
        return 0;
    }

    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->messages[queue->rear] = message;
    queue->size++;

    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->mutex);
    return 1;
}

/**
 * @brief Dequeues a message from the queue.
 * @param queue Pointer to the message queue to dequeue from.
 * @param message Double pointer to store the dequeued message.
 * @return 1 on success, 0 if the queue is empty.
 */
int dequeue_message(message_queue_t *queue, sip_message_t **message) {
    pthread_mutex_lock(&queue->mutex);

    while (queue->size == 0) {
        pthread_cond_wait(&queue->cond, &queue->mutex);
    }

    *message = queue->messages[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;

    pthread_mutex_unlock(&queue->mutex);
    return 1;
}

/**
 * @brief Worker thread function to process SIP messages.
 * @param arg Pointer to the worker thread's message queue.
 * @return NULL
 */
void* process_sip_messages(void* arg) {
    message_queue_t *queue = (message_queue_t *)arg;
    sip_message_t *message;

    while (1) {
        if (dequeue_message(queue, &message)) {
            // Process the SIP message here
            printf("Processing SIP message: %s\n", message->buffer);

            // TODO: Simulate processing and response
            // Here is where you'd implement message processing and response sending logic

            free(message);
        }
    }

    return NULL;
}
