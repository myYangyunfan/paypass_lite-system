#include "ledger_system.h"
#include "account.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

static void ls_now(char *buf, int size) {
    time_t t = time(NULL);
    struct tm *local = localtime(&t);
    snprintf(buf, size, "%04d-%02d-%02d %02d:%02d:%02d",
             local->tm_year + 1900,
             local->tm_mon + 1,
             local->tm_mday,
             local->tm_hour,
             local->tm_min,
             local->tm_sec);
}

static int ls_next_txn_id(LedgerSystem *sys) {
    return sys->next_txn_id++;
}

void ls_init(LedgerSystem *sys) {
    ht_init(&sys->accounts);
    stack_init(&sys->undo_stack);
    sys->next_txn_id = 1;
}

void ls_destroy(LedgerSystem *sys) {
    ht_destroy(&sys->accounts);
}

int ls_create_account(LedgerSystem *sys, const char *acc_num, const char *name, const char *phone) {
    if (ht_search(&sys->accounts, acc_num)) {
        printf("[ERROR] Account [%s] already exists.\n", acc_num);
        return 0;
    }
    Account *acc = account_create(acc_num, name, phone);
    if (!acc) {
        printf("[ERROR] Failed to create account [%s].\n", acc_num);
        return 0;
    }
    if (!ht_insert(&sys->accounts, acc_num, acc)) {
        account_destroy(acc);
        printf("[ERROR] Failed to create account [%s].\n", acc_num);
        return 0;
    }
    printf("[OK] Account [%s] created for %s.\n", acc_num, name);
    return 1;
}

int ls_delete_account(LedgerSystem *sys, const char *acc_num) {
    if (!ht_remove(&sys->accounts, acc_num)) {
        printf("[ERROR] Account [%s] not found.\n", acc_num);
        return 0;
    }
    printf("[OK] Account [%s] deleted.\n", acc_num);
    return 1;
}

Account* ls_search_account(LedgerSystem *sys, const char *acc_num) {
    return ht_search(&sys->accounts, acc_num);
}

int ls_deposit(LedgerSystem *sys, const char *acc_num, double amount) {
    if (amount <= 0) {
        printf("[ERROR] Deposit amount must be positive.\n");
        return 0;
    }
    Account *acc = ht_search(&sys->accounts, acc_num);
    if (!acc) {
        printf("[ERROR] Account [%s] not found.\n", acc_num);
        return 0;
    }
    int txn_id = ls_next_txn_id(sys);
    char ts[32];
    ls_now(ts, sizeof(ts));
    if (!account_deposit(acc, txn_id, amount, ts)) {
        printf("[ERROR] Deposit failed.\n");
        return 0;
    }
    UndoRecord rec;
    memset(&rec, 0, sizeof(rec));
    rec.op = OP_DEPOSIT;
    strncpy(rec.account, acc_num, 63);
    rec.account[63] = '\0';
    rec.amount = amount;
    rec.txn_id = txn_id;
    stack_push(&sys->undo_stack, &rec);
    printf("[OK] Deposit %.2f to [%s], balance=%.2f\n", amount, acc_num, acc->balance);
    return 1;
}

int ls_withdraw(LedgerSystem *sys, const char *acc_num, double amount) {
    if (amount <= 0) {
        printf("[ERROR] Withdraw amount must be positive.\n");
        return 0;
    }
    Account *acc = ht_search(&sys->accounts, acc_num);
    if (!acc) {
        printf("[ERROR] Account [%s] not found.\n", acc_num);
        return 0;
    }
    if (!account_can_withdraw(acc, amount)) {
        printf("[ERROR] Insufficient balance in [%s]. Balance=%.2f, requested=%.2f\n",
               acc_num, acc->balance, amount);
        return 0;
    }
    int txn_id = ls_next_txn_id(sys);
    char ts[32];
    ls_now(ts, sizeof(ts));
    account_withdraw(acc, txn_id, amount, ts);
    UndoRecord rec;
    memset(&rec, 0, sizeof(rec));
    rec.op = OP_WITHDRAW;
    strncpy(rec.account, acc_num, 63);
    rec.account[63] = '\0';
    rec.amount = amount;
    rec.txn_id = txn_id;
    stack_push(&sys->undo_stack, &rec);
    printf("[OK] Withdraw %.2f from [%s], balance=%.2f\n", amount, acc_num, acc->balance);
    return 1;
}
