/**
 * @file sip_server.c
 * @brief Implementation of SIP server functionalities, including message processing and queue management.
 */

#include "sip_server.h"
#include "sip_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void send_sip_error_response(int server_socket, sip_message_t *request, int status_code, const char *reason)
{
    printf("Sending SIP error response: %d %s\n", status_code, reason);

    request->response_length = snprintf(request->response, sizeof(request->response),
                                        SIP_PROTOCOL_AND_VERSION " %d %s\r\n" HEADER_NAME_VIA ": %.*s\r\n" HEADER_NAME_FROM ": %.*s\r\n" HEADER_NAME_TO ": %.*s\r\n" HEADER_NAME_CALL_ID ": %.*s\r\n" HEADER_NAME_CSEQ ": %.*s\r\n" HEADER_NAME_CONTENT_LENGTH ": 0\r\n"
                                                                 "\r\n",
                                        status_code, reason,
                                        (int)request->via_length, request->via,
                                        (int)request->from_length, request->from,
                                        (int)request->to_length, request->to,
                                        (int)request->call_id_length, request->call_id,
                                        (int)request->cseq_length, request->cseq);
    send_message(server_socket, request->response, request->response_length, &request->client_addr, request->client_addr_len);
}

int send_sip_error_response_over_transaction(int server_socket, sip_transaction_t *transaction, int status_code, const char *reason)
{
    printf("Sending SIP error response over transaction: %d %s\n", status_code, reason);
    sip_message_t *request = transaction->message;
    if (request == NULL)
    {
        printf("Transaction has no associated request message\n");
        return -1;
    }

    request->response_length = snprintf(request->response, sizeof(request->response),
                                        SIP_PROTOCOL_AND_VERSION " %d %s\r\n" HEADER_NAME_VIA ": %.*s\r\n" HEADER_NAME_FROM ": %.*s\r\n" HEADER_NAME_TO ": %.*s\r\n" HEADER_NAME_CALL_ID ": %.*s\r\n" HEADER_NAME_CSEQ ": %.*s\r\n" HEADER_NAME_CONTENT_LENGTH ": 0\r\n"
                                                                 "\r\n",
                                        status_code, reason,
                                        (int)request->via_length, request->via,
                                        (int)request->from_length, request->from,
                                        (int)request->to_length, request->to,
                                        (int)request->call_id_length, request->call_id,
                                        (int)request->cseq_length, request->cseq);
    transaction->final_response_code = status_code;
    return send_message(server_socket, request->response, request->response_length, &request->client_addr, request->client_addr_len);
    // TODO retransmit
}

int send_100_trying_response_over_transaction(int server_socket, sip_transaction_t *transaction)
{
    printf("Sending 100 Trying response over transaction\n");

    sip_message_t *request = transaction->message;
    if (request == NULL)
    {
        printf("Transaction has no associated request message\n");
        return -1;
    }

    request->response_length = snprintf(request->response, sizeof(request->response),
                                        SIP_PROTOCOL_AND_VERSION " %d " RESPONSE_TEXT_100_TRYING "\r\n" HEADER_NAME_VIA ": %.*s\r\n" HEADER_NAME_FROM ": %.*s\r\n" HEADER_NAME_TO ": %.*s\r\n" HEADER_NAME_CALL_ID ": %.*s\r\n" HEADER_NAME_CSEQ ": %.*s\r\n" HEADER_NAME_CONTENT_LENGTH ": 0\r\n"
                                                                 "\r\n",
                                        RESPONSE_CODE_100,
                                        (int)request->via_length, request->via,
                                        (int)request->from_length, request->from,
                                        (int)request->to_length, request->to,
                                        (int)request->call_id_length, request->call_id,
                                        (int)request->cseq_length, request->cseq);
    transaction->final_response_code = RESPONSE_CODE_100;
    return send_message(server_socket, request->response, request->response_length, &request->client_addr, request->client_addr_len);
    // TODO retransmit
}

