#include "ledger_system.h"
#include "account.h"
#include <iostream>
#include <ctime>
#include <sstream>
#include <iomanip>

LedgerSystem::LedgerSystem() : next_txn_id_(1) {}

std::string LedgerSystem::now() const {
    std::time_t t = std::time(nullptr);
    std::tm* local = std::localtime(&t);
    std::ostringstream oss;
    oss << std::setfill('0')
        << (local->tm_year + 1900) << "-"
        << std::setw(2) << (local->tm_mon + 1) << "-"
        << std::setw(2) << local->tm_mday << " "
        << std::setw(2) << local->tm_hour << ":"
        << std::setw(2) << local->tm_min << ":"
        << std::setw(2) << local->tm_sec;
    return oss.str();
}

int LedgerSystem::nextTxnId() {
    return next_txn_id_++;
}

// ==================== 账户管理 ====================

bool LedgerSystem::createAccount(const std::string& acc_num,
                                  const std::string& name,
                                  const std::string& phone) {
    if (accounts_.search(acc_num)) {
        std::cout << "[ERROR] Account [" << acc_num << "] already exists.\n";
        return false;
    }
    Account* acc = new Account(acc_num, name, phone);
    if (!accounts_.insert(acc_num, acc)) {
        delete acc;
        std::cout << "[ERROR] Failed to create account [" << acc_num << "].\n";
        return false;
    }
    std::cout << "[OK] Account [" << acc_num << "] created for " << name << ".\n";
    return true;
}

bool LedgerSystem::deleteAccount(const std::string& acc_num) {
    if (!accounts_.remove(acc_num)) {
        std::cout << "[ERROR] Account [" << acc_num << "] not found.\n";
        return false;
    }
    std::cout << "[OK] Account [" << acc_num << "] deleted.\n";
    return true;
}

Account* LedgerSystem::searchAccount(const std::string& acc_num) {
    return accounts_.search(acc_num);
}

// ==================== 交易操作 ====================

bool LedgerSystem::deposit(const std::string& acc_num, double amount) {
    if (amount <= 0) {
        std::cout << "[ERROR] Deposit amount must be positive.\n";
        return false;
    }
    Account* acc = accounts_.search(acc_num);
    if (!acc) {
        std::cout << "[ERROR] Account [" << acc_num << "] not found.\n";
        return false;
    }

    int txn_id = nextTxnId();
    std::string ts = now();
    if (!acc->deposit(txn_id, amount, ts)) {
        std::cout << "[ERROR] Deposit failed.\n";
        return false;
    }

    UndoRecord rec;
    rec.op      = OpType::OP_DEPOSIT;
    rec.account = acc_num;
    rec.amount  = amount;
    rec.txn_id  = txn_id;
    undo_stack_.push(rec);

    std::cout << "[OK] Deposit " << std::fixed << std::setprecision(2)
              << amount << " to [" << acc_num << "], balance="
              << acc->getBalance() << "\n";
    return true;
}

bool LedgerSystem::withdraw(const std::string& acc_num, double amount) {
    if (amount <= 0) {
        std::cout << "[ERROR] Withdraw amount must be positive.\n";
        return false;
    }
    Account* acc = accounts_.search(acc_num);
    if (!acc) {
        std::cout << "[ERROR] Account [" << acc_num << "] not found.\n";
        return false;
    }
    if (!acc->canWithdraw(amount)) {
        std::cout << "[ERROR] Insufficient balance in [" << acc_num
                  << "]. Balance=" << acc->getBalance()
                  << ", requested=" << amount << "\n";
        return false;
    }

    int txn_id = nextTxnId();
    std::string ts = now();
    acc->withdraw(txn_id, amount, ts);

    UndoRecord rec;
    rec.op      = OpType::OP_WITHDRAW;
    rec.account = acc_num;
    rec.amount  = amount;
    rec.txn_id  = txn_id;
    undo_stack_.push(rec);

    std::cout << "[OK] Withdraw " << std::fixed << std::setprecision(2)
              << amount << " from [" << acc_num << "], balance="
              << acc->getBalance() << "\n";
    return true;
}

