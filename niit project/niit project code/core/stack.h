#ifndef STACK_H
#define STACK_H

#define OP_DEPOSIT  0
#define OP_WITHDRAW 1
#define OP_TRANSFER 2

#define MAX_UNDO 1000

typedef struct {
    int op;
    char account[64];
    char from_account[64];
    char to_account[64];
    double amount;
    int txn_id;
} UndoRecord;

typedef struct {
    UndoRecord data[MAX_UNDO];
    int top;
} UndoStack;

void stack_init(UndoStack *s);
int stack_push(UndoStack *s, UndoRecord *rec);
int stack_pop(UndoStack *s, UndoRecord *rec);
int stack_empty(UndoStack *s);
int stack_full(UndoStack *s);
int stack_size(UndoStack *s);

#endif
