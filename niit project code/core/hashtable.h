#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "account.h"

#define HT_SIZE 101

typedef struct HashNode {
    char key[64];
    Account *account;
    struct HashNode *next;
} HashNode;

typedef struct {
    HashNode *buckets[HT_SIZE];
} HashTable;

void ht_init(HashTable *ht);
void ht_destroy(HashTable *ht);
int ht_insert(HashTable *ht, const char *key, Account *account);
Account* ht_search(HashTable *ht, const char *key);
int ht_remove(HashTable *ht, const char *key);

#endif
