/**
 * @file sip_server.c
 * @brief Implementation of SIP server functionalities, including message processing and queue management.
 */

#include "sip_server.h"
#include "sip_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void send_sip_error_response(sip_message_t *request, int status_code, const char *reason)
{
    printf("Sending SIP error response: %d %s\n", status_code, reason);
}

int send_message(int server_socket, char *message, size_t message_length, struct sockaddr_in *client_addr, socklen_t client_addr_len)
{
    // TODO maybe use dedicated sender thread
    printf("Sending SIP message\n%s\n", message);
    int rc = sendto(server_socket, message, message_length, 0, (struct sockaddr *)client_addr, client_addr_len);
    if (rc < 0)
    {
        perror("Failed to send SIP message");
        return -1;
    }
    return 0;
}

int send_100_trying_response(int server_socket, sip_message_t *request)
{
    printf("Sending 100 Trying response\n");

    request->response_length = snprintf(request->response, sizeof(request->response),
                                        SIP_PROTOCOL_AND_VERSION " " RESPONSE_CODE_100 " " RESPONSE_TEXT_TRYING "\r\n" HEADER_NAME_VIA ": %.*s\r\n" HEADER_NAME_FROM ": %.*s\r\n" HEADER_NAME_TO ": %.*s\r\n" HEADER_NAME_CALL_ID ": %.*s\r\n" HEADER_NAME_CSEQ ": %.*s\r\n" HEADER_NAME_CONTENT_LENGTH ": 0\r\n"
                                                                 "\r\n",
                                        (int)request->via_length, request->via,
                                        (int)request->from_length, request->from,
                                        (int)request->to_length, request->to,
                                        (int)request->call_id_length, request->call_id,
                                        (int)request->cseq_length, request->cseq);
    return send_message(server_socket, request->response, request->response_length, &request->client_addr, request->client_addr_len);
}

int send_180_ring_response(int server_socket, sip_message_t *request)
{
    printf("Sending 180 Ringing response\n");
    request->response_length = snprintf(request->response, sizeof(request->response),
                                        SIP_PROTOCOL_AND_VERSION " " RESPONSE_CODE_180 " " RESPONSE_TEXT_RINGING "\r\n" HEADER_NAME_VIA ": %.*s\r\n" HEADER_NAME_FROM ": %.*s\r\n" HEADER_NAME_TO ": %.*s\r\n" HEADER_NAME_CALL_ID ": %.*s\r\n" HEADER_NAME_CSEQ ": %.*s\r\n" HEADER_NAME_CONTENT_LENGTH ": 0\r\n"
                                                                 "\r\n",
                                        (int)request->via_length, request->via,
                                        (int)request->from_length, request->from,
                                        (int)request->to_length, request->to,
                                        (int)request->call_id_length, request->call_id,
                                        (int)request->cseq_length, request->cseq);
    return send_message(server_socket, request->response, request->response_length, &request->client_addr, request->client_addr_len);
}

int send_sip_200_ok_response(int server_socket, sip_message_t *request)
{
    printf("Sending 200 OK response\n");
    request->response_length = snprintf(request->response, sizeof(request->response),
                                        SIP_PROTOCOL_AND_VERSION " " RESPONSE_CODE_200 " " RESPONSE_TEXT_OK "\r\n" HEADER_NAME_VIA ": %.*s\r\n" HEADER_NAME_FROM ": %.*s\r\n" HEADER_NAME_TO ": %.*s\r\n" HEADER_NAME_CALL_ID ": %.*s\r\n" HEADER_NAME_CSEQ ": %.*s\r\n" HEADER_NAME_CONTENT_LENGTH ": 0\r\n"
                                                                 "\r\n",
                                        (int)request->via_length, request->via,
                                        (int)request->from_length, request->from,
                                        (int)request->to_length, request->to,
                                        (int)request->call_id_length, request->call_id,
                                        (int)request->cseq_length, request->cseq);
    return send_message(server_socket, request->response, request->response_length, &request->client_addr, request->client_addr_len);
}

