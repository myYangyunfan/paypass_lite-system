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

int ls_transfer(LedgerSystem *sys, const char *from, const char *to, double amount) {
    if (amount <= 0) {
        printf("[ERROR] Transfer amount must be positive.\n");
        return 0;
    }
    if (strcmp(from, to) == 0) {
        printf("[ERROR] Cannot transfer to the same account.\n");
        return 0;
    }
    Account *from_acc = ht_search(&sys->accounts, from);
    if (!from_acc) {
        printf("[ERROR] Source account [%s] not found.\n", from);
        return 0;
    }
    Account *to_acc = ht_search(&sys->accounts, to);
    if (!to_acc) {
        printf("[ERROR] Destination account [%s] not found.\n", to);
        return 0;
    }
    if (!account_can_withdraw(from_acc, amount)) {
        printf("[ERROR] Insufficient balance in [%s]. Balance=%.2f, requested=%.2f\n",
               from, from_acc->balance, amount);
        return 0;
    }
    int txn_id = ls_next_txn_id(sys);
    char ts[32];
    ls_now(ts, sizeof(ts));
    account_send_transfer(from_acc, txn_id, amount, to, ts);
    account_receive_transfer(to_acc, txn_id, amount, from, ts);
    UndoRecord rec;
    memset(&rec, 0, sizeof(rec));
    rec.op = OP_TRANSFER;
    strncpy(rec.from_account, from, 63);
    rec.from_account[63] = '\0';
    strncpy(rec.to_account, to, 63);
    rec.to_account[63] = '\0';
    rec.amount = amount;
    rec.txn_id = txn_id;
    stack_push(&sys->undo_stack, &rec);
    printf("[OK] Transfer %.2f from [%s] to [%s]\n", amount, from, to);
    return 1;
}

int ls_undo(LedgerSystem *sys) {
    if (stack_empty(&sys->undo_stack)) {
        printf("[INFO] Nothing to undo.\n");
        return 0;
    }
    UndoRecord rec;
    stack_pop(&sys->undo_stack, &rec);
    switch (rec.op) {
        case OP_DEPOSIT: {
            Account *acc = ht_search(&sys->accounts, rec.account);
            if (!acc) {
                printf("[ERROR] Undo: account [%s] no longer exists.\n", rec.account);
                return 0;
            }
            account_force_adjust(acc, -rec.amount);
            account_remove_transaction(acc, rec.txn_id);
            printf("[UNDO] Deposit of %.2f to [%s] reversed. Balance=%.2f\n",
                   rec.amount, rec.account, acc->balance);
            return 1;
        }
        case OP_WITHDRAW: {
            Account *acc = ht_search(&sys->accounts, rec.account);
            if (!acc) {
                printf("[ERROR] Undo: account [%s] no longer exists.\n", rec.account);
                return 0;
            }
            account_force_adjust(acc, rec.amount);
            account_remove_transaction(acc, rec.txn_id);
            printf("[UNDO] Withdrawal of %.2f from [%s] reversed. Balance=%.2f\n",
                   rec.amount, rec.account, acc->balance);
            return 1;
        }
        case OP_TRANSFER: {
            Account *from_acc = ht_search(&sys->accounts, rec.from_account);
            Account *to_acc = ht_search(&sys->accounts, rec.to_account);
            if (!from_acc || !to_acc) {
                printf("[ERROR] Undo transfer: one or both accounts gone.\n");
                return 0;
            }
            account_force_adjust(from_acc, rec.amount);
            account_force_adjust(to_acc, -rec.amount);
            account_remove_transaction(from_acc, rec.txn_id);
            account_remove_transaction(to_acc, rec.txn_id);
            printf("[UNDO] Transfer of %.2f [%s -> %s] reversed.\n",
                   rec.amount, rec.from_account, rec.to_account);
            return 1;
        }
        default:
            printf("[ERROR] Unknown undo operation type.\n");
            return 0;
    }
}

void ls_view_ledger(LedgerSystem *sys, const char *acc_num) {
    Account *acc = ht_search(&sys->accounts, acc_num);
    if (!acc) {
        printf("[ERROR] Account [%s] not found.\n", acc_num);
        return;
    }
    account_print_ledger(acc);
}
