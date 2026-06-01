#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <string>

// 交易类型
enum class TxnType {
    DEPOSIT,    // 存款
    WITHDRAW,   // 取款
    TRANSFER    // 转账
};

// 将枚举转为可读字符串
const char* txnTypeToStr(TxnType type);

// 交易节点 — 双向链表的基本单元
struct Transaction {
    int    txn_id;          // 交易编号，全局自增
    std::string from_account;  // 转出方（DEPOSIT/WITHDRAW 时为空）
    std::string to_account;    // 转入方（TRANSFER 时有值）
    double amount;             // 金额
    TxnType type;              // 交易类型
    std::string timestamp;     // 时间戳（简化用字符串）

    Transaction* prev;         // 前驱指针
    Transaction* next;         // 后继指针

    Transaction(int id, const std::string& from, const std::string& to,
                double amt, TxnType tp, const std::string& ts);
};

// 双向链表 — 带头尾哨兵节点
class DoublyLinkedList {
private:
    Transaction* head_sentinel_;  // 头哨兵（不存有效数据）
    Transaction* tail_sentinel_;  // 尾哨兵（不存有效数据）
    int count_;                   // 有效节点数

public:
    DoublyLinkedList();
    ~DoublyLinkedList();

    // —— 禁止拷贝 ——
    DoublyLinkedList(const DoublyLinkedList&) = delete;
    DoublyLinkedList& operator=(const DoublyLinkedList&) = delete;

    // 在尾部追加交易（O(1)）
    void pushBack(Transaction* txn);

    // 按 txn_id 删除指定节点，返回是否成功
    bool removeById(int txn_id);

    // 返回最后一个有效节点（用于 Undo），空链表返回 nullptr
    Transaction* last() const;

    // 有效节点数
    int size() const;

    // 是否为空
    bool empty() const;

    // 清空所有有效节点
    void clear();

    // 打印所有交易（供 View Ledger 使用）
    void printAll() const;
};

#endif // TRANSACTION_H
