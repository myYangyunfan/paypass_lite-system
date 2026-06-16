#ifndef TRANSACTION_H
#define TRANSACTION_H

#define TXN_DEPOSIT  0
#define TXN_WITHDRAW 1
#define TXN_TRANSFER 2

typedef struct Transaction {
    int txn_id;
    char from_account[64];
    char to_account[64];
    double amount;
    int type;
    char timestamp[32];
    struct Transaction *prev;
    struct Transaction *next;
} Transaction;

typedef struct {
    Transaction *head;
    Transaction *tail;
    int count;
} DoublyLinkedList;

const char* txn_type_str(int type);
Transaction* txn_create(int id, const char *from, const char *to, double amount, int type, const char *ts);
void dll_init(DoublyLinkedList *list);
void dll_destroy(DoublyLinkedList *list);
void dll_push_back(DoublyLinkedList *list, Transaction *txn);
int dll_remove_by_id(DoublyLinkedList *list, int txn_id);
Transaction* dll_last(const DoublyLinkedList *list);
int dll_empty(const DoublyLinkedList *list);
void dll_clear(DoublyLinkedList *list);
void dll_print_all(DoublyLinkedList *list);

#endif
