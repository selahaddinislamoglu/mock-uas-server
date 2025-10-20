#include "sip_message.h"
#include <stddef.h>
#include <string.h>

#define CRLF "\r\n"
#define CALL_ID_HEADER_NAME "Call-ID"

static const char *get_header_value(const char *buffer, const char *header_name, size_t *length)
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