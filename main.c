/**
 * @file main.c
 * @brief Main entry point for the SIP server.
 */

#include "sip_server.h"
#include "network_utils.h"
#include "log.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_THREADS 5
#define QUEUE_CAPACITY 10
#define SIP_PORT 5060

worker_thread_t worker_threads[MAX_THREADS];

void setup_server_socket(int *server_socket, struct sockaddr_in *server_addr);
void handle_new_message(int server_socket);

int main()
{
    int server_socket;
    struct sockaddr_in server_addr;

    // Setup server socket
    setup_server_socket(&server_socket, &server_addr);

    // Initialize worker threads and their queues
    for (int i = 0; i < MAX_THREADS; i++)
    {
        worker_threads[i].calls = NULL;
        worker_threads[i].server_socket = server_socket;
        initialize_message_queue(&worker_threads[i].queue, QUEUE_CAPACITY);
        if (pthread_create(&worker_threads[i].thread, NULL, process_sip_messages, &worker_threads[i]) != 0)
        {
            error("Failed to create worker thread: %s", strerror(errno));
            close(server_socket);
            exit(EXIT_FAILURE);
        }
    }

    // Main server loop
    while (1)
    {
        handle_new_message(server_socket);
    }

    // Cleanup (not reached in current setup)
    for (int i = 0; i < MAX_THREADS; i++)
    {
        pthread_join(worker_threads[i].thread, NULL);
        destroy_message_queue(&worker_threads[i].queue);
    }
    close(server_socket);

    return 0;
}

void setup_server_socket(int *server_socket, struct sockaddr_in *server_addr)
{
    *server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (*server_socket < 0)
    {
        error("Failed to create server socket: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int flags = fcntl(*server_socket, F_GETFL, 0);
    if (flags == -1 || fcntl(*server_socket, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        error("Failed to set non-blocking socket: %s", strerror(errno));
        close(*server_socket);
        exit(EXIT_FAILURE);
    }

    memset(server_addr, 0, sizeof(struct sockaddr_in));
    server_addr->sin_family = AF_INET;
    server_addr->sin_addr.s_addr = INADDR_ANY;
    server_addr->sin_port = htons(SIP_PORT);

    if (bind(*server_socket, (struct sockaddr *)server_addr, sizeof(struct sockaddr_in)) < 0)
    {
        error("Failed to bind server socket: %s", strerror(errno));
        close(*server_socket);
        exit(EXIT_FAILURE);
    }

    info("SIP server started on port %d (non-blocking mode)", SIP_PORT);
}

void handle_new_message(int server_socket)
{
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(server_socket, &read_fds);

    struct timeval tv = {.tv_sec = 5, .tv_usec = 0};

    if (select(server_socket + 1, &read_fds, NULL, NULL, &tv) < 0)
    {
        error("Select error: %s", strerror(errno));
        return;
    }

    if (FD_ISSET(server_socket, &read_fds))
    {
        sip_message_t *message = malloc(sizeof(sip_message_t));
        if (message == NULL)
        {
            error("Memory allocation failed");
            return;
        }
        memset(message, 0, sizeof(sip_message_t));

        message->client_addr_len = sizeof(message->client_addr);
        ssize_t bytes_received = recvfrom(server_socket, message->buffer, sizeof(message->buffer) - 1, 0,
                                          (struct sockaddr *)&message->client_addr, &message->client_addr_len);

        if (bytes_received > 0)
        {
            message->buffer_length = (size_t)bytes_received;
            const char *call_id;
            size_t call_id_length;
            call_id = get_message_call_id(message, &call_id_length);
            if (call_id == NULL)
            {
                error("Received SIP message without Call-ID");
                // TODO: send error response
                free(message);
            }
            else
            {
                log("Received SIP message with Call-ID: %.*s", (int)call_id_length, call_id);
                int hash = string_to_int_hash(call_id, call_id_length);
                int selected_thread = 0;
                if (MAX_THREADS > 0)
                {
                    selected_thread = hash % MAX_THREADS;
                }
                log("Dispatching to worker thread %d", selected_thread);
                if (!enqueue_message(&worker_threads[selected_thread].queue, message))
                {
                    error("Failed to enqueue message");
                    // TODO: send error response
                    free(message);
                }
            }
        }
        else
        {
            if (bytes_received < 0 && errno != EWOULDBLOCK)
            {
                error("Failed to receive SIP message: %s\n", strerror(errno));
            }
            free(message);
        }
    }
}
