#include "utils.h"

int string_to_int_hash(const char *str, size_t length)
{
    int hash = 0;
    for (size_t i = 0; i < length; i++)
    {
        hash += str[i];
    }
    return hash;
}