int send_180_ring_response_over_transaction(int server_socket, sip_transaction_t *transaction)
{
    printf("Sending 180 Ringing response over transaction\n");
    sip_message_t *request = transaction->message;
    if (request == NULL)
    {
        printf("Transaction has no associated request message\n");
        return -1;
    }

    request->response_length = snprintf(request->response, sizeof(request->response),
                                        SIP_PROTOCOL_AND_VERSION " %d " RESPONSE_TEXT_180_RINGING "\r\n" HEADER_NAME_VIA ": %.*s\r\n" HEADER_NAME_FROM ": %.*s\r\n" HEADER_NAME_TO ": %.*s;" PARAM_NAME_TAG "=%.*s\r\n" HEADER_NAME_CALL_ID ": %.*s\r\n" HEADER_NAME_CSEQ ": %.*s\r\n" HEADER_NAME_CONTENT_LENGTH ": 0\r\n"
                                                                 "\r\n",
                                        RESPONSE_CODE_180,
                                        (int)request->via_length, request->via,
                                        (int)request->from_length, request->from,
                                        (int)request->to_length, request->to,
                                        (int)transaction->dialog->to_tag_length, transaction->dialog->to_tag,
                                        (int)request->call_id_length, request->call_id,
                                        (int)request->cseq_length, request->cseq);
    transaction->final_response_code = RESPONSE_CODE_180;
    return send_message(server_socket, request->response, request->response_length, &request->client_addr, request->client_addr_len);
    // TODO retransmit
}

int send_sip_200_ok_response_over_transaction(int server_socket, sip_transaction_t *transaction)
{
    printf("Sending 200 OK response over transaction\n");
    sip_message_t *request = transaction->message;
    if (request == NULL)
    {
        printf("Transaction has no associated request message\n");
        return -1;
    }

    request->response_length = snprintf(request->response, sizeof(request->response),
                                        SIP_PROTOCOL_AND_VERSION " %d " RESPONSE_TEXT_200_OK "\r\n" HEADER_NAME_VIA ": %.*s\r\n" HEADER_NAME_FROM ": %.*s\r\n" HEADER_NAME_TO ": %.*s;" PARAM_NAME_TAG "=%.*s\r\n" HEADER_NAME_CALL_ID ": %.*s\r\n" HEADER_NAME_CSEQ ": %.*s\r\n" HEADER_NAME_CONTENT_LENGTH ": 0\r\n"
                                                                 "\r\n",
                                        RESPONSE_CODE_200,
                                        (int)request->via_length, request->via,
                                        (int)request->from_length, request->from,
                                        (int)request->to_length, request->to,
                                        (int)transaction->dialog->to_tag_length, transaction->dialog->to_tag,
                                        (int)request->call_id_length, request->call_id,
                                        (int)request->cseq_length, request->cseq);
    transaction->final_response_code = RESPONSE_CODE_200;
    return send_message(server_socket, request->response, request->response_length, &request->client_addr, request->client_addr_len);
    // TODO retransmit
}

// TODO, do we need to update client address info from via header?
int send_last_response_over_transaction(int server_socket, sip_transaction_t *transaction)
{
    printf("Resending last response over transaction\n");
    sip_message_t *request = transaction->message;
    if (request == NULL)
    {
        printf("Transaction has no associated request message\n");
        return -1;
    }
    return send_message(server_socket, request->response, request->response_length, &request->client_addr, request->client_addr_len);
    // TODO retransmit
}

int sockaddr_in_equal(const struct sockaddr_in *a, const struct sockaddr_in *b)
{
    if (a->sin_family != b->sin_family)
        return -1;
    if (a->sin_port != b->sin_port)
        return -1;
    if (a->sin_addr.s_addr != b->sin_addr.s_addr)
        return -1;
    return 0;
}

