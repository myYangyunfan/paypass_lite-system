#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <string>

class Account;  // Forward declaration

// Hash bucket node (singly linked list for chaining to resolve collisions)
struct HashNode {
    std::string key;      // account_number
    Account*    account;  // Pointer to account object
    HashNode*   next;

    HashNode(const std::string& k, Account* a)
        : key(k), account(a), next(nullptr) {}
};

// Hash table - chaining method, static capacity 101
class HashTable {
private:
    static const int TABLE_SIZE = 101;  // Prime number to reduce collisions
    HashNode* buckets_[TABLE_SIZE];

    // Hash function (djb2 variant)
    int hash(const std::string& key) const;

public:
    HashTable();
    ~HashTable();

    // —— No copying ——
    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;

    // Insert account (key is account_number), returns false if key already exists
    bool insert(const std::string& key, Account* account);

    // Search, returns nullptr if not found
    Account* search(const std::string& key) const;

    // Remove (frees Account object), returns false if not found
    bool remove(const std::string& key);
};

#endif // HASHTABLE_H
