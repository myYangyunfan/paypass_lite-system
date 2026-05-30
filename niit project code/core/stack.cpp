#include "stack.h"

UndoStack::UndoStack() : top_(-1) {}

bool UndoStack::push(const UndoRecord& rec) {
    if (isFull()) return false;
    data_[++top_] = rec;
    return true;
}

bool UndoStack::pop(UndoRecord& rec) {
    if (isEmpty()) return false;
    rec = data_[top_--];
    return true;
}

const UndoRecord* UndoStack::peek() const {
    if (isEmpty()) return nullptr;
    return &data_[top_];
}

bool UndoStack::isEmpty() const {
    return top_ < 0;
}

bool UndoStack::isFull() const {
    return top_ >= MAX_CAPACITY - 1;
}

int UndoStack::size() const {
    return top_ + 1;
}
