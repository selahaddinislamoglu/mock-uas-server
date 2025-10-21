/**
 * @file sip_message.h
 * @brief Header for SIP message structure.
 */

#ifndef SIP_MESSAGE_H
#define SIP_MESSAGE_H

#include <netinet/in.h>
#include <stdbool.h>

#define BUFFER_SIZE 1024

#define SIP_PROTOCOL_AND_VERSION "SIP/2.0"

// SIP headers
#define HEADER_NAME_CALL_ID "Call-ID"
#define HEADER_NAME_FROM "From"
#define HEADER_NAME_TO "To"
#define HEADER_NAME_VIA "Via"
#define HEADER_NAME_CSEQ "CSeq"
#define HEADER_NAME_MAX_FORWARDS "Max-Forwards"
#define HEADER_NAME_CONTENT_LENGTH "Content-Length"

typedef enum
{
    ERROR_NONE = 0,
    ERROR_MALFORMED_MESSAGE,
    ERROR_MISSING_MANDATORY_HEADER
} sip_msg_error_t;

#define METHOD_NAME_INVITE "INVITE"
#define METHOD_NAME_ACK "ACK"
#define METHOD_NAME_BYE "BYE"
#define METHOD_NAME_CANCEL "CANCEL"
#define METHOD_NAME_OPTIONS "OPTIONS"
#define METHOD_NAME_REGISTER "REGISTER"
#define METHOD_NAME_PRACK "PRACK"
#define METHOD_NAME_UPDATE "UPDATE"
#define METHOD_NAME_SUBSCRIBE "SUBSCRIBE"
#define METHOD_NAME_NOTIFY "NOTIFY"
#define METHOD_NAME_PUBLISH "PUBLISH"
#define METHOD_NAME_INFO "INFO"
#define METHOD_NAME_REFER "REFER"
#define METHOD_NAME_MESSAGE "MESSAGE"

#define METHOD_SIZE_INVITE 6
#define METHOD_SIZE_ACK 3
#define METHOD_SIZE_BYE 3
#define METHOD_SIZE_CANCEL 6
#define METHOD_SIZE_OPTIONS 7
#define METHOD_SIZE_REGISTER 8
#define METHOD_SIZE_PRACK 5
#define METHOD_SIZE_UPDATE 6
#define METHOD_SIZE_SUBSCRIBE 9
#define METHOD_SIZE_NOTIFY 6
#define METHOD_SIZE_PUBLISH 7
#define METHOD_SIZE_INFO 4
#define METHOD_SIZE_REFER 5
#define METHOD_SIZE_MESSAGE 7

typedef enum
{
    UNKNOWN,
    INVITE,
    ACK,
    BYE,
    CANCEL,
    OPTIONS,
    REGISTER,
    PRACK,
    UPDATE,
    SUBSCRIBE,
    NOTIFY,
    PUBLISH,
    INFO,
    REFER,
    MESSAGE,
} sip_method_t;

typedef enum
{
    RESPONSE_PROVISIONAL_START = 100,
    RESPONSE_PROVISIONAL_END = 199,
    RESPONSE_SUCCESS_START = 200,
    RESPONSE_SUCCESS_END = 299,
    RESPONSE_REDIRECTION_START = 300,
    RESPONSE_REDIRECTION_END = 399,
    RESPONSE_CLIENT_ERROR_START = 400,
    RESPONSE_CLIENT_ERROR_END = 499,
    RESPONSE_SERVER_ERROR_START = 500,
    RESPONSE_SERVER_ERROR_END = 599,
    RESPONSE_GLOBAL_FAILURE_START = 600,
    RESPONSE_GLOBAL_FAILURE_END = 699
} sip_response_range_t;

#define RESPONSE_CODE_100 "100"
#define RESPONSE_CODE_180 "180"
#define RESPONSE_CODE_200 "200"
#define RESPONSE_CODE_300 "300"
#define RESPONSE_CODE_400 "400"
#define RESPONSE_CODE_500 "500"
#define RESPONSE_CODE_600 "600"

#define RESPONSE_TEXT_TRYING "Trying"
#define RESPONSE_TEXT_RINGING "Ringing"
#define RESPONSE_TEXT_OK "OK"

/**
 * @struct sip_message_t
 * @brief Structure to hold SIP message data and client address information.
 */
typedef struct
{
    char buffer[BUFFER_SIZE];
    size_t buffer_length;

    char response[BUFFER_SIZE]; // TODO make it dynamic
    size_t response_length;

    struct sockaddr_in client_addr;
    socklen_t client_addr_len;

    const char *call_id;
    size_t call_id_length;

    const char *from;
    size_t from_length;

    const char *from_tag;
    size_t from_tag_length;

    const char *to;
    size_t to_length;

    const char *to_tag;
    size_t to_tag_length;

    // TODO multiple Via headers support
    const char *via;
    size_t via_length;

    const char *branch;
    size_t branch_length;

    const char *cseq;
    size_t cseq_length;

    bool is_request;
    int status_code;
    int version_major;
    int version_minor;
    const char *reason;
    size_t reason_length;

    sip_method_t method_type;
    const char *method;
    size_t method_length;

    const char *uri;
    size_t uri_length;

} sip_message_t;

void cleanup_sip_message(sip_message_t *message);

const char *get_header_value(const char *buffer, const char *header_name, size_t *length);
const char *get_message_call_id(sip_message_t *message, size_t *length);
const char *get_message_from(sip_message_t *message, size_t *length);
const char *get_message_to(sip_message_t *message, size_t *length);
const char *get_message_via(sip_message_t *message, size_t *length);
const char *get_message_cseq(sip_message_t *message, size_t *length);

const char *get_from_tag(sip_message_t *message, size_t *length);
const char *get_to_tag(sip_message_t *message, size_t *length);
const char *get_branch_param(sip_message_t *message, size_t *length);

sip_msg_error_t parse_message(sip_message_t *message);
sip_method_t get_message_method(sip_message_t *message);

#endif // SIP_MESSAGE_H