void process_invite_request(worker_thread_t *worker, sip_message_t *request)
{
    printf("Processing SIP INVITE request\n");

    sip_call_t *call = find_call_by_id(worker->calls, request->call_id, request->call_id_length);
    if (call == NULL)
    {
        sip_call_t *new_call = create_new_call(&worker->calls, request->call_id, request->call_id_length);
        if (new_call == NULL)
        {
            printf("Failed to create new SIP call\n");
            send_sip_error_response(request, 500, "Internal Server Error");
            cleanup_sip_message(request);
            return;
        }

        new_call->state = SIP_CALL_STATE_INCOMING;

        if (send_100_trying_response(worker->server_socket, request) != 0)
        {
            printf("Failed to send 100 Trying response\n");
            send_sip_error_response(request, 500, "Internal Server Error");
            delete_call(&worker->calls, request->call_id, request->call_id_length);
            cleanup_sip_message(request);
            return;
        }

        const char *from_tag = NULL;
        size_t from_tag_length = 0;
        from_tag = get_from_tag(request, &from_tag_length);
        if (from_tag == NULL)
        {
            printf("From tag is missing in INVITE request\n");
            send_sip_error_response(request, 400, "Bad Request");
            delete_call(&worker->calls, request->call_id, request->call_id_length);
            cleanup_sip_message(request);
            return;
        }

        sip_dialog_t *dialog = create_new_dialog(&new_call->dialog, from_tag, from_tag_length);
        if (dialog == NULL)
        {
            printf("Failed to create new SIP dialog\n");
            send_sip_error_response(request, 500, "Internal Server Error");
            delete_call(&worker->calls, request->call_id, request->call_id_length);
            cleanup_sip_message(request);
            return;
        }

        dialog->state = SIP_DIALOG_STATE_EARLY;

        const char *branch = NULL;
        size_t branch_length = 0;
        branch = get_branch_param(request, &branch_length);
        if (branch == NULL)
        {
            printf("Via branch is missing in INVITE request\n");
            send_sip_error_response(request, 400, "Bad Request");
            delete_call(&worker->calls, request->call_id, request->call_id_length);
            cleanup_sip_message(request);
            return;
        }

        sip_transaction_t *transaction = create_new_transaction(&dialog->transaction, branch, branch_length);
        if (transaction == NULL)
        {
            printf("Failed to create new SIP transaction\n");
            send_sip_error_response(request, 500, "Internal Server Error");
            delete_call(&worker->calls, request->call_id, request->call_id_length);
            cleanup_sip_message(request);
            return;
        }
        // TODO after transaction creation, maybe need to set timer for non-200 responses retransmissions and call delete_call after timeout

        transaction->state = SIP_TRANSACTION_STATE_PROCEEDING;
        transaction->last_message = request;

        // build and send 180 Ringing response here
        // TODO to simulate call setup delay, send 180 Ringing after a short delay in a timer logic
        if (send_180_ring_response(worker->server_socket, request) != 0)
        {
            printf("Failed to send 180 Ringing response\n");
            delete_call(&worker->calls, request->call_id, request->call_id_length);
            send_sip_error_response(request, 500, "Internal Server Error");
            return;
        }

        new_call->state = SIP_CALL_STATE_RINGING;

        // build and send 200 OK response here
        // TODO to simulate call setup delay, send 200 OK after a short delay in a timer logic
        if (send_sip_200_ok_response(worker->server_socket, request) != 0)
        {
            printf("Failed to send 200 OK response\n");
            delete_call(&worker->calls, request->call_id, request->call_id_length);
            send_sip_error_response(request, 500, "Internal Server Error");
            return;
        }

        new_call->state = SIP_CALL_STATE_ESTABLISHED;
        dialog->state = SIP_DIALOG_STATE_CONFIRMED;
        transaction->state = SIP_TRANSACTION_STATE_TERMINATED;
    }
    else
    {
        // existing call
        cleanup_sip_message(request);
    }
}

void process_prack_request(worker_thread_t *worker, sip_message_t *message)
{
    printf("Processing SIP PRACK request\n");
}

void process_ack_request(worker_thread_t *worker, sip_message_t *message)
{
    printf("Processing SIP ACK request\n");
}

