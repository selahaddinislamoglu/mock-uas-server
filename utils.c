#include "utils.h"
#include "log.h"

int string_to_int_hash(const char *str, size_t length)
{
    if (str == NULL || length == 0)
    {
        error("Invalid parameters");
        return 0;
    }
    int hash = 0;
    for (size_t i = 0; i < length; i++)
    {
        hash += str[i];
    }
    return hash;
}