#include "transaction.h"
#include <iostream>
#include <iomanip>

const char* txnTypeToStr(TxnType type) {
    switch (type) {
        case TxnType::DEPOSIT:  return "DEPOSIT";
        case TxnType::WITHDRAW: return "WITHDRAW";
        case TxnType::TRANSFER: return "TRANSFER";
        default:                return "UNKNOWN";
    }
}

// ==================== Transaction ====================
Transaction::Transaction(int id, const std::string& from, const std::string& to,
                         double amt, TxnType tp, const std::string& ts)
    : txn_id(id), from_account(from), to_account(to),
      amount(amt), type(tp), timestamp(ts),
      prev(nullptr), next(nullptr) {}

// ==================== DoublyLinkedList ====================

DoublyLinkedList::DoublyLinkedList() : count_(0) {
    // 创建头尾哨兵
    head_sentinel_ = new Transaction(-1, "", "", 0.0, TxnType::DEPOSIT, "");
    tail_sentinel_ = new Transaction(-1, "", "", 0.0, TxnType::DEPOSIT, "");
    head_sentinel_->next = tail_sentinel_;
    tail_sentinel_->prev = head_sentinel_;
}

DoublyLinkedList::~DoublyLinkedList() {
    clear();
    delete head_sentinel_;
    delete tail_sentinel_;
}

void DoublyLinkedList::pushBack(Transaction* txn) {
    if (!txn) return;

    // 插入到尾哨兵之前
    Transaction* before = tail_sentinel_->prev;
    txn->prev = before;
    txn->next = tail_sentinel_;
    before->next = txn;
    tail_sentinel_->prev = txn;
    ++count_;
}

bool DoublyLinkedList::removeById(int txn_id) {
    Transaction* cur = head_sentinel_->next;
    while (cur != tail_sentinel_) {
        if (cur->txn_id == txn_id) {
            cur->prev->next = cur->next;
            cur->next->prev = cur->prev;
            delete cur;
            --count_;
            return true;
        }
        cur = cur->next;
    }
    return false;
}

Transaction* DoublyLinkedList::last() const {
    if (empty()) return nullptr;
    return tail_sentinel_->prev;
}

int DoublyLinkedList::size() const {
    return count_;
}

bool DoublyLinkedList::empty() const {
    return count_ == 0;
}

void DoublyLinkedList::clear() {
    Transaction* cur = head_sentinel_->next;
    while (cur != tail_sentinel_) {
        Transaction* nxt = cur->next;
        delete cur;
        cur = nxt;
    }
    head_sentinel_->next = tail_sentinel_;
    tail_sentinel_->prev = head_sentinel_;
    count_ = 0;
}

void DoublyLinkedList::printAll() const {
    if (empty()) {
        std::cout << "  (No transactions)\n";
        return;
    }
    std::cout << "  " << std::setw(4)  << "ID"
              << "  " << std::setw(10) << "Type"
              << "  " << std::setw(12) << "Amount"
              << "  " << std::setw(15) << "From"
              << "  " << std::setw(15) << "To"
              << "  " << "Time" << "\n";
    std::cout << "  " << std::string(80, '-') << "\n";

    Transaction* cur = head_sentinel_->next;
    while (cur != tail_sentinel_) {
        std::cout << "  " << std::setw(4)  << cur->txn_id
                  << "  " << std::setw(10) << txnTypeToStr(cur->type)
                  << "  " << std::setw(12) << std::fixed << std::setprecision(2) << cur->amount
                  << "  " << std::setw(15) << (cur->from_account.empty() ? "-" : cur->from_account)
                  << "  " << std::setw(15) << (cur->to_account.empty()   ? "-" : cur->to_account)
                  << "  " << cur->timestamp << "\n";
        cur = cur->next;
    }
}
