#include "sip_message.h"
#include "log.h"
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define CRLF "\r\n"

/**
 * @brief Frees memory allocated for a SIP message.
 * @param message The SIP message to free.
 */
void cleanup_sip_message(sip_message_t *message)
{
    if (message != NULL)
    {
        free(message);
    }
    else
    {
        log("Message is NULL");
    }
}

/**
 * @brief Retrieves the value of a specific header from a SIP message.
 * @param buffer The buffer containing the SIP message.
 * @param header_name The name of the header to retrieve.
 * @param length Pointer to store the length of the header value.
 * @return The value of the header, or NULL if the header is not found.
 */
const char *get_header_value(const char *buffer, const char *header_name, size_t *length)
{
    if (buffer == NULL || header_name == NULL || length == NULL)
    {
        error("Invalid parameters");
        return NULL;
    }
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

/**
 * @brief Retrieves the value of the "Call-ID" header from a SIP message.
 * @param message The SIP message to retrieve the header value from.
 * @param length Pointer to store the length of the "Call-ID" header value.
 * @return The value of the "Call-ID" header, or NULL if the header is not found.
 */
const char *get_message_call_id(sip_message_t *message, size_t *length)
{
    if (message == NULL || length == NULL)
    {
        error("Invalid parameters");
        return NULL;
    }
    if (message->call_id_length > 0 && message->call_id != NULL)
    {
        *length = message->call_id_length;
        return message->call_id;
    }

    const char *call_id = get_header_value(message->buffer, HEADER_NAME_CALL_ID, length);
    if (call_id != NULL)
    {
        message->call_id = call_id;
        message->call_id_length = *length;
    }
    return call_id;
}

/**
 * @brief Retrieves the value of the "From" header from a SIP message.
 * @param message The SIP message to retrieve the header value from.
 * @param length Pointer to store the length of the "From" header value.
 * @return The value of the "From" header, or NULL if the header is not found.
 */
const char *get_message_from(sip_message_t *message, size_t *length)
{
    if (message == NULL || length == NULL)
    {
        error("Invalid parameters");
        return NULL;
    }
    if (message->from_length > 0 && message->from != NULL)
    {
        *length = message->from_length;
        return message->from;
    }

    const char *from = get_header_value(message->buffer, HEADER_NAME_FROM, length);
    if (from != NULL)
    {
        message->from = from;
        message->from_length = *length;
    }
    return from;
}

/**
 * @brief Retrieves the value of the "To" header from a SIP message.
 * @param message The SIP message to retrieve the header value from.
 * @param length Pointer to store the length of the "To" header value.
 * @return The value of the "To" header, or NULL if the header is not found.
 */
const char *get_message_to(sip_message_t *message, size_t *length)
{
    if (message == NULL || length == NULL)
    {
        error("Invalid parameters");
        return NULL;
    }
    if (message->to_length > 0 && message->to != NULL)
    {
        *length = message->to_length;
        return message->to;
    }

    const char *to = get_header_value(message->buffer, HEADER_NAME_TO, length);
    if (to != NULL)
    {
        message->to = to;
        message->to_length = *length;
    }
    return to;
}

/**
 * @brief Retrieves the value of the "Via" header from a SIP message.
 * @param message The SIP message to retrieve the header value from.
 * @param length Pointer to store the length of the "Via" header value.
 * @return The value of the "Via" header, or NULL if the header is not found.
 */
const char *get_message_via(sip_message_t *message, size_t *length)
{
    if (message == NULL || length == NULL)
    {
        error("Invalid parameters");
        return NULL;
    }
    if (message->via_length > 0 && message->via != NULL)
    {
        *length = message->via_length;
        return message->via;
    }

    const char *via = get_header_value(message->buffer, HEADER_NAME_VIA, length);
    if (via != NULL)
    {
        message->via = via;
        message->via_length = *length;
    }
    return via;
}

/**
 * @brief Retrieves the value of the "CSeq" header from a SIP message.
 * @param message The SIP message to retrieve the header value from.
 * @param length Pointer to store the length of the "CSeq" header value.
 * @return The value of the "CSeq" header, or NULL if the header is not found.
 */
const char *get_message_cseq(sip_message_t *message, size_t *length)
{
    if (message == NULL || length == NULL)
    {
        error("Invalid parameters");
        return NULL;
    }
    if (message->cseq_length > 0 && message->cseq != NULL)
    {
        *length = message->cseq_length;
        return message->cseq;
    }

    const char *cseq = get_header_value(message->buffer, HEADER_NAME_CSEQ, length);
    if (cseq != NULL)
    {
        message->cseq = cseq;
        message->cseq_length = *length;
    }
    return cseq;
}

/**
 * @brief Parses the first line of a SIP message.
 * @param message The SIP message to parse.
 * @return A sip_msg_error_t indicating the result of the parsing operation.
 */
sip_msg_error_t parse_first_line(sip_message_t *message)
{
    if (message == NULL)
    {
        error("Invalid parameters");
        return ERROR_INVALID_PARAMETERS;
    }
    const char *line_end = strstr(message->buffer, CRLF);
    if (line_end == NULL)
    {
        error("First line is malformed");
        return ERROR_MALFORMED_MESSAGE;
    }
    const char *line_start = message->buffer;

    if (strncmp(line_start, "SIP", 3) == 0)
    {
        message->is_request = false;

        const char *dash_pos = strchr(line_start, '/');
        if (dash_pos == NULL || dash_pos + 2 >= line_end)
        {
            error("SIP version is malformed");
            return ERROR_MALFORMED_MESSAGE;
        }
        message->version_major = atoi(dash_pos + 1);
        message->version_minor = atoi(dash_pos + 3);

        if (message->version_major != 2 || message->version_minor != 0)
        {
            error("Unsupported SIP version");
            return ERROR_UNSUPPORTED_SIP_VERSION;
        }

        const char *status_code_start = dash_pos + 4;
        while (status_code_start < line_end && *status_code_start == ' ')
        {
            status_code_start++;
        }
        if (status_code_start >= line_end)
        {
            error("Status code is malformed");
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
            error("Reason is malformed");
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
            error("Request line is malformed");
            return ERROR_MALFORMED_MESSAGE;
        }
        message->method = method;
        message->method_length = space_pos - method;

        sip_method_t method_type = get_message_method(message);
        if (method_type == UNKNOWN)
        {
            error("Method is unknown");
            return ERROR_UNKNOWN_METHOD;
        }

        const char *sip_pos = space_pos + 1;
        while (sip_pos < line_end && *sip_pos == ' ')
        {
            sip_pos++;
        }
        if (sip_pos >= line_end)
        {
            error("URI is malformed");
            return ERROR_MALFORMED_MESSAGE;
        }
        const char *uri_end = strchr(sip_pos, ' ');
        if (uri_end == NULL || uri_end >= line_end)
        {
            error("URI is malformed");
            return ERROR_MALFORMED_MESSAGE;
        }
        message->uri = sip_pos;
        message->uri_length = uri_end - sip_pos;

        // TODO URI check

        const char *dash_pos = strchr(uri_end, '/');
        if (dash_pos == NULL || dash_pos + 2 >= line_end)
        {
            error("SIP version is malformed");
            return ERROR_MALFORMED_MESSAGE;
        }
        message->version_major = atoi(dash_pos + 1);
        message->version_minor = atoi(dash_pos + 3);

        if (message->version_major != 2 || message->version_minor != 0)
        {
            error("Unsupported SIP version");
            return ERROR_UNSUPPORTED_SIP_VERSION;
        }

        return ERROR_NONE;
    }
}

/**
 * @brief Retrieves the "From" tag from a SIP message.
 * @param message The SIP message to retrieve the tag from.
 * @param length The length of the tag, if found.
 * @return A pointer to the tag, or NULL if the tag is not found.
 */
const char *get_from_tag(sip_message_t *message, size_t *length)
{
    if (message->from == NULL || message->from_length == 0)
    {
        error("Invalid parameters");
        return NULL;
    }
    const char *tag_start = strchr(message->from, ';');
    if (tag_start == NULL)
    {
        return NULL;
    }
    tag_start++;
    while (*tag_start == ' ' || *tag_start == '\t')
    {
        tag_start++;
    }
    tag_start += 4;
    const char *tag_end = strchr(tag_start, '\r');
    if (tag_end == NULL)
    {
        tag_end = message->from + message->from_length;
    }
    message->from_tag = tag_start;
    message->from_tag_length = tag_end - tag_start;
    *length = message->from_tag_length;
    return tag_start;
}

/**
 * @brief Retrieves the "To" tag from a SIP message.
 * @param message The SIP message to retrieve the tag from.
 * @param length The length of the tag, if found.
 * @return A pointer to the tag, or NULL if the tag is not found.
 */
const char *get_to_tag(sip_message_t *message, size_t *length)
{
    if (message->to == NULL || message->to_length == 0)
    {
        error("Invalid parameters");
        return NULL;
    }
    const char *tag_start = strchr(message->to, ';');
    if (tag_start == NULL)
    {
        return NULL;
    }
    tag_start++;
    while (*tag_start == ' ' || *tag_start == '\t')
    {
        tag_start++;
    }
    tag_start += 4;
    const char *tag_end = strchr(tag_start, '\r');
    if (tag_end == NULL)
    {
        tag_end = message->to + message->to_length;
    }
    message->to_tag = tag_start;
    message->to_tag_length = tag_end - tag_start;
    *length = message->to_tag_length;
    return tag_start;
}

/**
 * @brief Retrieves the branch parameter from a SIP message.
 * @param message The SIP message to retrieve the branch parameter from.
 * @param length The length of the branch parameter, if found.
 * @return A pointer to the branch parameter, or NULL if the parameter is not found.
 */
const char *get_branch_param(sip_message_t *message, size_t *length)
{
    if (message->via == NULL || message->via_length == 0)
    {
        error("Invalid parameters");
        return NULL;
    }
    const char *branch_start = strchr(message->via, ';');
    if (branch_start == NULL)
    {
        return NULL;
    }
    branch_start++;
    while (*branch_start == ' ' || *branch_start == '\t')
    {
        branch_start++;
    }
    branch_start += 7;
    const char *branch_end = strchr(branch_start, '\r');
    if (branch_end == NULL)
    {
        branch_end = message->via + message->via_length;
    }
    message->branch = branch_start;
    message->branch_length = branch_end - branch_start;
    *length = message->branch_length;
    return branch_start;
}

/**
 * @brief Parses a SIP message. Uses lazy parsing to avoid unnecessary allocations.
 * @param message The SIP message to parse.
 * @return An error code indicating the success or failure of the parsing operation.
 */
sip_msg_error_t parse_message(sip_message_t *message)
{
    const char *header;
    size_t length;

    if (message == NULL)
    {
        error("Invalid parameters");
        return ERROR_INVALID_PARAMETERS;
    }
    // Parse first line
    sip_msg_error_t err = parse_first_line(message);
    if (err != ERROR_NONE)
    {
        error("Failed to parse first line");
        return err;
    }

    sip_method_t method = get_message_method(message);
    if (method == UNKNOWN)
    {
        error("Unknown method");
        return ERROR_UNKNOWN_METHOD;
    }

    // Check mandatory headers
    // Call-ID header checked before calling this function

    // Check for From header
    header = get_message_from(message, &length);
    if (header == NULL || length == 0)
    {
        error("From header is missing");
        return ERROR_MISSING_MANDATORY_HEADER;
    }

    // check for From tag
    header = get_from_tag(message, &length);
    if (header == NULL || length == 0)
    {
        error("From tag parameter is missing in From header");
        return ERROR_MISSING_MANDATORY_PARAMETER;
    }

    // Check for To header
    header = get_message_to(message, &length);
    if (header == NULL || length == 0)
    {
        error("To header is missing");
        return ERROR_MISSING_MANDATORY_HEADER;
    }

    get_to_tag(message, &length);

    // Check for Via header
    header = get_message_via(message, &length);
    if (header == NULL || length == 0)
    {
        error("Via header is missing");
        return ERROR_MISSING_MANDATORY_HEADER;
    }

    // Check for Branch parameter in Via header
    header = get_branch_param(message, &length);
    if (header == NULL || length == 0)
    {
        error("Branch parameter is missing in Via header");
        return ERROR_MISSING_MANDATORY_PARAMETER;
    }

    // Check for CSeq header
    header = get_message_cseq(message, &length);
    if (header == NULL || length == 0)
    {
        error("CSeq header is missing");
        return ERROR_MISSING_MANDATORY_HEADER;
    }

    if (message->is_request)
    {
        // Check for Max-Forwards header
        header = get_header_value(message->buffer, HEADER_NAME_MAX_FORWARDS, &length);
        if (header == NULL || length == 0)
        {
            error("Max-Forwards header is missing");
            return ERROR_MISSING_MANDATORY_HEADER;
        }
    }

    // Check for Content-Length header
    header = get_header_value(message->buffer, HEADER_NAME_CONTENT_LENGTH, &length);
    if (header == NULL || length == 0)
    {
        error("Content-Length header is missing");
        return ERROR_MISSING_MANDATORY_HEADER;
    }

    // TODO additional validations

    return ERROR_NONE;
}

/**
 * @brief Retrieves the method of a SIP message.
 */
sip_method_t get_message_method(sip_message_t *message)
{
    if (message == NULL)
    {
        error("Invalid parameters");
        return UNKNOWN;
    }
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