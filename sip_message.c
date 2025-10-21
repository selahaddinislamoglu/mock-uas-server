#include "sip_message.h"
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define CRLF "\r\n"

const char *get_header_value(const char *buffer, const char *header_name, size_t *length)
{
    size_t header_name_len = strlen(header_name);
    const char *line_start = buffer;
    const char *line_end;

    while ((line_end = strstr(line_start, CRLF)) != NULL)
    {
        if (strncasecmp(line_start, header_name, header_name_len) == 0)
        {
            // Found the header line
            const char *value_start = line_start + header_name_len;
            while (*value_start == ':' || *value_start == ' ' || *value_start == '\t')
            {
                value_start++;
            }

            size_t value_len = line_end - value_start;
            *length = value_len;
            return value_start;
        }
        line_start = line_end + 2; // Move to the start of the next line
    }

    return NULL;
}

const char *get_message_call_id(sip_message_t *message, size_t *length)
{
    if (message->call_id_length > 0 && message->call_id != NULL)
    {
        *length = message->call_id_length;
        return message->call_id;
    }

    const char *call_id = get_header_value(message->buffer, CALL_ID_HEADER_NAME, length);
    if (call_id != NULL)
    {
        message->call_id = call_id;
        message->call_id_length = *length;
    }
    return call_id;
}

const char *get_message_from(sip_message_t *message, size_t *length)
{
    if (message->from_length > 0 && message->from != NULL)
    {
        *length = message->from_length;
        return message->from;
    }

    const char *from = get_header_value(message->buffer, FROM_HEADER_NAME, length);
    if (from != NULL)
    {
        message->from = from;
        message->from_length = *length;
    }
    return from;
}

const char *get_message_to(sip_message_t *message, size_t *length)
{
    if (message->to_length > 0 && message->to != NULL)
    {
        *length = message->to_length;
        return message->to;
    }

    const char *to = get_header_value(message->buffer, TO_HEADER_NAME, length);
    if (to != NULL)
    {
        message->to = to;
        message->to_length = *length;
    }
    return to;
}

const char *get_message_via(sip_message_t *message, size_t *length)
{
    if (message->via_length > 0 && message->via != NULL)
    {
        *length = message->via_length;
        return message->via;
    }

    const char *via = get_header_value(message->buffer, VIA_HEADER_NAME, length);
    if (via != NULL)
    {
        message->via = via;
        message->via_length = *length;
    }
    return via;
}

const char *get_message_cseq(sip_message_t *message, size_t *length)
{
    if (message->cseq_length > 0 && message->cseq != NULL)
    {
        *length = message->cseq_length;
        return message->cseq;
    }

    const char *cseq = get_header_value(message->buffer, CSEQ_HEADER_NAME, length);
    if (cseq != NULL)
    {
        message->cseq = cseq;
        message->cseq_length = *length;
    }
    return cseq;
}

sip_msg_error_t parse_first_line(sip_message_t *message)
{
    const char *line_end = strstr(message->buffer, CRLF);
    if (line_end == NULL)
    {
        printf("First line is malformed\n");
        return ERROR_MALFORMED_MESSAGE;
    }
    const char *line_start = message->buffer;

    if (strncmp(line_start, "SIP", 3) == 0)
    {
        message->is_request = false;

        const char *dash_pos = strchr(line_start, '/');
        if (dash_pos == NULL || dash_pos + 2 >= line_end)
        {
            printf("SIP version is malformed\n");
            return ERROR_MALFORMED_MESSAGE;
        }
        message->version_major = atoi(dash_pos + 1);
        message->version_minor = atoi(dash_pos + 3);

        // TODO version check

        const char *status_code_start = dash_pos + 4;
        while (status_code_start < line_end && *status_code_start == ' ')
        {
            status_code_start++;
        }
        if (status_code_start >= line_end)
        {
            printf("Status code is malformed\n");
            return ERROR_MALFORMED_MESSAGE;
        }
        message->status_code = atoi(status_code_start);

        // TODO status code check

        const char *reason_start = status_code_start;
        while (reason_start < line_end && *reason_start != ' ')
        {
            reason_start++;
        }
        if (reason_start >= line_end)
        {
            printf("Reason is malformed\n");
            return ERROR_MALFORMED_MESSAGE;
        }
        message->reason = reason_start + 1;
        message->reason_length = line_end - message->reason;

        // TODO reason check

        return ERROR_NONE;
    }
    else
    {
        message->is_request = true;

        const char *method = line_start;
        const char *space_pos = strchr(method, ' ');
        if (space_pos == NULL || space_pos >= line_end)
        {
            printf("Request line is malformed\n");
            return ERROR_MALFORMED_MESSAGE;
        }
        message->method = method;
        message->method_length = space_pos - method;

        // TODO method check

        const char *sip_pos = space_pos + 1;
        while (sip_pos < line_end && *sip_pos == ' ')
        {
            sip_pos++;
        }
        if (sip_pos >= line_end)
        {
            printf("URI is malformed\n");
            return ERROR_MALFORMED_MESSAGE;
        }
        const char *uri_end = strchr(sip_pos, ' ');
        if (uri_end == NULL || uri_end >= line_end)
        {
            printf("URI is malformed\n");
            return ERROR_MALFORMED_MESSAGE;
        }
        message->uri = sip_pos;
        message->uri_length = uri_end - sip_pos;

        // TODO URI check

        const char *dash_pos = strchr(uri_end, '/');
        if (dash_pos == NULL || dash_pos + 2 >= line_end)
        {
            printf("SIP version is malformed\n");
            return ERROR_MALFORMED_MESSAGE;
        }
        message->version_major = atoi(dash_pos + 1);
        message->version_minor = atoi(dash_pos + 3);

        // TODO version check

        return ERROR_NONE;
    }
}

