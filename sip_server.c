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
                                        SIP_PROTOCOL_AND_VERSION " " RESPONSE_CODE_180 " " RESPONSE_TEXT_RINGING "\r\n" HEADER_NAME_VIA ": %.*s\r\n" HEADER_NAME_FROM ": %.*s\r\n" HEADER_NAME_TO ": %.*s;tag=%.*s\r\n" HEADER_NAME_CALL_ID ": %.*s\r\n" HEADER_NAME_CSEQ ": %.*s\r\n" HEADER_NAME_CONTENT_LENGTH ": 0\r\n"
                                                                 "\r\n",
                                        (int)request->via_length, request->via,
                                        (int)request->from_length, request->from,
                                        (int)request->to_length, request->to,
                                        (int)request->to_tag_length, request->to_tag,
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

int send_sip_error_response_over_transaction(int server_socket, sip_transaction_t *transaction, const char *status_code, const char *reason)
{
    printf("Sending SIP error response over transaction: %s %s\n", status_code, reason);
    sip_message_t *request = transaction->message;
    if (request == NULL)
    {
        printf("Transaction has no associated request message\n");
        return -1;
    }

    request->response_length = snprintf(request->response, sizeof(request->response),
                                        SIP_PROTOCOL_AND_VERSION " %s %s\r\n" HEADER_NAME_VIA ": %.*s\r\n" HEADER_NAME_FROM ": %.*s\r\n" HEADER_NAME_TO ": %.*s\r\n" HEADER_NAME_CALL_ID ": %.*s\r\n" HEADER_NAME_CSEQ ": %.*s\r\n" HEADER_NAME_CONTENT_LENGTH ": 0\r\n"
                                                                 "\r\n",
                                        status_code, reason,
                                        (int)request->via_length, request->via,
                                        (int)request->from_length, request->from,
                                        (int)request->to_length, request->to,
                                        (int)request->call_id_length, request->call_id,
                                        (int)request->cseq_length, request->cseq);
    return send_message(server_socket, request->response, request->response_length, &request->client_addr, request->client_addr_len);
    // TODO
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
                                        SIP_PROTOCOL_AND_VERSION " " RESPONSE_CODE_100 " " RESPONSE_TEXT_TRYING "\r\n" HEADER_NAME_VIA ": %.*s\r\n" HEADER_NAME_FROM ": %.*s\r\n" HEADER_NAME_TO ": %.*s\r\n" HEADER_NAME_CALL_ID ": %.*s\r\n" HEADER_NAME_CSEQ ": %.*s\r\n" HEADER_NAME_CONTENT_LENGTH ": 0\r\n"
                                                                 "\r\n",
                                        (int)request->via_length, request->via,
                                        (int)request->from_length, request->from,
                                        (int)request->to_length, request->to,
                                        (int)request->call_id_length, request->call_id,
                                        (int)request->cseq_length, request->cseq);
    return send_message(server_socket, request->response, request->response_length, &request->client_addr, request->client_addr_len);
    // TODO
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
                                        SIP_PROTOCOL_AND_VERSION " " RESPONSE_CODE_180 " " RESPONSE_TEXT_RINGING "\r\n" HEADER_NAME_VIA ": %.*s\r\n" HEADER_NAME_FROM ": %.*s\r\n" HEADER_NAME_TO ": %.*s\r\n" HEADER_NAME_CALL_ID ": %.*s\r\n" HEADER_NAME_CSEQ ": %.*s\r\n" HEADER_NAME_CONTENT_LENGTH ": 0\r\n"
                                                                 "\r\n",
                                        (int)request->via_length, request->via,
                                        (int)request->from_length, request->from,
                                        (int)request->to_length, request->to,
                                        (int)request->call_id_length, request->call_id,
                                        (int)request->cseq_length, request->cseq);
    return send_message(server_socket, request->response, request->response_length, &request->client_addr, request->client_addr_len);
    // TODO
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
                                        SIP_PROTOCOL_AND_VERSION " " RESPONSE_CODE_200 " " RESPONSE_TEXT_OK "\r\n" HEADER_NAME_VIA ": %.*s\r\n" HEADER_NAME_FROM ": %.*s\r\n" HEADER_NAME_TO ": %.*s;" PARAM_NAME_TAG "=%.*s\r\n" HEADER_NAME_CALL_ID ": %.*s\r\n" HEADER_NAME_CSEQ ": %.*s\r\n" HEADER_NAME_CONTENT_LENGTH ": 0\r\n"
                                                                 "\r\n",
                                        (int)request->via_length, request->via,
                                        (int)request->from_length, request->from,
                                        (int)request->to_length, request->to,
                                        (int)request->to_tag_length, request->to_tag,
                                        (int)request->call_id_length, request->call_id,
                                        (int)request->cseq_length, request->cseq);
    return send_message(server_socket, request->response, request->response_length, &request->client_addr, request->client_addr_len);
    // TODO
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
    // TODO
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
    sip_dialog_t *dialog = NULL;

    if (transaction->dialog == NULL)
    {
        dialog = find_dialog_by_id(worker->dialogs, request->from_tag, request->from_tag_length, NULL, 0);
        if (dialog != NULL)
        {
            transaction->dialog = dialog;
        }
    }

    if (transaction->dialog == NULL)
    { // new INVITE request

        if (send_100_trying_response_over_transaction(worker->server_socket, transaction) != 0)
        {
            printf("Failed to send 100 Trying response\n");
            send_sip_error_response_over_transaction(worker->server_socket, transaction, RESPONSE_CODE_500, RESPONSE_TEXT_INTERNAL_SERVER_ERROR);
            delete_transaction(&worker->transactions, request->branch, request->branch_length); // TODO must be done after transaction timeout
            return;
        }

        sip_dialog_t *dialog = create_new_dialog(&worker->dialogs, request->from_tag, request->from_tag_length);
        if (dialog == NULL)
        {
            printf("Failed to create new SIP dialog\n");
            send_sip_error_response_over_transaction(worker->server_socket, transaction, RESPONSE_CODE_500, RESPONSE_TEXT_INTERNAL_SERVER_ERROR);
            delete_transaction(&worker->transactions, request->branch, request->branch_length); // TODO must be done after transaction timeout
            return;
        }

        dialog->transaction = transaction;
        dialog->state = SIP_DIALOG_STATE_EARLY;
        transaction->dialog = dialog;

        // TODO check if call exists, it would be re-INVITE
        sip_call_t *call = create_new_call(&worker->calls, request->call_id, request->call_id_length);
        if (call == NULL)
        {
            printf("Failed to create new SIP call\n");
            send_sip_error_response_over_transaction(worker->server_socket, transaction, RESPONSE_CODE_500, RESPONSE_TEXT_INTERNAL_SERVER_ERROR);
            delete_dialog(worker->dialogs, request->from_tag, request->from_tag_length, NULL, 0); // TODO must be done after dialog timeout
            delete_transaction(&worker->transactions, request->branch, request->branch_length);   // TODO must be done after transaction timeout
            return;
        }

        call->state = SIP_CALL_STATE_INCOMING;
        call->dialog = dialog;

        if (send_180_ring_response_over_transaction(worker->server_socket, transaction) != 0)
        {
            printf("Failed to send 180 Ringing response\n");
            send_sip_error_response_over_transaction(worker->server_socket, transaction, RESPONSE_CODE_500, RESPONSE_TEXT_INTERNAL_SERVER_ERROR);
            delete_call(&worker->calls, request->call_id, request->call_id_length);               // TODO must be done after call timeout
            delete_dialog(worker->dialogs, request->from_tag, request->from_tag_length, NULL, 0); // TODO must be done after dialog timeout
            delete_transaction(&worker->transactions, request->branch, request->branch_length);   // TODO must be done after transaction timeout
            return;
        }

        call->state = SIP_CALL_STATE_RINGING;
        // TODO to simulate call setup delay, send 200 OK after a short delay in a timer logic
        if (send_sip_200_ok_response_over_transaction(worker->server_socket, transaction) != 0)
        {
            printf("Failed to send 200 OK response\n");
            send_sip_error_response_over_transaction(worker->server_socket, transaction, RESPONSE_CODE_500, RESPONSE_TEXT_INTERNAL_SERVER_ERROR);
            delete_call(&worker->calls, request->call_id, request->call_id_length);               // TODO must be done after call timeout
            delete_dialog(worker->dialogs, request->from_tag, request->from_tag_length, NULL, 0); // TODO must be done after dialog timeout
            delete_transaction(&worker->transactions, request->branch, request->branch_length);   // TODO must be done after transaction timeout
            return;
        }
        transaction->state = SIP_TRANSACTION_STATE_TERMINATED;
        dialog->state = SIP_DIALOG_STATE_CONFIRMED;
        call->state = SIP_CALL_STATE_ESTABLISHED;
    }
    else
    { // retransmission of INVITE
        switch (transaction->state)
        {
        case SIP_TRANSACTION_STATE_PROCEEDING:
        case SIP_TRANSACTION_STATE_COMPLETED:
        case SIP_TRANSACTION_STATE_TERMINATED:

            const char *cseq = NULL;
            size_t cseq_length = 0;
            cseq = get_message_cseq(request, &cseq_length);
            if (cseq_length != transaction->message->cseq_length ||
                strncmp(cseq, transaction->message->cseq, cseq_length) != 0 ||
                sockaddr_in_equal(&request->client_addr, &transaction->message->client_addr) != 0)
            {
                printf("Received INVITE is different than existing transaction, ignoring\n");
                // TODO for this case it is need to be create new transaction? but i dont have any idea how to differentiate them
            }
            else
            {
                if (send_last_response_over_transaction(worker->server_socket, transaction) != 0)
                {
                    printf("Failed to resend last response over transaction\n");
                    send_sip_error_response_over_transaction(worker->server_socket, transaction, RESPONSE_CODE_500, RESPONSE_TEXT_INTERNAL_SERVER_ERROR);
                }
            }
            break;

        default:
            break;
        }
        cleanup_sip_message(request);
    }
}

