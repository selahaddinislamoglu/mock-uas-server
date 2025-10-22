/**
 * @file sip_server.h
 * @brief Header for SIP server functionalities, including message processing and queue management.
 */

#ifndef SIP_SERVER_H
#define SIP_SERVER_H

#include <pthread.h>
#include "message_queue.h"
#include "sip_utils.h"

/**
 * @struct worker_thread_t
 * @brief Structure for worker thread and its associated message queue.
 */
typedef struct
{
    message_queue_t queue;
    pthread_t thread;
    sip_call_t *calls;               // TODO hash map for calls
    sip_dialog_t *dialogs;           // TODO hash map for dialogs
    sip_transaction_t *transactions; // TODO hash map for transactions
    int server_socket;               // TODO maybe need to implement dedicated sender thread
} worker_thread_t;

void *process_sip_messages(void *arg);

#endif // SIP_SERVER_H