void process_invite_request(worker_thread_t *worker, sip_transaction_t *transaction)
{
    printf("Processing SIP INVITE request\n");
    sip_message_t *request = transaction->message;

    if (transaction->dialog == NULL)
    { // new INVITE request

        if (send_100_trying_response_over_transaction(worker->server_socket, transaction) != 0)
        {
            printf("Failed to send 100 Trying response\n");
            send_sip_error_response_over_transaction(worker->server_socket, transaction, RESPONSE_CODE_500, RESPONSE_TEXT_500_INTERNAL_SERVER_ERROR);
            // TODO resource cleanup
            return;
        }

        transaction->state = SIP_TRANSACTION_STATE_PROCEEDING;

        sip_dialog_t *dialog = create_new_dialog(&worker->dialogs, request->from_tag, request->from_tag_length);
        if (dialog == NULL)
        {
            printf("Failed to create new SIP dialog\n");
            send_sip_error_response_over_transaction(worker->server_socket, transaction, RESPONSE_CODE_500, RESPONSE_TEXT_500_INTERNAL_SERVER_ERROR);
            // TODO resource cleanup
            return;
        }

        set_transaction_dialog(transaction, dialog);
        dialog->state = SIP_DIALOG_STATE_EARLY;

        // TODO check if call exists, it would be re-INVITE
        sip_call_t *call = create_new_call(&worker->calls, request->call_id, request->call_id_length);
        if (call == NULL)
        {
            printf("Failed to create new SIP call\n");
            send_sip_error_response_over_transaction(worker->server_socket, transaction, RESPONSE_CODE_500, RESPONSE_TEXT_500_INTERNAL_SERVER_ERROR);
            // TODO resource cleanup
            return;
        }

        set_dialog_call(dialog, call);
        call->state = SIP_CALL_STATE_INCOMING;

        if (send_180_ring_response_over_transaction(worker->server_socket, transaction) != 0)
        {
            printf("Failed to send 180 Ringing response\n");
            send_sip_error_response_over_transaction(worker->server_socket, transaction, RESPONSE_CODE_500, RESPONSE_TEXT_500_INTERNAL_SERVER_ERROR);
            // TODO resource cleanup
            return;
        }

        call->state = SIP_CALL_STATE_RINGING;
        // TODO to simulate call setup delay, send 200 OK after a short delay in a timer logic
        if (send_sip_200_ok_response_over_transaction(worker->server_socket, transaction) != 0)
        {
            printf("Failed to send 200 OK response\n");
            send_sip_error_response_over_transaction(worker->server_socket, transaction, RESPONSE_CODE_500, RESPONSE_TEXT_500_INTERNAL_SERVER_ERROR);
            // TODO resource cleanup
            return;
        }
        transaction->state = SIP_TRANSACTION_STATE_TERMINATED;
        dialog->state = SIP_TRANSACTION_STATE_COMPLETED;
        call->state = SIP_CALL_STATE_ESTABLISHED;
    }
    else
    {
        // TODO re-INVITE
    }
}

void process_ack_request(worker_thread_t *worker, sip_transaction_t *transaction)
{
    printf("Processing SIP ACK request\n");

    if (transaction->state == SIP_TRANSACTION_STATE_COMPLETED && transaction->message->method_type == INVITE && transaction->ack_message != NULL && transaction->ack_message->method_type == ACK)
    {
        // ack for failed INVITE
        printf("ACK for failed INVITE\n");
        transaction->state = SIP_TRANSACTION_STATE_CONFIRMED;
        // TODO resource cleanup
    }
    else if (transaction->state == SIP_TRANSACTION_STATE_IDLE && transaction->message->method_type == ACK && transaction->dialog->state == SIP_DIALOG_STATE_CONFIRMED)
    {
        // ACK for successful INVITE
        printf("ACK for successful INVITE\n");
        transaction->state = SIP_TRANSACTION_STATE_TERMINATED;
        // TODO resource cleanup
    }
    else
    {
        // ACK for other requests
        printf("ACK for other requests\n");
        transaction->state = SIP_TRANSACTION_STATE_TERMINATED;
        // TODO resource cleanup
    }
}

void process_bye_request(worker_thread_t *worker, sip_transaction_t *transaction)
{
    printf("Processing SIP BYE request\n");

    if (transaction->dialog->state == SIP_DIALOG_STATE_CONFIRMED)
    {
        send_sip_200_ok_response_over_transaction(worker->server_socket, transaction);
        transaction->dialog->call->state = SIP_CALL_STATE_TERMINATED;
        transaction->dialog->state = SIP_DIALOG_STATE_TERMINATED;
        transaction->state = SIP_TRANSACTION_STATE_TERMINATED;
    }
    else
    {
        send_sip_error_response_over_transaction(worker->server_socket, transaction, RESPONSE_CODE_403, RESPONSE_TEXT_403_FORBIDDEN);
    }
    // TODO resource cleanup
}

