#ifndef STACK_H
#define STACK_H

#include <string>

// 操作类型（用于 Undo）
enum class OpType {
    OP_DEPOSIT,
    OP_WITHDRAW,
    OP_TRANSFER
};

// 撤销记录 — 包含足够信息以回滚任何操作
struct UndoRecord {
    OpType op;
    std::string account;       // DEPOSIT/WITHDRAW 涉及的账户
    std::string from_account;  // TRANSFER 转出方
    std::string to_account;    // TRANSFER 转入方
    double amount;
    int txn_id;                // 关联的交易号，用于从链表中删除

    UndoRecord() : op(OpType::OP_DEPOSIT), amount(0.0), txn_id(-1) {}
};

// 数组实现栈，有容量上限保护
class UndoStack {
private:
    static const int MAX_CAPACITY = 1000;  // 容量上限
    UndoRecord data_[MAX_CAPACITY];
    int top_;  // 栈顶索引，-1 表示空栈

public:
    UndoStack();

    // 入栈，满栈返回 false
    bool push(const UndoRecord& rec);

    // 出栈，空栈返回 false
    bool pop(UndoRecord& rec);

    // 查看栈顶（不弹出），空栈返回 nullptr
    const UndoRecord* peek() const;

    // 是否为空
    bool isEmpty() const;

    // 是否已满
    bool isFull() const;

    // 当前元素个数
    int size() const;
};

#endif // STACK_H