void process_ack_request(worker_thread_t *worker, sip_transaction_t *transaction)
{
    printf("Processing SIP ACK request\n");
}

void process_bye_request(worker_thread_t *worker, sip_transaction_t *transaction)
{
    printf("Processing SIP BYE request\n");
}

void process_sip_request(worker_thread_t *worker, sip_message_t *message)
{
    sip_transaction_t *transaction = find_transaction_by_branch(worker->transactions, message->branch, message->branch_length);
    if (transaction == NULL)
    {
        transaction = create_new_transaction(&worker->transactions, message->branch, message->branch_length);
        if (transaction == NULL)
        {
            printf("Failed to create new SIP transaction\n");
            send_sip_error_response(message, 500 /*TODO*/, RESPONSE_TEXT_INTERNAL_SERVER_ERROR);
            cleanup_sip_message(message);
            return;
        }

        transaction->state = SIP_TRANSACTION_STATE_PROCEEDING;
        transaction->message = message;
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
        send_sip_error_response_over_transaction(worker->server_socket, transaction, RESPONSE_CODE_501, RESPONSE_TEXT_NOT_IMPLEMENTED);
        delete_transaction(&worker->transactions, message->branch, message->branch_length); // TODO must be done after transaction timeout
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
    sip_transaction_t *transaction = find_transaction_by_branch(worker->transactions, message->branch, message->branch_length);
    if (transaction == NULL)
    {
        printf("No matching transaction found for SIP response with branch: %.*s\n", (int)message->branch_length, message->branch);
        cleanup_sip_message(message);
        return;
    }

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
