/**
 * @file sip_server.c
 * @brief Implementation of SIP server functionalities, including message processing and queue management.
 */

#include "sip_server.h"
#include "sip_utils.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/**
 * @brief Sends a SIP message to a specified destination and port.
 *
 * @param server_socket The socket to send the message on.
 * @param message The SIP message to be sent.
 * @param message_length The length of the SIP message.
 * @param client_addr The address of the client to send the message to.
 * @param client_addr_len The length of the client address structure.
 */
int send_message(int server_socket, char *message, size_t message_length, struct sockaddr_in *client_addr, socklen_t client_addr_len)
{
    if (server_socket < 0 || message == NULL || message_length == 0 || client_addr == NULL || client_addr_len == 0)
    {
        error("Invalid parameters");
        return -1;
    }
    // TODO maybe use dedicated sender thread
    log("Outgoing SIP message:\n<<<<<<<<<<<<<<<<<<<<<<<<<\n%.*s<<<<<<<<<<<<<<<<<<<<<<<<<\n", (int)message_length, message);

    int rc = sendto(server_socket, message, message_length, 0, (struct sockaddr *)client_addr, client_addr_len);
    if (rc < 0)
    {
        error("Failed to send SIP message: %s", strerror(errno));
        return -1;
    }
    return 0;
}

/**
 * @brief Sends a SIP error response.
 *
 * @param server_socket The socket to send the response on.
 * @param request The request message that triggered the error response.
 * @param status_code The status code of the error response.
 * @param reason The reason for the error response.
 */