sip_msg_error_t parse_message(sip_message_t *message)
{
    const char *header;
    size_t length;

    // Parse first line
    sip_msg_error_t err = parse_first_line(message);
    if (err != ERROR_NONE)
    {
        return err;
    }

    // Check mandatory headers
    // Call-ID header checked before calling this function

    // Check for From header
    header = get_message_from(message, &length);
    if (header == NULL || length == 0)
    {
        return false;
    }

    // Check for To header
    header = get_message_to(message, &length);
    if (header == NULL || length == 0)
    {
        return false;
    }

    // Check for Via header
    header = get_message_via(message, &length);
    if (header == NULL || length == 0)
    {
        return false;
    }

    // Check for CSeq header
    header = get_message_cseq(message, &length);
    if (header == NULL || length == 0)
    {
        return false;
    }

    if (message->is_request)
    {
        // Check for Max-Forwards header
        header = get_header_value(message->buffer, MAX_FORWARD_HEADER_NAME, &length);
        if (header == NULL || length == 0)
        {
            return false;
        }
    }

    // Check for Content-Length header
    header = get_header_value(message->buffer, CONTENT_LENGTH_HEADER_NAME, &length);
    if (header == NULL || length == 0)
    {
        return false;
    }

    // TODO additional validations

    return true;
}

sip_method_t get_message_method(sip_message_t *message)
{
    if (!message->is_request)
    {
        return UNKNOWN;
    }

    if (message->method_type != UNKNOWN)
    {
        return message->method_type;
    }

    if (message->method_length == METHOD_SIZE_INVITE && strncasecmp(message->method, METHOD_NAME_INVITE, METHOD_SIZE_INVITE) == 0)
    {
        message->method_type = INVITE;
    }
    else if (message->method_length == METHOD_SIZE_ACK && strncasecmp(message->method, METHOD_NAME_ACK, METHOD_SIZE_ACK) == 0)
    {
        message->method_type = ACK;
    }
    else if (message->method_length == METHOD_SIZE_BYE && strncasecmp(message->method, METHOD_NAME_BYE, METHOD_SIZE_BYE) == 0)
    {
        message->method_type = BYE;
    }
    else if (message->method_length == METHOD_SIZE_CANCEL && strncasecmp(message->method, METHOD_NAME_CANCEL, METHOD_SIZE_CANCEL) == 0)
    {
        message->method_type = CANCEL;
    }
    else if (message->method_length == METHOD_SIZE_OPTIONS && strncasecmp(message->method, METHOD_NAME_OPTIONS, METHOD_SIZE_OPTIONS) == 0)
    {
        message->method_type = OPTIONS;
    }
    else if (message->method_length == METHOD_SIZE_REGISTER && strncasecmp(message->method, METHOD_NAME_REGISTER, METHOD_SIZE_REGISTER) == 0)
    {
        message->method_type = REGISTER;
    }
    else if (message->method_length == METHOD_SIZE_PRACK && strncasecmp(message->method, METHOD_NAME_PRACK, METHOD_SIZE_PRACK) == 0)
    {
        message->method_type = PRACK;
    }
    else if (message->method_length == METHOD_SIZE_UPDATE && strncasecmp(message->method, METHOD_NAME_UPDATE, METHOD_SIZE_UPDATE) == 0)
    {
        message->method_type = UPDATE;
    }
    else if (message->method_length == METHOD_SIZE_SUBSCRIBE && strncasecmp(message->method, METHOD_NAME_SUBSCRIBE, METHOD_SIZE_SUBSCRIBE) == 0)
    {
        message->method_type = SUBSCRIBE;
    }
    else if (message->method_length == METHOD_SIZE_NOTIFY && strncasecmp(message->method, METHOD_NAME_NOTIFY, METHOD_SIZE_NOTIFY) == 0)
    {
        message->method_type = NOTIFY;
    }
    else if (message->method_length == METHOD_SIZE_PUBLISH && strncasecmp(message->method, METHOD_NAME_PUBLISH, METHOD_SIZE_PUBLISH) == 0)
    {
        message->method_type = PUBLISH;
    }
    else if (message->method_length == METHOD_SIZE_INFO && strncasecmp(message->method, METHOD_NAME_INFO, METHOD_SIZE_INFO) == 0)
    {
        message->method_type = INFO;
    }
    else if (message->method_length == METHOD_SIZE_REFER && strncasecmp(message->method, METHOD_NAME_REFER, METHOD_SIZE_REFER) == 0)
    {
        message->method_type = REFER;
    }
    else if (message->method_length == METHOD_SIZE_MESSAGE && strncasecmp(message->method, METHOD_NAME_MESSAGE, METHOD_SIZE_MESSAGE) == 0)
    {
        message->method_type = MESSAGE;
    }
    else
    {
        message->method_type = UNKNOWN;
    }

    return message->method_type;
}