void process_bye_request(worker_thread_t *worker, sip_message_t *message)
{
    printf("Processing SIP BYE request\n");
}

void process_cancel_request(worker_thread_t *worker, sip_message_t *message)
{
    printf("Processing SIP CANCEL request\n");
}

void process_sip_request(worker_thread_t *worker, sip_message_t *message)
{
    sip_method_t method = get_message_method(message);
    switch (method)
    {
    case INVITE:
        return process_invite_request(worker, message);
    case PRACK:
        return process_prack_request(worker, message);
    case ACK:
        return process_ack_request(worker, message);
    case BYE:
        return process_bye_request(worker, message);
    case CANCEL:
        return process_cancel_request(worker, message);
    default:
        printf("Unsupported SIP method: %s\n", message->method);
        send_sip_error_response(message, 501, "Not Implemented");
        cleanup_sip_message(message);
    }
}

void process_provisional_response(worker_thread_t *worker, sip_message_t *message)
{
    printf("Processing SIP provisional response\n");
}

void process_successful_response(worker_thread_t *worker, sip_message_t *message)
{
    printf("Processing SIP successful response\n");
}

void process_redirection_response(worker_thread_t *worker, sip_message_t *message)
{
    printf("Processing SIP redirection response\n");
}

void process_client_error_response(worker_thread_t *worker, sip_message_t *message)
{
    printf("Processing SIP client error response\n");
}

void process_server_error_response(worker_thread_t *worker, sip_message_t *message)
{
    printf("Processing SIP server error response\n");
}

void process_global_failure_response(worker_thread_t *worker, sip_message_t *message)
{
    printf("Processing SIP global failure response\n");
}

void process_sip_response(worker_thread_t *worker, sip_message_t *message)
{
    if (message->status_code >= RESPONSE_PROVISIONAL_START && message->status_code <= RESPONSE_PROVISIONAL_END)
    {
        return process_provisional_response(worker, message);
    }
    else if (message->status_code >= RESPONSE_SUCCESS_START && message->status_code <= RESPONSE_SUCCESS_END)
    {
        return process_successful_response(worker, message);
    }
    else if (message->status_code >= RESPONSE_REDIRECTION_START && message->status_code <= RESPONSE_REDIRECTION_END)
    {
        return process_redirection_response(worker, message);
    }
    else if (message->status_code >= RESPONSE_CLIENT_ERROR_START && message->status_code <= RESPONSE_CLIENT_ERROR_END)
    {
        return process_client_error_response(worker, message);
    }
    else if (message->status_code >= RESPONSE_SERVER_ERROR_START && message->status_code <= RESPONSE_SERVER_ERROR_END)
    {
        return process_server_error_response(worker, message);
    }
    else if (message->status_code >= RESPONSE_GLOBAL_FAILURE_START && message->status_code <= RESPONSE_GLOBAL_FAILURE_END)
    {
        return process_global_failure_response(worker, message);
    }
    else
    {
        printf("Unknown SIP response status code: %d\n", message->status_code);
        cleanup_sip_message(message);
    }
}

/**
 * @brief Worker thread function to process SIP messages.
 * @param arg Pointer to the worker thread's message queue.
 * @return NULL
 */
void *process_sip_messages(void *arg)
{
    worker_thread_t *worker = (worker_thread_t *)arg;
    message_queue_t *queue = &worker->queue;
    sip_message_t *message;

    while (1)
    {
        if (dequeue_message(queue, &message))
        {
            // Process the SIP message here
            printf("Processing SIP message: \n\n%s\n", message->buffer);

            sip_msg_error_t err = parse_message(message);
            if (err != ERROR_NONE)
            {
                printf("Failed to parse SIP message: %d\n", err);
                if (message->is_request)
                {
                    send_sip_error_response(message, 400, "Bad Request");
                }
                cleanup_sip_message(message);
            }

            if (message->is_request)
            {
                process_sip_request(worker, message);
            }
            else
            {
                process_sip_response(worker, message);
            }
        }
    }

    return NULL;
}
