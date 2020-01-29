#ifndef __KEYSET_H_
#define __KEYSET_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint64_t keys[4];
} keyset_t;

void keyset_initialize(keyset_t *keyset);
bool keyset_contains(keyset_t *keyset, char ch);
void keyset_insert(keyset_t *keyset, char ch);
void keyset_remove(keyset_t *keyset, char ch);

#endif /* __KEYSET_H_ */