void process_sip_request(worker_thread_t *worker, sip_message_t *message)
{
    sip_transaction_t *transaction = find_transaction_by_id(worker->transactions, message->branch, message->branch_length);
    if (transaction == NULL)
    {
        transaction = create_new_transaction(&worker->transactions, message->branch, message->branch_length);
        if (transaction == NULL)
        {
            printf("Failed to create new SIP transaction\n");
            send_sip_error_response(worker->server_socket, message, RESPONSE_CODE_500, RESPONSE_TEXT_500_INTERNAL_SERVER_ERROR);
            cleanup_sip_message(message);
            return;
        }

        transaction->message = message;
    }
    else
    {
        if (message->cseq_length != transaction->message->cseq_length ||
            strncmp(message->cseq, transaction->message->cseq, message->cseq_length) != 0 ||
            sockaddr_in_equal(&message->client_addr, &transaction->message->client_addr) != 0)
        {
            printf("Received message is different than existing transaction message.\n");
            if (transaction->message->method_type == INVITE && message->method_type == ACK)
            {
                printf("ACK for INVITE\n");
                transaction->ack_message = message;
            }
            else
            {
                printf("New request for existing transaction branch id\n");
                cleanup_sip_message(message);
                return;
            }
        }
        else
        {
            if (send_last_response_over_transaction(worker->server_socket, transaction) != 0)
            {
                printf("Failed to resend last response over transaction\n");
                send_sip_error_response_over_transaction(worker->server_socket, transaction, RESPONSE_CODE_500, RESPONSE_TEXT_500_INTERNAL_SERVER_ERROR);
            }
            cleanup_sip_message(message);
            return;
        }
    }

    if (transaction->dialog == NULL)
    {
        sip_dialog_t *dialog = find_dialog_by_id(worker->dialogs, message->from_tag, message->from_tag_length, message->to_tag, message->to_tag_length);
        if (dialog != NULL)
        {
            set_transaction_dialog(transaction, dialog);
        }
    }

    sip_method_t method = get_message_method(message);
    switch (method)
    {
    case INVITE:
        return process_invite_request(worker, transaction);
    case ACK:
        return process_ack_request(worker, transaction);
    case BYE:
        return process_bye_request(worker, transaction);
    default:
        // TODO other methods
        printf("Unsupported SIP method: %s\n", message->method);
        send_sip_error_response_over_transaction(worker->server_socket, transaction, RESPONSE_CODE_501, RESPONSE_TEXT_501_NOT_IMPLEMENTED);
        // TODO resource cleanup
    }
}

void process_provisional_response(worker_thread_t *worker, sip_message_t *message)
{
    printf("Processing SIP provisional response\n");
    cleanup_sip_message(message);
}

void process_successful_response(worker_thread_t *worker, sip_message_t *message)
{
    printf("Processing SIP successful response\n");
    cleanup_sip_message(message);
}

void process_redirection_response(worker_thread_t *worker, sip_message_t *message)
{
    printf("Processing SIP redirection response\n");
    cleanup_sip_message(message);
}

void process_client_error_response(worker_thread_t *worker, sip_message_t *message)
{
    printf("Processing SIP client error response\n");
    cleanup_sip_message(message);
}

void process_server_error_response(worker_thread_t *worker, sip_message_t *message)
{
    printf("Processing SIP server error response\n");
    cleanup_sip_message(message);
}

void process_global_failure_response(worker_thread_t *worker, sip_message_t *message)
{
    printf("Processing SIP global failure response\n");
    cleanup_sip_message(message);
}

void process_sip_response(worker_thread_t *worker, sip_message_t *message)
{
    sip_transaction_t *transaction = find_transaction_by_id(worker->transactions, message->branch, message->branch_length);
    if (transaction == NULL)
    {
        printf("No matching transaction found for SIP response with branch: %.*s\n", (int)message->branch_length, message->branch);
        cleanup_sip_message(message);
        return;
    }

    if (message->status_code >= RESPONSE_CODE_PROVISIONAL_START && message->status_code <= RESPONSE_CODE_PROVISIONAL_END)
    {
        return process_provisional_response(worker, message);
    }
    else if (message->status_code >= RESPONSE_CODE_SUCCESS_START && message->status_code <= RESPONSE_CODE_SUCCESS_END)
    {
        return process_successful_response(worker, message);
    }
    else if (message->status_code >= RESPONSE_CODE_REDIRECTION_START && message->status_code <= RESPONSE_CODE_REDIRECTION_END)
    {
        return process_redirection_response(worker, message);
    }
    else if (message->status_code >= RESPONSE_CODE_CLIENT_ERROR_START && message->status_code <= RESPONSE_CODE_CLIENT_ERROR_END)
    {
        return process_client_error_response(worker, message);
    }
    else if (message->status_code >= RESPONSE_CODE_SERVER_ERROR_START && message->status_code <= RESPONSE_CODE_SERVER_ERROR_END)
    {
        return process_server_error_response(worker, message);
    }
    else if (message->status_code >= RESPONSE_CODE_GLOBAL_FAILURE_START && message->status_code <= RESPONSE_CODE_GLOBAL_FAILURE_END)
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
                cleanup_sip_message(message);
                continue;
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
