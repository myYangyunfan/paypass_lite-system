#include "hashtable.h"
#include "account.h"

int HashTable::hash(const std::string& key) const {
    // djb2 hash algorithm - even distribution, good for string keys
    unsigned long h = 5381;
    for (char c : key) {
        h = ((h << 5) + h) + static_cast<unsigned char>(c);  // h * 33 + c
    }
    return static_cast<int>(h % TABLE_SIZE);
}

HashTable::HashTable() {
    for (int i = 0; i < TABLE_SIZE; ++i) {
        buckets_[i] = nullptr;
    }
}

HashTable::~HashTable() {
    for (int i = 0; i < TABLE_SIZE; ++i) {
        HashNode* cur = buckets_[i];
        while (cur) {
            HashNode* nxt = cur->next;
            delete cur->account;  // Free Account object
            delete cur;           // Free HashNode
            cur = nxt;
        }
        buckets_[i] = nullptr;
    }
}

bool HashTable::insert(const std::string& key, Account* account) {
    if (!account) return false;

    // Check if key already exists
    int idx = hash(key);
    HashNode* cur = buckets_[idx];
    while (cur) {
        if (cur->key == key) {
            return false;  // Duplicate key, insertion failed
        }
        cur = cur->next;
    }

    // Head insertion: new node inserted at bucket list head (O(1))
    HashNode* node = new HashNode(key, account);
    node->next = buckets_[idx];
    buckets_[idx] = node;
    return true;
}

Account* HashTable::search(const std::string& key) const {
    int idx = hash(key);
    HashNode* cur = buckets_[idx];
    while (cur) {
        if (cur->key == key) {
            return cur->account;
        }
        cur = cur->next;
    }
    return nullptr;
}

bool HashTable::remove(const std::string& key) {
    int idx = hash(key);
    HashNode* cur  = buckets_[idx];
    HashNode* prev = nullptr;

    while (cur) {
        if (cur->key == key) {
            if (prev) {
                prev->next = cur->next;
            } else {
                buckets_[idx] = cur->next;
            }
            delete cur->account;  // Free Account
            delete cur;           // Free HashNode
            return true;
        }
        prev = cur;
        cur = cur->next;
    }
    return false;
}
