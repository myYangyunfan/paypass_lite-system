#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "transaction.h"

typedef struct {
    char account_number[64];
    char user_name[128];
    char phone_number[32];
    double balance;
    DoublyLinkedList ledger;
} Account;

Account* account_create(const char *acc_num, const char *name, const char *phone);
void account_destroy(Account *acc);
int account_deposit(Account *acc, int txn_id, double amount, const char *timestamp);
int account_withdraw(Account *acc, int txn_id, double amount, const char *timestamp);
int account_receive_transfer(Account *acc, int txn_id, double amount, const char *from, const char *timestamp);
int account_send_transfer(Account *acc, int txn_id, double amount, const char *to, const char *timestamp);
Transaction* account_last_transaction(Account *acc);
int account_remove_transaction(Account *acc, int txn_id);
int account_can_withdraw(Account *acc, double amount);
void account_force_adjust(Account *acc, double delta);
void account_print_ledger(Account *acc);

#endif
