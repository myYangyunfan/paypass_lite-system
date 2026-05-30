#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <string>
#include "transaction.h"

// 账户类 — 内含双向链表账本
class Account {
private:
    std::string account_number_;
    std::string user_name_;
    std::string phone_number_;
    double balance_;               // 当前余额
    DoublyLinkedList ledger_;      // 交易历史（双向链表）

public:
    Account(const std::string& acc_num, const std::string& name,
            const std::string& phone);

    // —— 禁止拷贝 (管理动态内存) ——
    Account(const Account&) = delete;
    Account& operator=(const Account&) = delete;

    // —— 访问器 ——
    const std::string& getAccountNumber() const;
    const std::string& getUserName()     const;
    const std::string& getPhoneNumber()  const;
    double             getBalance()      const;

    // —— 交易操作 ——
    // 存款，返回是否成功
    bool deposit(int txn_id, double amount, const std::string& timestamp);

    // 取款，余额不足返回 false
    bool withdraw(int txn_id, double amount, const std::string& timestamp);

    // 接收转账（对方→本账户）
    bool receiveTransfer(int txn_id, double amount,
                         const std::string& from, const std::string& timestamp);

    // 发出转账（本账户→对方），余额不足返回 false
    bool sendTransfer(int txn_id, double amount,
                      const std::string& to, const std::string& timestamp);

    // —— 账本操作 ——
    void printLedger() const;
    const Transaction* getLastTransaction() const;
    bool removeTransactionFromLedger(int txn_id);

    // 余额检查
    bool canWithdraw(double amount) const;

    // 强制调整余额（仅用于 Undo 操作，不产生新交易记录）
    void forceAdjustBalance(double delta);
};

#endif // ACCOUNT_H
