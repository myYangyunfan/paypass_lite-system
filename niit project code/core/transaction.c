#include "transaction.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* txn_type_str(int type) {
    switch (type) {
        case TXN_DEPOSIT:  return "DEPOSIT";
        case TXN_WITHDRAW: return "WITHDRAW";
        case TXN_TRANSFER: return "TRANSFER";
        default:           return "UNKNOWN";
    }
}

Transaction* txn_create(int id, const char *from, const char *to, double amount, int type, const char *ts) {
    Transaction *txn = (Transaction*)malloc(sizeof(Transaction));
    if (!txn) return NULL;
    txn->txn_id = id;
    strncpy(txn->from_account, from, 63);
    txn->from_account[63] = '\0';
    strncpy(txn->to_account, to, 63);
    txn->to_account[63] = '\0';
    txn->amount = amount;
    txn->type = type;
    strncpy(txn->timestamp, ts, 31);
    txn->timestamp[31] = '\0';
    txn->prev = NULL;
    txn->next = NULL;
    return txn;
}

void dll_init(DoublyLinkedList *list) {
    list->head = txn_create(-1, "", "", 0.0, TXN_DEPOSIT, "");
    list->tail = txn_create(-1, "", "", 0.0, TXN_DEPOSIT, "");
    list->head->next = list->tail;
    list->tail->prev = list->head;
    list->count = 0;
}

void dll_destroy(DoublyLinkedList *list) {
    dll_clear(list);
    free(list->head);
    free(list->tail);
}

void dll_push_back(DoublyLinkedList *list, Transaction *txn) {
    if (!txn) return;
    Transaction *before = list->tail->prev;
    txn->prev = before;
    txn->next = list->tail;
    before->next = txn;
    list->tail->prev = txn;
    list->count++;
}

int dll_remove_by_id(DoublyLinkedList *list, int txn_id) {
    Transaction *cur = list->head->next;
    while (cur != list->tail) {
        if (cur->txn_id == txn_id) {
            cur->prev->next = cur->next;
            cur->next->prev = cur->prev;
            free(cur);
            list->count--;
            return 1;
        }
        cur = cur->next;
    }
    return 0;
}

Transaction* dll_last(const DoublyLinkedList *list) {
    if (dll_empty(list)) return NULL;
    return list->tail->prev;
}

int dll_empty(const DoublyLinkedList *list) {
    return list->count == 0;
}

void dll_clear(DoublyLinkedList *list) {
    Transaction *cur = list->head->next;
    while (cur != list->tail) {
        Transaction *nxt = cur->next;
        free(cur);
        cur = nxt;
    }
    list->head->next = list->tail;
    list->tail->prev = list->head;
    list->count = 0;
}

void dll_print_all(DoublyLinkedList *list) {
    if (dll_empty(list)) {
        printf("  (No transactions)\n");
        return;
    }
    printf("  %4s  %10s  %12s  %15s  %15s  %s\n",
           "ID", "Type", "Amount", "From", "To", "Time");
    printf("  ");
    for (int i = 0; i < 80; i++) printf("-");
    printf("\n");
    Transaction *cur = list->head->next;
    while (cur != list->tail) {
        printf("  %4d  %10s  %12.2f  %15s  %15s  %s\n",
               cur->txn_id,
               txn_type_str(cur->type),
               cur->amount,
               cur->from_account[0] ? cur->from_account : "-",
               cur->to_account[0] ? cur->to_account : "-",
               cur->timestamp);
        cur = cur->next;
    }
}
