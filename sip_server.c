/**
 * @file sip_server.c
 * @brief Implementation of SIP server functionalities, including message processing and queue management.
 */

#include "sip_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void send_sip_error_response(const sip_message_t *request, int status_code, const char *reason)
{
    // Placeholder implementation for sending SIP error responses
    printf("Sending SIP error response: %d %s\n", status_code, reason);
}

void process_sip_request(const sip_message_t *message)
{
    // Placeholder implementation for processing SIP requests
    printf("Processing SIP request: %.*s\n", (int)message->method_length, message->method);
}

void process_sip_response(const sip_message_t *message)
{
    // Placeholder implementation for processing SIP responses
    printf("Processing SIP response: %d %.*s\n", message->status_code, (int)message->reason_length, message->reason);
}

/**
 * @brief Worker thread function to process SIP messages.
 * @param arg Pointer to the worker thread's message queue.
 * @return NULL
 */
void *process_sip_messages(void *arg)
{
    message_queue_t *queue = (message_queue_t *)arg;
    sip_message_t *message;

    while (1)
    {
        if (dequeue_message(queue, &message))
        {
            // Process the SIP message here
            printf("Processing SIP message: %s\n", message->buffer);

            sip_msg_error_t err = parse_message(message);
            if (err != ERROR_NONE)
            {
                printf("Failed to parse SIP message first line\n");
                if (is_request(message))
                {
                    send_sip_error_response(message, 400, "Bad Request");
                }
                goto cleanup;
            }

            if (is_request(message))
            {
                process_sip_request(message);
            }
            else
            {
                process_sip_response(message);
            }

        cleanup:
            free(message);
        }
    }

    return NULL;
}
