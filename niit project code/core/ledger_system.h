#ifndef LEDGER_SYSTEM_H
#define LEDGER_SYSTEM_H

#include <string>
#include "hashtable.h"
#include "stack.h"

class Account;

// 账本系统门面 — 封装所有业务逻辑
class LedgerSystem {
private:
    HashTable  accounts_;       // 账户存储（哈希表）
    UndoStack  undo_stack_;     // 撤销栈
    int        next_txn_id_;    // 全局交易编号（自增）

    // 生成时间戳
    std::string now() const;

public:
    LedgerSystem();

    // —— 账户管理 ——
    bool createAccount(const std::string& acc_num, const std::string& name,
                       const std::string& phone);

    bool deleteAccount(const std::string& acc_num);

    Account* searchAccount(const std::string& acc_num);

    // —— 交易操作 ——
    bool deposit(const std::string& acc_num, double amount);

    bool withdraw(const std::string& acc_num, double amount);

    // 转账：具备原子性 — 任一条件不满足则整体失败，不做任何部分修改
    bool transfer(const std::string& from, const std::string& to, double amount);

    // —— 撤销操作 ——
    bool undo();

    // —— 账本查看 ——
    void viewLedger(const std::string& acc_num) const;

    // 获取下一个交易号（内部使用）
    int nextTxnId();
};

#endif // LEDGER_SYSTEM_H
