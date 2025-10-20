/**
 * @file sip_message.h
 * @brief Header for SIP message structure.
 */

#ifndef SIP_MESSAGE_H
#define SIP_MESSAGE_H

#include <netinet/in.h>
#include <stdbool.h>

#define BUFFER_SIZE 1024

// SIP headers
#define CALL_ID_HEADER_NAME "Call-ID"
#define FROM_HEADER_NAME "From"
#define TO_HEADER_NAME "To"
#define VIA_HEADER_NAME "Via"
#define CSEQ_HEADER_NAME "CSeq"
#define MAX_FORWARD_HEADER_NAME "Max-Forwards"
#define CONTENT_LENGTH_HEADER_NAME "Content-Length"

typedef enum
{
    ERROR_NONE = 0,
    ERROR_MALFORMED_MESSAGE
} sip_msg_error_t;

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

    const char *from;
    size_t from_length;

    const char *to;
    size_t to_length;

    const char *via;
    size_t via_length;

    const char *cseq;
    size_t cseq_length;

    bool is_request;
    int status_code;
    int version_major;
    int version_minor;
    const char *reason;
    size_t reason_length;

    const char *method;
    size_t method_length;

    const char *uri;
    size_t uri_length;

} sip_message_t;

const char *get_header_value(const char *buffer, const char *header_name, size_t *length);
const char *get_message_call_id(sip_message_t *message, size_t *length);
const char *get_message_from(sip_message_t *message, size_t *length);
const char *get_message_to(sip_message_t *message, size_t *length);
const char *get_message_via(sip_message_t *message, size_t *length);
const char *get_message_cseq(sip_message_t *message, size_t *length);

sip_msg_error_t parse_message(sip_message_t *message);

bool is_request(sip_message_t *message);

#endif // SIP_MESSAGE_H