void send_sip_error_response(int server_socket, sip_message_t *request, int status_code, const char *reason)
{
    if (server_socket < 0 || request == NULL || reason == NULL)
    {
        error("Invalid parameters");
        return;
    }
    log("Sending SIP error response: %d %s", status_code, reason);

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

/**
 * @brief Sends a SIP error response over a transaction.
 *
 * @param server_socket The socket to send the response on.
 * @param transaction The transaction to send the response for.
 * @param status_code The status code of the error response.
 * @param reason The reason for the error response.
 */
int send_sip_error_response_over_transaction(int server_socket, sip_transaction_t *transaction, int status_code, const char *reason)
{
    if (server_socket < 0 || transaction == NULL || reason == NULL)
    {
        error("Invalid parameters");
        return -1;
    }
    // TODO set dialog state on error
    log("Sending SIP error response over transaction: %d %s", status_code, reason);
    sip_message_t *request = transaction->message;
    if (request == NULL)
    {
        error("Transaction has no associated request message");
        return -1;
    }

    char to_tag[sizeof(PARAM_NAME_TAG) + SIP_TAG_MAX_LENGTH + 8] = {0};
    if (transaction->dialog != NULL && transaction->dialog->to_tag != NULL && strstr(request->to, transaction->dialog->to_tag) == NULL)
    {
        snprintf(to_tag, sizeof(to_tag), ";" PARAM_NAME_TAG "=%.*s", (int)transaction->dialog->to_tag_length, transaction->dialog->to_tag);
    }

    request->response_length = snprintf(request->response, sizeof(request->response),
                                        SIP_PROTOCOL_AND_VERSION " %d %s\r\n" HEADER_NAME_VIA ": %.*s\r\n" HEADER_NAME_FROM ": %.*s\r\n" HEADER_NAME_TO ": %.*s%s\r\n" HEADER_NAME_CALL_ID ": %.*s\r\n" HEADER_NAME_CSEQ ": %.*s\r\n" HEADER_NAME_CONTENT_LENGTH ": 0\r\n"
                                                                 "\r\n",
                                        status_code, reason,
                                        (int)request->via_length, request->via,
                                        (int)request->from_length, request->from,
                                        (int)request->to_length, request->to,
                                        to_tag[0] != '\0' ? to_tag : "",
                                        (int)request->call_id_length, request->call_id,
                                        (int)request->cseq_length, request->cseq);
    transaction->final_response_code = status_code;
    return send_message(server_socket, request->response, request->response_length, &request->client_addr, request->client_addr_len);
    // TODO retransmit
}

/**
 * @brief Sends a 100 Trying response over a transaction.
 *
 * @param server_socket The socket to send the response on.
 * @param transaction The transaction to send the response for.
 * @return 0 on success, -1 on failure.
 */
int send_100_trying_response_over_transaction(int server_socket, sip_transaction_t *transaction)
{
    if (server_socket < 0 || transaction == NULL || transaction->message == NULL)
    {
        error("Invalid parameters");
        return -1;
    }
    log("Sending 100 Trying response over transaction");

    sip_message_t *request = transaction->message;
    if (request == NULL)
    {
        error("Transaction has no associated request message");
        return -1;
    }

    char to_tag[sizeof(PARAM_NAME_TAG) + SIP_TAG_MAX_LENGTH + 8] = {0};
    if (transaction->dialog != NULL && transaction->dialog->to_tag != NULL && strstr(request->to, transaction->dialog->to_tag) == NULL)
    {
        snprintf(to_tag, sizeof(to_tag), ";" PARAM_NAME_TAG "=%.*s", (int)transaction->dialog->to_tag_length, transaction->dialog->to_tag);
    }

    request->response_length = snprintf(request->response, sizeof(request->response),
                                        SIP_PROTOCOL_AND_VERSION " %d " RESPONSE_TEXT_100_TRYING "\r\n" HEADER_NAME_VIA ": %.*s\r\n" HEADER_NAME_FROM ": %.*s\r\n" HEADER_NAME_TO ": %.*s%s\r\n" HEADER_NAME_CALL_ID ": %.*s\r\n" HEADER_NAME_CSEQ ": %.*s\r\n" HEADER_NAME_CONTENT_LENGTH ": 0\r\n"
                                                                 "\r\n",
                                        RESPONSE_CODE_100,
                                        (int)request->via_length, request->via,
                                        (int)request->from_length, request->from,
                                        (int)request->to_length, request->to,
                                        to_tag[0] != '\0' ? to_tag : "",
                                        (int)request->call_id_length, request->call_id,
                                        (int)request->cseq_length, request->cseq);
    transaction->final_response_code = RESPONSE_CODE_100;
    return send_message(server_socket, request->response, request->response_length, &request->client_addr, request->client_addr_len);
    // TODO retransmit
}

/**
 * @brief Sends a 180 Ringing response over a transaction.
 *
 * @param server_socket The socket to send the response on.
 * @param transaction The transaction to send the response for.
 * @return 0 on success, -1 on failure.
 */
int send_180_ring_response_over_transaction(int server_socket, sip_transaction_t *transaction)
{
    if (server_socket < 0 || transaction == NULL || transaction->message == NULL)
    {
        error("Invalid parameters");
        return -1;
    }
    log("Sending 180 Ringing response over transaction");
    sip_message_t *request = transaction->message;
    if (request == NULL)
    {
        error("Transaction has no associated request message");
        return -1;
    }

    char to_tag[sizeof(PARAM_NAME_TAG) + SIP_TAG_MAX_LENGTH + 8] = {0};
    if (transaction->dialog != NULL && transaction->dialog->to_tag != NULL && strstr(request->to, transaction->dialog->to_tag) == NULL)
    {
        snprintf(to_tag, sizeof(to_tag), ";" PARAM_NAME_TAG "=%.*s", (int)transaction->dialog->to_tag_length, transaction->dialog->to_tag);
    }

    request->response_length = snprintf(request->response, sizeof(request->response),
                                        SIP_PROTOCOL_AND_VERSION " %d " RESPONSE_TEXT_180_RINGING "\r\n" HEADER_NAME_VIA ": %.*s\r\n" HEADER_NAME_FROM ": %.*s\r\n" HEADER_NAME_TO ": %.*s%s\r\n" HEADER_NAME_CALL_ID ": %.*s\r\n" HEADER_NAME_CSEQ ": %.*s\r\n" HEADER_NAME_CONTENT_LENGTH ": 0\r\n"
                                                                 "\r\n",
                                        RESPONSE_CODE_180,
                                        (int)request->via_length, request->via,
                                        (int)request->from_length, request->from,
                                        (int)request->to_length, request->to,
                                        to_tag[0] != '\0' ? to_tag : "",
                                        (int)request->call_id_length, request->call_id,
                                        (int)request->cseq_length, request->cseq);
    transaction->final_response_code = RESPONSE_CODE_180;
    return send_message(server_socket, request->response, request->response_length, &request->client_addr, request->client_addr_len);
    // TODO retransmit
}

/**
 * @brief Sends a 200 OK response over a transaction.
 *
 * @param server_socket The socket to send the response on.
 * @param transaction The transaction to send the response for.
 * @return 0 on success, -1 on failure.
 */
int send_sip_200_ok_response_over_transaction(int server_socket, sip_transaction_t *transaction)
{
    if (server_socket < 0 || transaction == NULL || transaction->message == NULL)
    {
        error("Invalid parameters");
        return -1;
    }
    log("Sending 200 OK response over transaction");
    sip_message_t *request = transaction->message;
    if (request == NULL)
    {
        error("Transaction has no associated request message");
        return -1;
    }

    char to_tag[sizeof(PARAM_NAME_TAG) + SIP_TAG_MAX_LENGTH + 8] = {0};
    if (transaction->dialog != NULL && transaction->dialog->to_tag != NULL && strstr(request->to, transaction->dialog->to_tag) == NULL)
    {
        snprintf(to_tag, sizeof(to_tag), ";" PARAM_NAME_TAG "=%.*s", (int)transaction->dialog->to_tag_length, transaction->dialog->to_tag);
    }

    request->response_length = snprintf(request->response, sizeof(request->response),
                                        SIP_PROTOCOL_AND_VERSION " %d " RESPONSE_TEXT_200_OK "\r\n" HEADER_NAME_VIA ": %.*s\r\n" HEADER_NAME_FROM ": %.*s\r\n" HEADER_NAME_TO ": %.*s%s\r\n" HEADER_NAME_CALL_ID ": %.*s\r\n" HEADER_NAME_CSEQ ": %.*s\r\n" HEADER_NAME_CONTENT_LENGTH ": 0\r\n"
                                                                 "\r\n",
                                        RESPONSE_CODE_200,
                                        (int)request->via_length, request->via,
                                        (int)request->from_length, request->from,
                                        (int)request->to_length, request->to,
                                        to_tag[0] != '\0' ? to_tag : "",
                                        (int)request->call_id_length, request->call_id,
                                        (int)request->cseq_length, request->cseq);
    transaction->final_response_code = RESPONSE_CODE_200;
    return send_message(server_socket, request->response, request->response_length, &request->client_addr, request->client_addr_len);
    // TODO retransmit
}

/**
 * @brief Sends the last response over a transaction.
 *
 * @param server_socket The socket to send the response on.
 * @param transaction The transaction to send the response for.
 * @return 0 on success, -1 on failure.
 */
int send_last_response_over_transaction(int server_socket, sip_transaction_t *transaction)
{
    if (server_socket < 0 || transaction == NULL || transaction->message == NULL)
    {
        error("Invalid parameters");
        return -1;
    }
    // TODO, do we need to update client address info from via header?
    log("Resending last response over transaction");
    sip_message_t *request = transaction->message;
    if (request == NULL)
    {
        error("Transaction has no associated request message");
        return -1;
    }
    return send_message(server_socket, request->response, request->response_length, &request->client_addr, request->client_addr_len);
    // TODO retransmit
}

/**
 * @brief Checks if two sockaddr_in structures are equal.
 *
 * @param a Pointer to the first sockaddr_in structure.
 * @param b Pointer to the second sockaddr_in structure.
 * @return 0 if the structures are equal, -1 otherwise.
 */
int sockaddr_in_equal(const struct sockaddr_in *a, const struct sockaddr_in *b)
{
    if (a == NULL || b == NULL)
        return -1;
    if (a->sin_family != b->sin_family)
        return -1;
    if (a->sin_port != b->sin_port)
        return -1;
    if (a->sin_addr.s_addr != b->sin_addr.s_addr)
        return -1;
    return 0;
}

/**
 * @brief Processes a SIP INVITE request.
 *
 * @param worker The SIP server worker thread.
 * @param transaction The SIP transaction associated with the request.
 */
void process_invite_request(worker_thread_t *worker, sip_transaction_t *transaction)
{
    if (worker == NULL || transaction == NULL || transaction->message == NULL)
    {
        error("Invalid parameters");
        return;
    }
    log("Processing SIP INVITE request");
    sip_message_t *request = transaction->message;

    if (transaction->dialog == NULL)
    { // new INVITE request

        if (send_100_trying_response_over_transaction(worker->server_socket, transaction) != 0)
        {
            error("Failed to send 100 Trying response");
            send_sip_error_response_over_transaction(worker->server_socket, transaction, RESPONSE_CODE_500, RESPONSE_TEXT_500_INTERNAL_SERVER_ERROR);
            set_transaction_state(transaction, SIP_TRANSACTION_STATE_COMPLETED);
            return;
        }

        set_transaction_state(transaction, SIP_TRANSACTION_STATE_PROCEEDING);

        sip_dialog_t *dialog = create_new_dialog(&worker->dialogs, request->from_tag, request->from_tag_length);
        if (dialog == NULL)
        {
            error("Failed to create new SIP dialog");
            send_sip_error_response_over_transaction(worker->server_socket, transaction, RESPONSE_CODE_500, RESPONSE_TEXT_500_INTERNAL_SERVER_ERROR);
            set_transaction_state(transaction, SIP_TRANSACTION_STATE_COMPLETED);
            return;
        }

        set_transaction_dialog(transaction, dialog);
        set_dialog_state(dialog, SIP_DIALOG_STATE_EARLY);

        // TODO check if call exists, it would be re-INVITE
        sip_call_t *call = create_new_call(&worker->calls, request->call_id, request->call_id_length);
        if (call == NULL)
        {
            error("Failed to create new SIP call");
            send_sip_error_response_over_transaction(worker->server_socket, transaction, RESPONSE_CODE_500, RESPONSE_TEXT_500_INTERNAL_SERVER_ERROR);
            set_transaction_state(transaction, SIP_TRANSACTION_STATE_COMPLETED);
            set_dialog_state(dialog, SIP_DIALOG_STATE_TERMINATED);
            return;
        }

        set_dialog_call(dialog, call);
        set_call_state(call, SIP_CALL_STATE_INCOMING);

        if (send_180_ring_response_over_transaction(worker->server_socket, transaction) != 0)
        {
            error("Failed to send 180 Ringing response");
            send_sip_error_response_over_transaction(worker->server_socket, transaction, RESPONSE_CODE_500, RESPONSE_TEXT_500_INTERNAL_SERVER_ERROR);
            set_transaction_state(transaction, SIP_TRANSACTION_STATE_COMPLETED);
            set_dialog_state(dialog, SIP_DIALOG_STATE_TERMINATED);
            set_call_state(call, SIP_CALL_STATE_FAILED);
            return;
        }

        set_call_state(call, SIP_CALL_STATE_RINGING);
        // TODO to simulate call setup delay, send 200 OK after a short delay in a timer logic
        if (send_sip_200_ok_response_over_transaction(worker->server_socket, transaction) != 0)
        {
            error("Failed to send 200 OK response");
            send_sip_error_response_over_transaction(worker->server_socket, transaction, RESPONSE_CODE_500, RESPONSE_TEXT_500_INTERNAL_SERVER_ERROR);
            set_transaction_state(transaction, SIP_TRANSACTION_STATE_COMPLETED);
            set_dialog_state(dialog, SIP_DIALOG_STATE_TERMINATED);
            set_call_state(call, SIP_CALL_STATE_FAILED);
            return;
        }
        set_transaction_state(transaction, SIP_TRANSACTION_STATE_TERMINATED);
        set_dialog_state(dialog, SIP_DIALOG_STATE_CONFIRMED);
        set_call_state(call, SIP_CALL_STATE_ESTABLISHED);
    }
    else
    {
        // TODO re-INVITE
    }
}

/**
 * @brief Processes a SIP ACK request.
 *
 * @param worker The SIP server worker thread.
 * @param transaction The SIP transaction associated with the request.
 */
void process_ack_request(worker_thread_t *worker, sip_transaction_t *transaction)
{
    if (worker == NULL || transaction == NULL || transaction->message == NULL)
    {
        error("Invalid parameters");
        return;
    }
    log("Processing SIP ACK request");

    if (transaction->state == SIP_TRANSACTION_STATE_COMPLETED && transaction->message->method_type == INVITE && transaction->ack_message != NULL && transaction->ack_message->method_type == ACK)
    {
        // ack for failed INVITE
        log("ACK for failed INVITE");
    }
    else if (transaction->state == SIP_TRANSACTION_STATE_IDLE && transaction->message->method_type == ACK && transaction->dialog != NULL && transaction->dialog->state == SIP_DIALOG_STATE_CONFIRMED)
    {
        // ACK for successful INVITE
        log("ACK for successful INVITE");
    }
    else
    {
        // ACK for other requests
        log("unexpected ACK");
    }
    set_transaction_state(transaction, SIP_TRANSACTION_STATE_TERMINATED);
}

/**
 * @brief Processes a SIP BYE request.
 *
 * @param worker The SIP server worker thread.
 * @param transaction The SIP transaction associated with the request.
 */
void process_bye_request(worker_thread_t *worker, sip_transaction_t *transaction)
{
    if (worker == NULL || transaction == NULL || transaction->message == NULL)
    {
        error("Invalid parameters");
        return;
    }
    log("Processing SIP BYE request");

    if (transaction->dialog == NULL || transaction->dialog->call == NULL)
    {
        send_sip_error_response_over_transaction(worker->server_socket, transaction, RESPONSE_CODE_404, RESPONSE_TEXT_404_NOT_FOUND);
        goto cleanup;
    }

    if (transaction->dialog->state == SIP_DIALOG_STATE_CONFIRMED)
    {
        set_call_state(transaction->dialog->call, SIP_CALL_STATE_TERMINATING);
        send_sip_200_ok_response_over_transaction(worker->server_socket, transaction);
        set_call_state(transaction->dialog->call, SIP_CALL_STATE_TERMINATED);
        set_dialog_state(transaction->dialog, SIP_DIALOG_STATE_TERMINATED);
    }
    else
    {
        send_sip_error_response_over_transaction(worker->server_socket, transaction, RESPONSE_CODE_403, RESPONSE_TEXT_403_FORBIDDEN);
    }

cleanup:
    set_transaction_state(transaction, SIP_TRANSACTION_STATE_TERMINATED);
}

/**
 * @brief Processes a SIP request message.
 *
 * @param worker The SIP server worker thread.
 * @param message The SIP message to process.
 */
void process_sip_request(worker_thread_t *worker, sip_message_t *message)
{
    if (worker == NULL || message == NULL)
    {
        error("Invalid parameters");
        return;
    }

    sip_transaction_t *transaction = worker->transactions ? find_transaction_by_id(worker->transactions, message->branch, message->branch_length) : NULL;
    if (transaction == NULL)
    {
        transaction = create_new_transaction(&worker->transactions, message->branch, message->branch_length);
        if (transaction == NULL)
        {
            error("Failed to create new SIP transaction");
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
            log("Received message is different than existing transaction message.");
            if (transaction->message->method_type == INVITE && message->method_type == ACK)
            {
                log("ACK for INVITE");
                transaction->ack_message = message;
            }
            else
            {
                error("New request for existing transaction branch id");
                cleanup_sip_message(message);
                return;
            }
        }
        else
        {
            if (send_last_response_over_transaction(worker->server_socket, transaction) != 0)
            {
                error("Failed to resend last response over transaction");
                send_sip_error_response_over_transaction(worker->server_socket, transaction, RESPONSE_CODE_500, RESPONSE_TEXT_500_INTERNAL_SERVER_ERROR);
            }
            cleanup_sip_message(message);
            return;
        }
    }

    if (transaction->dialog == NULL)
    {
        sip_dialog_t *dialog = worker->dialogs ? find_dialog_by_id(worker->dialogs, message->from_tag, message->from_tag_length, message->to_tag, message->to_tag_length) : NULL;
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
        error("Unsupported SIP method: %s", message->method);
        send_sip_error_response_over_transaction(worker->server_socket, transaction, RESPONSE_CODE_501, RESPONSE_TEXT_501_NOT_IMPLEMENTED);
        set_transaction_state(transaction, SIP_TRANSACTION_STATE_TERMINATED);
    }
}

/**
 * @brief Processes a SIP response message.
 *
 * @param worker The SIP server worker thread.
 * @param message The SIP message to process.
 */
void process_sip_response(worker_thread_t *worker, sip_message_t *message)
{
    if (worker == NULL || message == NULL)
    {
        error("Invalid parameters");
        return;
    }
    sip_transaction_t *transaction = find_transaction_by_id(worker->transactions, message->branch, message->branch_length);
    if (transaction == NULL)
    {
        error("No matching transaction found for SIP response with branch: %.*s", (int)message->branch_length, message->branch);
        cleanup_sip_message(message);
        return;
    }
    // TODO
    cleanup_sip_message(message);
}

/**
 * @brief Worker thread function to process SIP messages. Parses and processes incoming SIP messages.
 * @param arg Pointer to the worker thread's message queue.
 * @return NULL
 */
void *process_sip_messages(void *arg)
{
    if (arg == NULL)
    {
        error("Invalid parameters");
        return NULL;
    }
    worker_thread_t *worker = (worker_thread_t *)arg;
    message_queue_t *queue = &worker->queue;
    sip_message_t *message;

    while (1)
    {
        if (dequeue_message(queue, &message))
        {
            // Process the SIP message here
            log("Incoming SIP message:\n>>>>>>>>>>>>>>>>>>>>>>>>>\n%s>>>>>>>>>>>>>>>>>>>>>>>>>\n", message->buffer);

            sip_msg_error_t err = parse_message(message);
            if (err != ERROR_NONE)
            {
                error("Failed to parse SIP message: %d", err);
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
