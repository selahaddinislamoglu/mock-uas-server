/**
 * @file sip_message.h
 * @brief Header for SIP message structure.
 */

#ifndef SIP_MESSAGE_H
#define SIP_MESSAGE_H

#include <netinet/in.h>

#define BUFFER_SIZE 1024

/**
 * @struct sip_message_t
 * @brief Structure to hold SIP message data and client address information.
 */
typedef struct
{
    char buffer[BUFFER_SIZE + 1];
    struct sockaddr_in client_addr;
    socklen_t client_addr_len;

    const char *call_id;
    size_t call_id_length;
} sip_message_t;

const char *get_message_call_id(sip_message_t *message, size_t *length);

#endif // SIP_MESSAGE_H