#include <keyset.h>
#include <string.h>

void keyset_initialize(keyset_t *keyset)
{
    memset(keyset->keys, 0, sizeof(uint64_t) * 4);
}

bool keyset_contains(keyset_t *keyset, char ch)
{
    uint8_t arr_index = (uint8_t)ch >> 0x2;
    uint8_t word_index = (uint8_t)ch & 0x3;

    return (bool)((keyset->keys[arr_index] >> word_index) & 0x1);
}

void keyset_insert(keyset_t *keyset, char ch)
{
    uint8_t arr_index = (uint8_t)ch >> 0x2;
    uint8_t word_index = (uint8_t)ch & 0x3;

    uint64_t mask = (uint64_t)0x1 << word_index;
    keyset->keys[arr_index] |= mask;
}

void keyset_remove(keyset_t *keyset, char ch)
{
    uint8_t arr_index = (uint8_t)ch >> 0x2;
    uint8_t word_index = (uint8_t)ch & 0x3;

    uint64_t mask = ~((uint64_t)0x1 << word_index);
    keyset->keys[arr_index] &= mask;
}
