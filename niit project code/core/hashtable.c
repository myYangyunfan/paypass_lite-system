#include "hashtable.h"
#include <stdlib.h>
#include <string.h>

static unsigned long ht_hash(const char *key) {
    unsigned long h = 5381;
    int c;
    while ((c = (unsigned char)*key++)) {
        h = ((h << 5) + h) + c;
    }
    return h % HT_SIZE;
}

void ht_init(HashTable *ht) {
    int i;
    for (i = 0; i < HT_SIZE; i++) {
        ht->buckets[i] = NULL;
    }
}

void ht_destroy(HashTable *ht) {
    int i;
    for (i = 0; i < HT_SIZE; i++) {
        HashNode *cur = ht->buckets[i];
        while (cur) {
            HashNode *nxt = cur->next;
            account_destroy(cur->account);
            free(cur);
            cur = nxt;
        }
        ht->buckets[i] = NULL;
    }
}

int ht_insert(HashTable *ht, const char *key, Account *account) {
    if (!account) return 0;
    unsigned long idx = ht_hash(key);
    HashNode *cur = ht->buckets[idx];
    while (cur) {
        if (strcmp(cur->key, key) == 0) {
            return 0;
        }
        cur = cur->next;
    }
    HashNode *node = (HashNode*)malloc(sizeof(HashNode));
    if (!node) return 0;
    strncpy(node->key, key, 63);
    node->key[63] = '\0';
    node->account = account;
    node->next = ht->buckets[idx];
    ht->buckets[idx] = node;
    return 1;
}

Account* ht_search(HashTable *ht, const char *key) {
    unsigned long idx = ht_hash(key);
    HashNode *cur = ht->buckets[idx];
    while (cur) {
        if (strcmp(cur->key, key) == 0) {
            return cur->account;
        }
        cur = cur->next;
    }
    return NULL;
}

int ht_remove(HashTable *ht, const char *key) {
    unsigned long idx = ht_hash(key);
    HashNode *cur = ht->buckets[idx];
    HashNode *prev = NULL;
    while (cur) {
        if (strcmp(cur->key, key) == 0) {
            if (prev) {
                prev->next = cur->next;
            } else {
                ht->buckets[idx] = cur->next;
            }
            account_destroy(cur->account);
            free(cur);
            return 1;
        }
        prev = cur;
        cur = cur->next;
    }
    return 0;
}
