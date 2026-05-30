#include "core/ledger_system.h"
#include "core/account.h"
#include <iostream>
#include <cassert>
#include <cmath>

// 断言辅助宏
#define TEST(name) std::cout << "\n== TEST: " << name << " ==\n"
#define VERIFY(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "  [FAIL] " << msg << "\n"; \
            failures++; \
        } else { \
            std::cout << "  [PASS] " << msg << "\n"; \
        } \
    } while(0)

int main() {
    int failures = 0;
    std::cout << "============================================\n";
    std::cout << "  paypass_lite (交易轻)\n";
    std::cout << "  Verification Test Suite\n";
    std::cout << "============================================\n";

    // ===============================================
    // 1. 账户创建与查询
    // ===============================================
    TEST("Account Creation & Search");
    {
        LedgerSystem sys;

        VERIFY(sys.createAccount("ACC001", "Alice", "13800001111"),
               "Create ACC001 (Alice)");
        VERIFY(!sys.createAccount("ACC001", "Bob", "13900002222"),
               "Duplicate ACC001 rejected");

        Account* a = sys.searchAccount("ACC001");
        VERIFY(a != nullptr, "Search ACC001 found");
        VERIFY(a->getUserName() == "Alice", "Name matches");
        VERIFY(a->getBalance() == 0.0, "Initial balance is 0");

        VERIFY(sys.searchAccount("NONEXIST") == nullptr,
               "Search non-existent returns nullptr");
    }
    // LedgerSystem 析构 — 释放所有 Account 和 Transaction

    // ===============================================
    // 2. 存款测试
    // ===============================================
    TEST("Deposit");
    {
        LedgerSystem sys;
        sys.createAccount("ACC001", "Alice", "13800001111");

        VERIFY(sys.deposit("ACC001", 1000.00), "Deposit 1000");
        Account* a = sys.searchAccount("ACC001");
        VERIFY(a && std::abs(a->getBalance() - 1000.00) < 0.001,
               "Balance = 1000 after deposit");

        VERIFY(!sys.deposit("NONEXIST", 100), "Deposit to non-existent fails");
        VERIFY(!sys.deposit("ACC001", -50), "Negative deposit fails");
        VERIFY(!sys.deposit("ACC001", 0),    "Zero deposit fails");

        sys.deposit("ACC001", 500.00);
        VERIFY(a && std::abs(a->getBalance() - 1500.00) < 0.001,
               "Balance = 1500 after second deposit");
    }

    // ===============================================
    // 3. 取款测试
    // ===============================================
    TEST("Withdraw");
    {
        LedgerSystem sys;
        sys.createAccount("ACC001", "Alice", "13800001111");
        sys.deposit("ACC001", 1000.00);

        VERIFY(sys.withdraw("ACC001", 300.00), "Withdraw 300");
        Account* a = sys.searchAccount("ACC001");
        VERIFY(a && std::abs(a->getBalance() - 700.00) < 0.001,
               "Balance = 700 after withdraw");

        VERIFY(!sys.withdraw("ACC001", 800.00),
               "Overdraft rejected (700 < 800)");
        VERIFY(!sys.withdraw("NONEXIST", 100),
               "Withdraw from non-existent fails");
        VERIFY(!sys.withdraw("ACC001", -50), "Negative withdraw fails");

        VERIFY(sys.withdraw("ACC001", 700.00), "Withdraw all (700)");
        VERIFY(a && a->getBalance() == 0.0, "Balance = 0 after full withdraw");

        VERIFY(!sys.withdraw("ACC001", 0.01),
               "Withdraw 0.01 from empty account fails");
    }

    // ===============================================
    // 4. 转账测试（含原子性校验）
    // ===============================================
    TEST("Transfer & Atomicity");
    {
        LedgerSystem sys;
        sys.createAccount("A", "Alice", "111");
        sys.createAccount("B", "Bob",   "222");
        sys.createAccount("C", "Carol", "333");
        sys.deposit("A", 1000.00);
        sys.deposit("B", 500.00);

        // 正常转账
        VERIFY(sys.transfer("A", "B", 300.00), "Transfer 300 A->B");
        Account* accA = sys.searchAccount("A");
        Account* accB = sys.searchAccount("B");
        VERIFY(accA && std::abs(accA->getBalance() - 700.00) < 0.001,
               "A balance = 700");
        VERIFY(accB && std::abs(accB->getBalance() - 800.00) < 0.001,
               "B balance = 800");

        // 原子性：余额不足 → 双方余额不变
        VERIFY(!sys.transfer("A", "C", 800.00),
               "Transfer 800 A->C rejected (insufficient)");
        VERIFY(accA && std::abs(accA->getBalance() - 700.00) < 0.001,
               "A balance still 700 (atomic)");
        Account* accC = sys.searchAccount("C");
        VERIFY(accC && accC->getBalance() == 0.0,
               "C balance still 0 (atomic)");

        // 不存在的账户
        VERIFY(!sys.transfer("A", "GHOST", 100), "Transfer to ghost fails");
        VERIFY(!sys.transfer("GHOST", "A", 100), "Transfer from ghost fails");
        VERIFY(!sys.transfer("A", "A", 100),     "Self-transfer rejected");
        VERIFY(!sys.transfer("A", "B", -50),     "Negative transfer rejected");
    }

    // ===============================================
    // 5. 撤销测试
    // ===============================================
    TEST("Undo Operations");
    {
        LedgerSystem sys;
        sys.createAccount("A", "Alice", "111");
        sys.createAccount("B", "Bob",   "222");
        sys.deposit("A", 1000.00);
        sys.deposit("B", 500.00);

        // 5a. 撤销存款
        VERIFY(sys.undo(), "Undo: deposit 500 to B");
        Account* accB = sys.searchAccount("B");
        VERIFY(accB && accB->getBalance() == 0.0,
               "B back to 0 after undo deposit");

        // 5b. 取款后撤销
        sys.withdraw("A", 200.00);
        VERIFY(sys.undo(), "Undo: withdraw 200 from A");
        Account* accA = sys.searchAccount("A");
        VERIFY(accA && std::abs(accA->getBalance() - 1000.00) < 0.001,
               "A back to 1000 after undo withdraw");

        // 5c. 转账后撤销
        sys.transfer("A", "B", 300.00);
        VERIFY(accA && std::abs(accA->getBalance() - 700.00) < 0.001,
               "A = 700 after transfer");
        VERIFY(accB && std::abs(accB->getBalance() - 300.00) < 0.001,
               "B = 300 after transfer");

        VERIFY(sys.undo(), "Undo: transfer 300 A->B");
        VERIFY(accA && std::abs(accA->getBalance() - 1000.00) < 0.001,
               "A back to 1000 after undo transfer");
        VERIFY(accB && std::abs(accB->getBalance() - 0.00) < 0.001,
               "B back to 0 after undo transfer");

        // 5d. 空栈撤销
        // 继续撤销直到栈空
        sys.undo(); // undo deposit 1000 to A
        sys.undo(); // undo deposit 500 to B (already partially undone)
        VERIFY(!sys.undo(), "Empty stack undo returns false");
    }

    // ===============================================
    // 6. 账本查看与删除
    // ===============================================
    TEST("Ledger & Delete");
    {
        LedgerSystem sys;
        sys.createAccount("ACC001", "Alice", "111");
        sys.deposit("ACC001", 100.00);
        sys.withdraw("ACC001", 30.00);
        sys.deposit("ACC001", 50.00);

        std::cout << "  --- View Ledger (manual check) ---\n";
        sys.viewLedger("ACC001");
        std::cout << "  --- End Ledger ---\n";

        VERIFY(sys.deleteAccount("ACC001"), "Delete ACC001");
        VERIFY(sys.searchAccount("ACC001") == nullptr,
               "ACC001 no longer searchable after delete");
        VERIFY(!sys.deleteAccount("ACC001"), "Double-delete fails");
    }

    // ===============================================
    // 7. 多账户 + 连续操作压力测试
    // ===============================================
    TEST("Multi-Account Stress");
    {
        LedgerSystem sys;

        // 创建 20 个账户
        for (int i = 1; i <= 20; ++i) {
            char buf[16];
            std::snprintf(buf, sizeof(buf), "ACC%03d", i);
            std::string name = "User" + std::to_string(i);
            VERIFY(sys.createAccount(buf, name, "13800000000"),
                   ("Create " + std::string(buf)).c_str());
        }

        // 给每个存 1000
        for (int i = 1; i <= 20; ++i) {
            char buf[16];
            std::snprintf(buf, sizeof(buf), "ACC%03d", i);
            VERIFY(sys.deposit(buf, 1000.00),
                   ("Deposit 1000 to " + std::string(buf)).c_str());
        }

        // 链式转账: ACC001 -> ACC002 -> ... -> ACC020
        for (int i = 1; i < 20; ++i) {
            char from[16], to[16];
            std::snprintf(from, sizeof(from), "ACC%03d", i);
            std::snprintf(to,   sizeof(to),   "ACC%03d", i + 1);
            VERIFY(sys.transfer(from, to, 100.00),
                   ("Transfer 100 from " + std::string(from) + " to " + std::string(to)).c_str());
        }

        // 连续撤销 5 次
        for (int i = 0; i < 5; ++i) {
            VERIFY(sys.undo(), ("Undo #" + std::to_string(i + 1)).c_str());
        }

        std::cout << "  Stress test completed without crashes.\n";
    }

    // ===============================================
    // 8. 边界测试
    // ===============================================
    TEST("Edge Cases");
    {
        LedgerSystem sys;

        // 浮点精度
        sys.createAccount("A", "Test", "111");
        sys.deposit("A", 999999.99);
        sys.withdraw("A", 0.01);
        Account* a = sys.searchAccount("A");
        VERIFY(a && std::abs(a->getBalance() - 999999.98) < 0.001,
               "Floating-point precision OK");

        // 大额整数
        sys.createAccount("B", "Big", "222");
        sys.deposit("B", 99999999.00);
        Account* b = sys.searchAccount("B");
        VERIFY(b && std::abs(b->getBalance() - 99999999.00) < 0.001,
               "Large amount deposit OK");

        // 连续多次撤销
        sys.createAccount("C", "Multi", "333");
        sys.deposit("C", 100.00);
        sys.deposit("C", 200.00);
        sys.deposit("C", 300.00);
        sys.undo(); // undo 300
        sys.undo(); // undo 200
        sys.undo(); // undo 100
        Account* c = sys.searchAccount("C");
        VERIFY(c && c->getBalance() == 0.0,
               "Multi-undo restores balance to 0");

        // 转账撤销后账户消失的情况（不崩溃）
        sys.createAccount("D", "TempD", "444");
        sys.createAccount("E", "TempE", "555");
        sys.deposit("D", 500.00);
        sys.transfer("D", "E", 200.00);
        sys.deleteAccount("D"); // D 先被删除
        // 此时 undo 会尝试撤回 transfer，但 D 已不存在 → 安全拦截
        VERIFY(!sys.undo(), "Undo transfer after source deleted handled");
    }

    // ===============================================
    // 9. 哈希表冲突测试
    // ===============================================
    TEST("Hash Table Collision Survivability");
    {
        LedgerSystem sys;

        // 创建大量账户以触发链地址法冲突，验证不会丢失或覆盖
        for (int i = 1; i <= 50; ++i) {
            char buf[16];
            std::snprintf(buf, sizeof(buf), "HK%03d", i);
            VERIFY(sys.createAccount(buf, "HashUser", "10086"),
                   ("Create hash key #" + std::to_string(i)).c_str());
        }

        // 全部能查到
        for (int i = 1; i <= 50; ++i) {
            char buf[16];
            std::snprintf(buf, sizeof(buf), "HK%03d", i);
            Account* a = sys.searchAccount(buf);
            VERIFY(a != nullptr, ("Search " + std::string(buf)).c_str());
        }

        // 删除部分
        for (int i = 1; i <= 10; ++i) {
            char buf[16];
            std::snprintf(buf, sizeof(buf), "HK%03d", i);
            VERIFY(sys.deleteAccount(buf), ("Delete " + std::string(buf)).c_str());
        }

        // 删除后不可查询
        for (int i = 1; i <= 10; ++i) {
            char buf[16];
            std::snprintf(buf, sizeof(buf), "HK%03d", i);
            VERIFY(sys.searchAccount(buf) == nullptr,
                   ("Deleted " + std::string(buf) + " not found").c_str());
        }
    }

    // ===============================================
    // 10. 账本在撤销后的完整性
    // ===============================================
    TEST("Ledger Integrity After Undo");
    {
        LedgerSystem sys;
        sys.createAccount("LEDGER1", "Tester", "111");
        sys.deposit("LEDGER1", 500.00);
        sys.withdraw("LEDGER1", 100.00);
        sys.deposit("LEDGER1", 200.00);

        std::cout << "  --- Ledger BEFORE undo ---\n";
        sys.viewLedger("LEDGER1");

        sys.undo(); // undo last deposit (200)

        std::cout << "  --- Ledger AFTER undo ---\n";
        sys.viewLedger("LEDGER1");

        Account* a = sys.searchAccount("LEDGER1");
        VERIFY(a && std::abs(a->getBalance() - 400.00) < 0.001,
               "Balance = 400 after undo (500 - 100)");
    }

    // ===============================================
    std::cout << "\n============================================\n";
    if (failures == 0) {
        std::cout << "  ALL TESTS PASSED\n";
    } else {
        std::cout << "  " << failures << " TEST(S) FAILED\n";
    }
    std::cout << "============================================\n";

    // 返回 0 表示全部通过，非 0 表示有失败
    return failures;
}
