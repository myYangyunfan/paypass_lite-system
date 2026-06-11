#include "core/ledger_system.h"
#include "core/account.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static int failures = 0;

#define TEST(name) printf("\n== TEST: %s ==\n", name)

#define VERIFY(cond, msg) \
    do { \
        if (!(cond)) { \
            printf("  [FAIL] %s\n", msg); \
            failures++; \
        } else { \
            printf("  [PASS] %s\n", msg); \
        } \
    } while(0)

int main(void) {
    printf("============================================\n");
    printf("  paypass_lite (交易轻)\n");
    printf("  Verification Test Suite\n");
    printf("============================================\n");

    TEST("Account Creation & Search");
    {
        LedgerSystem sys;
        ls_init(&sys);

        VERIFY(ls_create_account(&sys, "ACC001", "Alice", "13800001111"),
               "Create ACC001 (Alice)");
        VERIFY(!ls_create_account(&sys, "ACC001", "Bob", "13900002222"),
               "Duplicate ACC001 rejected");

        Account *a = ls_search_account(&sys, "ACC001");
        VERIFY(a != NULL, "Search ACC001 found");
        VERIFY(strcmp(a->user_name, "Alice") == 0, "Name matches");
        VERIFY(a->balance == 0.0, "Initial balance is 0");

        VERIFY(ls_search_account(&sys, "NONEXIST") == NULL,
               "Search non-existent returns NULL");

        ls_destroy(&sys);
    }

    TEST("Deposit");
    {
        LedgerSystem sys;
        ls_init(&sys);
        ls_create_account(&sys, "ACC001", "Alice", "13800001111");

        VERIFY(ls_deposit(&sys, "ACC001", 1000.00), "Deposit 1000");
        Account *a = ls_search_account(&sys, "ACC001");
        VERIFY(a && fabs(a->balance - 1000.00) < 0.001,
               "Balance = 1000 after deposit");

        VERIFY(!ls_deposit(&sys, "NONEXIST", 100), "Deposit to non-existent fails");
        VERIFY(!ls_deposit(&sys, "ACC001", -50), "Negative deposit fails");
        VERIFY(!ls_deposit(&sys, "ACC001", 0),    "Zero deposit fails");

        ls_deposit(&sys, "ACC001", 500.00);
        VERIFY(a && fabs(a->balance - 1500.00) < 0.001,
               "Balance = 1500 after second deposit");

        ls_destroy(&sys);
    }

    TEST("Withdraw");
    {
        LedgerSystem sys;
        ls_init(&sys);
        ls_create_account(&sys, "ACC001", "Alice", "13800001111");
        ls_deposit(&sys, "ACC001", 1000.00);

        VERIFY(ls_withdraw(&sys, "ACC001", 300.00), "Withdraw 300");
        Account *a = ls_search_account(&sys, "ACC001");
        VERIFY(a && fabs(a->balance - 700.00) < 0.001,
               "Balance = 700 after withdraw");

        VERIFY(!ls_withdraw(&sys, "ACC001", 800.00),
               "Overdraft rejected (700 < 800)");
        VERIFY(!ls_withdraw(&sys, "NONEXIST", 100),
               "Withdraw from non-existent fails");
        VERIFY(!ls_withdraw(&sys, "ACC001", -50), "Negative withdraw fails");

        VERIFY(ls_withdraw(&sys, "ACC001", 700.00), "Withdraw all (700)");
        VERIFY(a && a->balance == 0.0, "Balance = 0 after full withdraw");

        VERIFY(!ls_withdraw(&sys, "ACC001", 0.01),
               "Withdraw 0.01 from empty account fails");

        ls_destroy(&sys);
    }

    TEST("Transfer & Atomicity");
    {
        LedgerSystem sys;
        ls_init(&sys);
        ls_create_account(&sys, "A", "Alice", "111");
        ls_create_account(&sys, "B", "Bob",   "222");
        ls_create_account(&sys, "C", "Carol", "333");
        ls_deposit(&sys, "A", 1000.00);
        ls_deposit(&sys, "B", 500.00);

        VERIFY(ls_transfer(&sys, "A", "B", 300.00), "Transfer 300 A->B");
        Account *accA = ls_search_account(&sys, "A");
        Account *accB = ls_search_account(&sys, "B");
        VERIFY(accA && fabs(accA->balance - 700.00) < 0.001,
               "A balance = 700");
        VERIFY(accB && fabs(accB->balance - 800.00) < 0.001,
               "B balance = 800");

        VERIFY(!ls_transfer(&sys, "A", "C", 800.00),
               "Transfer 800 A->C rejected (insufficient)");
        VERIFY(accA && fabs(accA->balance - 700.00) < 0.001,
               "A balance still 700 (atomic)");
        Account *accC = ls_search_account(&sys, "C");
        VERIFY(accC && accC->balance == 0.0,
               "C balance still 0 (atomic)");

        VERIFY(!ls_transfer(&sys, "A", "GHOST", 100), "Transfer to ghost fails");
        VERIFY(!ls_transfer(&sys, "GHOST", "A", 100), "Transfer from ghost fails");
        VERIFY(!ls_transfer(&sys, "A", "A", 100),     "Self-transfer rejected");
        VERIFY(!ls_transfer(&sys, "A", "B", -50),     "Negative transfer rejected");

        ls_destroy(&sys);
    }

    TEST("Undo Operations");
    {
        LedgerSystem sys;
        ls_init(&sys);
        ls_create_account(&sys, "A", "Alice", "111");
        ls_create_account(&sys, "B", "Bob",   "222");
        ls_deposit(&sys, "A", 1000.00);
        ls_deposit(&sys, "B", 500.00);

        VERIFY(ls_undo(&sys), "Undo: deposit 500 to B");
        Account *accB = ls_search_account(&sys, "B");
        VERIFY(accB && accB->balance == 0.0,
               "B back to 0 after undo deposit");

        ls_withdraw(&sys, "A", 200.00);
        VERIFY(ls_undo(&sys), "Undo: withdraw 200 from A");
        Account *accA = ls_search_account(&sys, "A");
        VERIFY(accA && fabs(accA->balance - 1000.00) < 0.001,
               "A back to 1000 after undo withdraw");

        ls_transfer(&sys, "A", "B", 300.00);
        VERIFY(accA && fabs(accA->balance - 700.00) < 0.001,
               "A = 700 after transfer");
        VERIFY(accB && fabs(accB->balance - 300.00) < 0.001,
               "B = 300 after transfer");

        VERIFY(ls_undo(&sys), "Undo: transfer 300 A->B");
        VERIFY(accA && fabs(accA->balance - 1000.00) < 0.001,
               "A back to 1000 after undo transfer");
        VERIFY(accB && fabs(accB->balance - 0.00) < 0.001,
               "B back to 0 after undo transfer");

        ls_undo(&sys);
        ls_undo(&sys);
        VERIFY(!ls_undo(&sys), "Empty stack undo returns false");

        ls_destroy(&sys);
    }

    TEST("Ledger & Delete");
    {
        LedgerSystem sys;
        ls_init(&sys);
        ls_create_account(&sys, "ACC001", "Alice", "111");
        ls_deposit(&sys, "ACC001", 100.00);
        ls_withdraw(&sys, "ACC001", 30.00);
        ls_deposit(&sys, "ACC001", 50.00);

        printf("  --- View Ledger (manual check) ---\n");
        ls_view_ledger(&sys, "ACC001");
        printf("  --- End Ledger ---\n");

        VERIFY(ls_delete_account(&sys, "ACC001"), "Delete ACC001");
        VERIFY(ls_search_account(&sys, "ACC001") == NULL,
               "ACC001 no longer searchable after delete");
        VERIFY(!ls_delete_account(&sys, "ACC001"), "Double-delete fails");

        ls_destroy(&sys);
    }

    TEST("Multi-Account Stress");
    {
        LedgerSystem sys;
        ls_init(&sys);

        for (int i = 1; i <= 20; ++i) {
            char buf[16];
            char name[16];
            snprintf(buf, sizeof(buf), "ACC%03d", i);
            snprintf(name, sizeof(name), "User%d", i);
            VERIFY(ls_create_account(&sys, buf, name, "13800000000"), buf);
        }

        for (int i = 1; i <= 20; ++i) {
            char buf[16];
            snprintf(buf, sizeof(buf), "ACC%03d", i);
            VERIFY(ls_deposit(&sys, buf, 1000.00), buf);
        }

        for (int i = 1; i < 20; ++i) {
            char from[16], to[16];
            snprintf(from, sizeof(from), "ACC%03d", i);
            snprintf(to,   sizeof(to),   "ACC%03d", i + 1);
            char msg[64];
            snprintf(msg, sizeof(msg), "Transfer 100 %s->%s", from, to);
            VERIFY(ls_transfer(&sys, from, to, 100.00), msg);
        }

        for (int i = 0; i < 5; ++i) {
            char msg[32];
            snprintf(msg, sizeof(msg), "Undo #%d", i + 1);
            VERIFY(ls_undo(&sys), msg);
        }

        printf("  Stress test completed without crashes.\n");

        ls_destroy(&sys);
    }

    TEST("Edge Cases");
    {
        LedgerSystem sys;
        ls_init(&sys);

        ls_create_account(&sys, "A", "Test", "111");
        ls_deposit(&sys, "A", 999999.99);
        ls_withdraw(&sys, "A", 0.01);
        Account *a = ls_search_account(&sys, "A");
        VERIFY(a && fabs(a->balance - 999999.98) < 0.001,
               "Floating-point precision OK");

        ls_create_account(&sys, "B", "Big", "222");
        ls_deposit(&sys, "B", 99999999.00);
        Account *b = ls_search_account(&sys, "B");
        VERIFY(b && fabs(b->balance - 99999999.00) < 0.001,
               "Large amount deposit OK");

        ls_create_account(&sys, "C", "Multi", "333");
        ls_deposit(&sys, "C", 100.00);
        ls_deposit(&sys, "C", 200.00);
        ls_deposit(&sys, "C", 300.00);
        ls_undo(&sys);
        ls_undo(&sys);
        ls_undo(&sys);
        Account *c = ls_search_account(&sys, "C");
        VERIFY(c && c->balance == 0.0,
               "Multi-undo restores balance to 0");

        ls_create_account(&sys, "D", "TempD", "444");
        ls_create_account(&sys, "E", "TempE", "555");
        ls_deposit(&sys, "D", 500.00);
        ls_transfer(&sys, "D", "E", 200.00);
        ls_delete_account(&sys, "D");
        VERIFY(!ls_undo(&sys), "Undo transfer after source deleted handled");

        ls_destroy(&sys);
    }

    TEST("Hash Table Collision Survivability");
    {
        LedgerSystem sys;
        ls_init(&sys);

        for (int i = 1; i <= 50; ++i) {
            char buf[16];
            snprintf(buf, sizeof(buf), "HK%03d", i);
            VERIFY(ls_create_account(&sys, buf, "HashUser", "10086"), buf);
        }

        for (int i = 1; i <= 50; ++i) {
            char buf[16];
            snprintf(buf, sizeof(buf), "HK%03d", i);
            Account *a = ls_search_account(&sys, buf);
            VERIFY(a != NULL, buf);
        }

        for (int i = 1; i <= 10; ++i) {
            char buf[16];
            snprintf(buf, sizeof(buf), "HK%03d", i);
            VERIFY(ls_delete_account(&sys, buf), buf);
        }

        for (int i = 1; i <= 10; ++i) {
            char buf[16];
            snprintf(buf, sizeof(buf), "HK%03d", i);
            VERIFY(ls_search_account(&sys, buf) == NULL, buf);
        }

        ls_destroy(&sys);
    }

    TEST("Ledger Integrity After Undo");
    {
        LedgerSystem sys;
        ls_init(&sys);
        ls_create_account(&sys, "LEDGER1", "Tester", "111");
        ls_deposit(&sys, "LEDGER1", 500.00);
        ls_withdraw(&sys, "LEDGER1", 100.00);
        ls_deposit(&sys, "LEDGER1", 200.00);

        printf("  --- Ledger BEFORE undo ---\n");
        ls_view_ledger(&sys, "LEDGER1");

        ls_undo(&sys);

        printf("  --- Ledger AFTER undo ---\n");
        ls_view_ledger(&sys, "LEDGER1");

        Account *a = ls_search_account(&sys, "LEDGER1");
        VERIFY(a && fabs(a->balance - 400.00) < 0.001,
               "Balance = 400 after undo (500 - 100)");

        ls_destroy(&sys);
    }

    printf("\n============================================\n");
    if (failures == 0) {
        printf("  ALL TESTS PASSED\n");
    } else {
        printf("  %d TEST(S) FAILED\n", failures);
    }
    printf("============================================\n");

    return failures;
}
