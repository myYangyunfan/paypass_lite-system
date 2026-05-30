#include "account.h"
#include <iostream>
#include <iomanip>

Account::Account(const std::string& acc_num, const std::string& name,
                 const std::string& phone)
    : account_number_(acc_num), user_name_(name),
      phone_number_(phone), balance_(0.0) {}

const std::string& Account::getAccountNumber() const { return account_number_; }
const std::string& Account::getUserName()     const { return user_name_; }
const std::string& Account::getPhoneNumber()  const { return phone_number_; }
double             Account::getBalance()      const { return balance_; }

bool Account::deposit(int txn_id, double amount, const std::string& timestamp) {
    if (amount <= 0) return false;

    balance_ += amount;
    Transaction* txn = new Transaction(txn_id, "", account_number_,
                                       amount, TxnType::DEPOSIT, timestamp);
    ledger_.pushBack(txn);
    return true;
}

bool Account::withdraw(int txn_id, double amount, const std::string& timestamp) {
    if (amount <= 0)      return false;
    if (!canWithdraw(amount)) return false;

    balance_ -= amount;
    Transaction* txn = new Transaction(txn_id, account_number_, "",
                                       amount, TxnType::WITHDRAW, timestamp);
    ledger_.pushBack(txn);
    return true;
}

bool Account::receiveTransfer(int txn_id, double amount,
                              const std::string& from, const std::string& timestamp) {
    if (amount <= 0) return false;

    balance_ += amount;
    Transaction* txn = new Transaction(txn_id, from, account_number_,
                                       amount, TxnType::TRANSFER, timestamp);
    ledger_.pushBack(txn);
    return true;
}

bool Account::sendTransfer(int txn_id, double amount,
                           const std::string& to, const std::string& timestamp) {
    if (amount <= 0)      return false;
    if (!canWithdraw(amount)) return false;

    balance_ -= amount;
    Transaction* txn = new Transaction(txn_id, account_number_, to,
                                       amount, TxnType::TRANSFER, timestamp);
    ledger_.pushBack(txn);
    return true;
}

void Account::printLedger() const {
    std::cout << "\n=== Ledger for Account [" << account_number_
              << "] " << user_name_ << " ===\n";
    std::cout << "  Current Balance: " << std::fixed << std::setprecision(2)
              << balance_ << "\n";
    ledger_.printAll();
}

const Transaction* Account::getLastTransaction() const {
    return ledger_.last();
}

bool Account::removeTransactionFromLedger(int txn_id) {
    return ledger_.removeById(txn_id);
}

bool Account::canWithdraw(double amount) const {
    return balance_ >= amount;
}

void Account::forceAdjustBalance(double delta) {
    balance_ += delta;  // delta 可为正（加钱）或负（扣钱）
}