bool LedgerSystem::transfer(const std::string& from, const std::string& to,
                             double amount) {
    if (amount <= 0) {
        std::cout << "[ERROR] Transfer amount must be positive.\n";
        return false;
    }
    if (from == to) {
        std::cout << "[ERROR] Cannot transfer to the same account.\n";
        return false;
    }

    // 原子性保证：先验证所有前置条件，任一失败则不做任何修改
    Account* from_acc = accounts_.search(from);
    if (!from_acc) {
        std::cout << "[ERROR] Source account [" << from << "] not found.\n";
        return false;
    }
    Account* to_acc = accounts_.search(to);
    if (!to_acc) {
        std::cout << "[ERROR] Destination account [" << to << "] not found.\n";
        return false;
    }
    if (!from_acc->canWithdraw(amount)) {
        std::cout << "[ERROR] Insufficient balance in [" << from
                  << "]. Balance=" << from_acc->getBalance()
                  << ", requested=" << amount << "\n";
        return false;
    }

    // 所有校验通过 → 执行转账（使用同一个 txn_id 关联双方账本）
    int txn_id = nextTxnId();
    std::string ts = now();

    from_acc->sendTransfer(txn_id, amount, to, ts);
    to_acc->receiveTransfer(txn_id, amount, from, ts);

    UndoRecord rec;
    rec.op           = OpType::OP_TRANSFER;
    rec.from_account = from;
    rec.to_account   = to;
    rec.amount       = amount;
    rec.txn_id       = txn_id;
    undo_stack_.push(rec);

    std::cout << "[OK] Transfer " << std::fixed << std::setprecision(2)
              << amount << " from [" << from << "] to [" << to << "]\n";
    return true;
}

// ==================== 撤销操作 ====================

bool LedgerSystem::undo() {
    if (undo_stack_.isEmpty()) {
        std::cout << "[INFO] Nothing to undo.\n";
        return false;
    }

    UndoRecord rec;
    undo_stack_.pop(rec);

    switch (rec.op) {
        case OpType::OP_DEPOSIT: {
            Account* acc = accounts_.search(rec.account);
            if (!acc) {
                std::cout << "[ERROR] Undo: account [" << rec.account
                          << "] no longer exists.\n";
                return false;
            }
            acc->forceAdjustBalance(-rec.amount);
            acc->removeTransactionFromLedger(rec.txn_id);
            std::cout << "[UNDO] Deposit of " << std::fixed << std::setprecision(2)
                      << rec.amount << " to [" << rec.account
                      << "] reversed. Balance=" << acc->getBalance() << "\n";
            return true;
        }

        case OpType::OP_WITHDRAW: {
            Account* acc = accounts_.search(rec.account);
            if (!acc) {
                std::cout << "[ERROR] Undo: account [" << rec.account
                          << "] no longer exists.\n";
                return false;
            }
            acc->forceAdjustBalance(rec.amount);
            acc->removeTransactionFromLedger(rec.txn_id);
            std::cout << "[UNDO] Withdrawal of " << std::fixed << std::setprecision(2)
                      << rec.amount << " from [" << rec.account
                      << "] reversed. Balance=" << acc->getBalance() << "\n";
            return true;
        }

        case OpType::OP_TRANSFER: {
            Account* from_acc = accounts_.search(rec.from_account);
            Account* to_acc   = accounts_.search(rec.to_account);
            if (!from_acc || !to_acc) {
                std::cout << "[ERROR] Undo transfer: one or both accounts gone.\n";
                return false;
            }
            // 反向转账：钱从 to 回到 from
            from_acc->forceAdjustBalance(rec.amount);
            to_acc->forceAdjustBalance(-rec.amount);
            from_acc->removeTransactionFromLedger(rec.txn_id);
            to_acc->removeTransactionFromLedger(rec.txn_id);
            std::cout << "[UNDO] Transfer of " << std::fixed << std::setprecision(2)
                      << rec.amount << " [" << rec.from_account << " -> "
                      << rec.to_account << "] reversed.\n";
            return true;
        }

        default:
            std::cout << "[ERROR] Unknown undo operation type.\n";
            return false;
    }
}

// ==================== 账本查看 ====================

void LedgerSystem::viewLedger(const std::string& acc_num) const {
    Account* acc = accounts_.search(acc_num);
    if (!acc) {
        std::cout << "[ERROR] Account [" << acc_num << "] not found.\n";
        return;
    }
    acc->printLedger();
}
