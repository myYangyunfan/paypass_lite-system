#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <string>

// Transaction type
enum class TxnType {
    DEPOSIT,    // Deposit
    WITHDRAW,   // Withdraw
    TRANSFER    // Transfer
};

// Convert enum to readable string
const char* txnTypeToStr(TxnType type);

// Transaction node - basic unit of doubly linked list
struct Transaction {
    int    txn_id;          // Transaction ID, global increment
    std::string from_account;  // Sender account (empty for DEPOSIT/WITHDRAW)
    std::string to_account;    // Receiver account (has value for TRANSFER)
    double amount;             // Amount
    TxnType type;              // Transaction type
    std::string timestamp;     // Timestamp (simplified as string)

    Transaction* prev;         // Previous pointer
    Transaction* next;         // Next pointer

    Transaction(int id, const std::string& from, const std::string& to,
                double amt, TxnType tp, const std::string& ts);
};

// Doubly linked list - with head and tail sentinel nodes
class DoublyLinkedList {
private:
    Transaction* head_sentinel_;  // Head sentinel (no valid data)
    Transaction* tail_sentinel_;  // Tail sentinel (no valid data)
    int count_;                   // Number of valid nodes

public:
    DoublyLinkedList();
    ~DoublyLinkedList();

    // —— No copying ——
    DoublyLinkedList(const DoublyLinkedList&) = delete;
    DoublyLinkedList& operator=(const DoublyLinkedList&) = delete;

    // Append transaction at tail (O(1))
    void pushBack(Transaction* txn);

    // Remove node by txn_id, returns success status
    bool removeById(int txn_id);

    // Return last valid node (for Undo), returns nullptr if empty
    Transaction* last() const;

    // Number of valid nodes
    int size() const;

    // Check if empty
    bool empty() const;

    // Clear all valid nodes
    void clear();

    // Print all transactions (for View Ledger)
    void printAll() const;
};

#endif // TRANSACTION_H
