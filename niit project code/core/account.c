#include "account.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Account* account_create(const char *acc_num, const char *name, const char *phone) {
    Account *acc = (Account*)malloc(sizeof(Account));
    if (!acc) return NULL;
    strncpy(acc->account_number, acc_num, 63);
    acc->account_number[63] = '\0';
    strncpy(acc->user_name, name, 127);
    acc->user_name[127] = '\0';
    strncpy(acc->phone_number, phone, 31);
    acc->phone_number[31] = '\0';
    acc->balance = 0.0;
    dll_init(&acc->ledger);
    return acc;
}

void account_destroy(Account *acc) {
    if (!acc) return;
    dll_destroy(&acc->ledger);
    free(acc);
}

int account_deposit(Account *acc, int txn_id, double amount, const char *timestamp) {
    if (amount <= 0) return 0;
    Transaction *txn = txn_create(txn_id, "", acc->account_number, amount, TXN_DEPOSIT, timestamp);
    if (!txn) return 0;
    acc->balance += amount;
    dll_push_back(&acc->ledger, txn);
    return 1;
}

int account_withdraw(Account *acc, int txn_id, double amount, const char *timestamp) {
    if (amount <= 0) return 0;
    if (!account_can_withdraw(acc, amount)) return 0;
    Transaction *txn = txn_create(txn_id, acc->account_number, "", amount, TXN_WITHDRAW, timestamp);
    if (!txn) return 0;
    acc->balance -= amount;
    dll_push_back(&acc->ledger, txn);
    return 1;
}

int account_receive_transfer(Account *acc, int txn_id, double amount, const char *from, const char *timestamp) {
    if (amount <= 0) return 0;
    Transaction *txn = txn_create(txn_id, from, acc->account_number, amount, TXN_TRANSFER, timestamp);
    if (!txn) return 0;
    acc->balance += amount;
    dll_push_back(&acc->ledger, txn);
    return 1;
}

int account_send_transfer(Account *acc, int txn_id, double amount, const char *to, const char *timestamp) {
    if (amount <= 0) return 0;
    if (!account_can_withdraw(acc, amount)) return 0;
    Transaction *txn = txn_create(txn_id, acc->account_number, to, amount, TXN_TRANSFER, timestamp);
    if (!txn) return 0;
    acc->balance -= amount;
    dll_push_back(&acc->ledger, txn);
    return 1;
}

Transaction* account_last_transaction(Account *acc) {
    return dll_last(&acc->ledger);
}

int account_remove_transaction(Account *acc, int txn_id) {
    return dll_remove_by_id(&acc->ledger, txn_id);
}

int account_can_withdraw(Account *acc, double amount) {
    return acc->balance >= amount;
}

void account_force_adjust(Account *acc, double delta) {
    acc->balance += delta;
}

void account_print_ledger(Account *acc) {
    printf("\n=== Ledger for Account [%s] %s ===\n", acc->account_number, acc->user_name);
    printf("  Current Balance: %.2f\n", acc->balance);
    dll_print_all(&acc->ledger);
}
