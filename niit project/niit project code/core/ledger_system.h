#ifndef LEDGER_SYSTEM_H
#define LEDGER_SYSTEM_H

#include "hashtable.h"
#include "stack.h"

typedef struct {
    HashTable accounts;
    UndoStack undo_stack;
    int next_txn_id;
} LedgerSystem;

void ls_init(LedgerSystem *sys);
void ls_destroy(LedgerSystem *sys);
int ls_create_account(LedgerSystem *sys, const char *acc_num, const char *name, const char *phone);
int ls_delete_account(LedgerSystem *sys, const char *acc_num);
Account* ls_search_account(LedgerSystem *sys, const char *acc_num);
int ls_deposit(LedgerSystem *sys, const char *acc_num, double amount);
int ls_withdraw(LedgerSystem *sys, const char *acc_num, double amount);
int ls_transfer(LedgerSystem *sys, const char *from, const char *to, double amount);
int ls_undo(LedgerSystem *sys);
void ls_view_ledger(LedgerSystem *sys, const char *acc_num);

#endif
