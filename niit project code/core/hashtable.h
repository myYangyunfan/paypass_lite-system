#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <string>

class Account;  // 前向声明

// 哈希桶节点（单链表，用于链地址法解决冲突）
struct HashNode {
    std::string key;      // account_number
    Account*    account;  // 指向账户对象
    HashNode*   next;

    HashNode(const std::string& k, Account* a)
        : key(k), account(a), next(nullptr) {}
};

// 哈希表 — 链地址法，静态容量 101
class HashTable {
private:
    static const int TABLE_SIZE = 101;  // 素数，减少冲突
    HashNode* buckets_[TABLE_SIZE];

    // 哈希函数 (djb2 变体)
    int hash(const std::string& key) const;

public:
    HashTable();
    ~HashTable();

    // —— 禁止拷贝 ——
    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;

    // 插入账户（key 为 account_number），key 重复返回 false
    bool insert(const std::string& key, Account* account);

    // 查询，不存在返回 nullptr
    Account* search(const std::string& key) const;

    // 删除（释放 Account 对象），不存在返回 false
    bool remove(const std::string& key);
};

#endif // HASHTABLE_H
