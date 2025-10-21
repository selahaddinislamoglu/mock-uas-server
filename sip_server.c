/**
 * @file sip_server.c
 * @brief Implementation of SIP server functionalities, including message processing and queue management.
 */

#include "sip_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void send_sip_error_response(sip_message_t *request, int status_code, const char *reason)
{
    // Placeholder implementation for sending SIP error responses
    printf("Sending SIP error response: %d %s\n", status_code, reason);
}

void process_invite_request(sip_message_t *message)
{
    // Placeholder implementation for processing SIP INVITE requests
    printf("Processing SIP INVITE request\n");
}

void process_prack_request(sip_message_t *message)
{
    // Placeholder implementation for processing SIP PRACK requests
    printf("Processing SIP PRACK request\n");
}

void process_ack_request(sip_message_t *message)
{
    // Placeholder implementation for processing SIP ACK requests
    printf("Processing SIP ACK request\n");
}

void process_bye_request(sip_message_t *message)
{
    // Placeholder implementation for processing SIP BYE requests
    printf("Processing SIP BYE request\n");
}

void process_cancel_request(sip_message_t *message)
{
    // Placeholder implementation for processing SIP CANCEL requests
    printf("Processing SIP CANCEL request\n");
}

void process_sip_request(sip_message_t *message)
{
    sip_method_t method = get_message_method(message);
    switch (method)
    {
    case INVITE:
        process_invite_request(message);
        break;
    case PRACK:
        process_prack_request(message);
        break;
    case ACK:
        process_ack_request(message);
        break;
    case BYE:
        process_bye_request(message);
        break;
    case CANCEL:
        process_cancel_request(message);
        break;
    default:
        printf("Unsupported SIP method: %s\n", message->method);
        send_sip_error_response(message, 501, "Not Implemented");
        break;
    }
}

void process_provisional_response(sip_message_t *message)
{
    // Placeholder implementation for processing SIP provisional responses
    printf("Processing SIP provisional response\n");
}

void process_successful_response(sip_message_t *message)
{
    // Placeholder implementation for processing SIP successful responses
    printf("Processing SIP successful response\n");
}

void process_redirection_response(sip_message_t *message)
{
    // Placeholder implementation for processing SIP redirection responses
    printf("Processing SIP redirection response\n");
}

void process_client_error_response(sip_message_t *message)
{
    // Placeholder implementation for processing SIP client error responses
    printf("Processing SIP client error response\n");
}

void process_server_error_response(sip_message_t *message)
{
    // Placeholder implementation for processing SIP server error responses
    printf("Processing SIP server error response\n");
}

void process_global_failure_response(sip_message_t *message)
{
    // Placeholder implementation for processing SIP global failure responses
    printf("Processing SIP global failure response\n");
}

void process_sip_response(sip_message_t *message)
{
    if (message->status_code >= RESPONSE_PROVISIONAL_START && message->status_code <= RESPONSE_PROVISIONAL_END)
    {
        process_provisional_response(message);
    }
    else if (message->status_code >= RESPONSE_SUCCESS_START && message->status_code <= RESPONSE_SUCCESS_END)
    {
        process_successful_response(message);
    }
    else if (message->status_code >= RESPONSE_REDIRECTION_START && message->status_code <= RESPONSE_REDIRECTION_END)
    {
        process_redirection_response(message);
    }
    else if (message->status_code >= RESPONSE_CLIENT_ERROR_START && message->status_code <= RESPONSE_CLIENT_ERROR_END)
    {
        process_client_error_response(message);
    }
    else if (message->status_code >= RESPONSE_SERVER_ERROR_START && message->status_code <= RESPONSE_SERVER_ERROR_END)
    {
        process_server_error_response(message);
    }
    else if (message->status_code >= RESPONSE_GLOBAL_FAILURE_START && message->status_code <= RESPONSE_GLOBAL_FAILURE_END)
    {
        process_global_failure_response(message);
    }
    else
    {
        printf("Unknown SIP response status code: %d\n", message->status_code);
    }
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
                if (message->is_request)
                {
                    send_sip_error_response(message, 400, "Bad Request");
                }
                goto cleanup;
            }

            if (message->is_request)
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
