#include "ledger_system.h"
#include "account.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <limits>
#include <cstdlib>
#include <sstream>

namespace {
    bool g_english = true;

    std::string _(const char* en, const char* zh) {
        return g_english ? std::string(en) : std::string(zh);
    }

    void toggleLanguage() {
        g_english = !g_english;
    }
}

void clearScreen() {
#ifdef _WIN32
    std::system("cls");
#else
    std::system("clear");
#endif
}

void pause() {
    std::cout << _("\n  Press Enter to continue...", "\n  按回车键继续...");
    std::string dummy;
    std::getline(std::cin, dummy);
}

std::string readLine(const std::string& prompt) {
    std::string input;
    while (true) {
        std::cout << prompt;
        std::getline(std::cin, input);
        if (input.empty()) {
            std::cout << _("  [ERROR] Input cannot be empty.\n", "  [ERROR] 输入不能为空。\n");
            continue;
        }
        size_t start = input.find_first_not_of(" \t");
        size_t end   = input.find_last_not_of(" \t");
        if (start == std::string::npos) {
            std::cout << _("  [ERROR] Input cannot be just whitespace.\n", "  [ERROR] 输入不能仅为空格。\n");
            continue;
        }
        return input.substr(start, end - start + 1);
    }
}

double readAmount(const std::string& prompt) {
    double value;
    while (true) {
        std::cout << prompt;
        if (!(std::cin >> value)) {
            std::cout << _("  [ERROR] Please enter a valid number.\n", "  [ERROR] 请输入有效数字。\n");
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (value <= 0) {
            std::cout << _("  [ERROR] Amount must be positive (> 0).\n", "  [ERROR] 金额必须为正数 (> 0)。\n");
            continue;
        }
        if (value > 999999999.99) {
            std::cout << _("  [ERROR] Amount exceeds limit (999,999,999.99).\n", "  [ERROR] 金额超出限制 (999,999,999.99)。\n");
            continue;
        }
        return value;
    }
}

int readChoice() {
    int choice;
    while (true) {
        std::cout << _("\n  Enter your choice [0-9]: ", "\n  请输入选项 [0-9]: ");
        if (!(std::cin >> choice)) {
            std::cout << _("  [ERROR] Please enter a number.\n", "  [ERROR] 请输入数字。\n");
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (choice < 0 || choice > 9) {
            std::cout << _("  [ERROR] Please enter 0-9.\n", "  [ERROR] 请输入 0-9。\n");
            continue;
        }
        return choice;
    }
}

bool confirm(const std::string& prompt) {
    while (true) {
        std::cout << prompt << " (y/n): ";
        std::string input;
        std::getline(std::cin, input);
        if (input == "y" || input == "Y") return true;
        if (input == "n" || input == "N") return false;
        std::cout << _("  Please enter 'y' or 'n'.\n", "  请输入 'y' 或 'n'。\n");
    }
}

void displayMenu() {
    std::cout << "\n";
    std::cout << "  +===================================================+\n";
    if (g_english) {
        std::cout << "  |              paypass_lite (交易轻)                 |\n";
    } else {
        std::cout << "  |              paypass_lite (交易轻)                 |\n";
    }
    std::cout << "  +===================================================+\n";
    std::cout << "  |                                                   |\n";
    if (g_english) {
        std::cout << "  |  [0] Toggle Language (中文/English)              |\n";
        std::cout << "  |  [1] Create Account                              |\n";
        std::cout << "  |  [2] Deposit Money                               |\n";
        std::cout << "  |  [3] Withdraw Money                              |\n";
        std::cout << "  |  [4] Transfer Money                              |\n";
        std::cout << "  |  [5] Search Account                              |\n";
        std::cout << "  |  [6] View Ledger                                 |\n";
        std::cout << "  |  [7] Undo Transaction                            |\n";
        std::cout << "  |  [8] Delete Account                              |\n";
        std::cout << "  |  [9] Exit                                        |\n";
    } else {
        std::cout << "  |  [0] 切换语言 (English/中文)                     |\n";
        std::cout << "  |  [1] 创建账户                                    |\n";
        std::cout << "  |  [2] 存款                                        |\n";
        std::cout << "  |  [3] 取款                                        |\n";
        std::cout << "  |  [4] 转账                                        |\n";
        std::cout << "  |  [5] 查询账户                                    |\n";
        std::cout << "  |  [6] 查看账本                                    |\n";
        std::cout << "  |  [7] 撤销交易                                    |\n";
        std::cout << "  |  [8] 删除账户                                    |\n";
        std::cout << "  |  [9] 退出                                        |\n";
    }
    std::cout << "  |                                                   |\n";
    std::cout << "  +===================================================+\n";
}

void handleCreateAccount(LedgerSystem& sys) {
    clearScreen();
    std::cout << "\n  ═══ " << _("Create Account", "创建账户") << " ═══\n\n";

    std::string accNum = readLine(_("  Account Number: ", "  账号："));
    std::string name   = readLine(_("  User Name:      ", "  用户名："));
    std::string phone  = readLine(_("  Phone Number:   ", "  电话："));

    sys.createAccount(accNum, name, phone);
    pause();
}

void handleDeposit(LedgerSystem& sys) {
    clearScreen();
    std::cout << "\n  ═══ " << _("Deposit Money", "存款") << " ═══\n\n";

    std::string accNum = readLine(_("  Account Number: ", "  账号："));
    double amount = readAmount(_("  Amount:         ", "  金额："));

    sys.deposit(accNum, amount);
    pause();
}

void handleWithdraw(LedgerSystem& sys) {
    clearScreen();
    std::cout << "\n  ═══ " << _("Withdraw Money", "取款") << " ═══\n\n";

    std::string accNum = readLine(_("  Account Number: ", "  账号："));
    double amount = readAmount(_("  Amount:         ", "  金额："));

    sys.withdraw(accNum, amount);
    pause();
}

void handleTransfer(LedgerSystem& sys) {
    clearScreen();
    std::cout << "\n  ═══ " << _("Transfer Money", "转账") << " ═══\n\n";

    std::string from = readLine(_("  From Account: ", "  来源账号："));
    std::string to   = readLine(_("  To Account:   ", "  目标账号："));
    double amount    = readAmount(_("  Amount:       ", "  金额："));

    sys.transfer(from, to, amount);
    pause();
}

void handleSearchAccount(LedgerSystem& sys) {
    clearScreen();
    std::cout << "\n  ═══ " << _("Search Account", "查询账户") << " ═══\n\n";

    std::string accNum = readLine(_("  Account Number: ", "  账号："));

    Account* acc = sys.searchAccount(accNum);
    if (!acc) {
        std::cout << _("\n  [ERROR] Account [", "\n  [ERROR] 账号[")
                  << accNum << _("] not found.\n", "] 未找到。\n");
    } else {
        if (g_english) {
            std::cout << "\n  ┌─────────────────────────────────┐\n";
            std::cout << "  │  Account Details                │\n";
            std::cout << "  ├─────────────────────────────────┤\n";
            std::cout << "  │  Account No: "
                      << std::left << std::setw(20) << acc->getAccountNumber() << "│\n";
            std::cout << "  │  User Name:  "
                      << std::left << std::setw(20) << acc->getUserName()     << "│\n";
            std::cout << "  │  Phone:      "
                      << std::left << std::setw(20) << acc->getPhoneNumber()  << "│\n";
            std::ostringstream bal_fmt;
            bal_fmt << std::fixed << std::setprecision(2) << acc->getBalance();
            std::cout << "  │  Balance:    "
                      << std::left << std::setw(20) << bal_fmt.str() << "│\n";
            std::cout << "  └─────────────────────────────────┘\n";
        } else {
            std::cout << "\n  ┌─────────────────────────────────┐\n";
            std::cout << "  │  账户详情                        │\n";
            std::cout << "  ├─────────────────────────────────┤\n";
            std::cout << "  │  账号："
                      << std::left << std::setw(22) << acc->getAccountNumber() << "│\n";
            std::cout << "  │  用户名："
                      << std::left << std::setw(20) << acc->getUserName()     << "│\n";
            std::cout << "  │  电话："
                      << std::left << std::setw(22) << acc->getPhoneNumber()  << "│\n";
            std::ostringstream bal_fmt;
            bal_fmt << std::fixed << std::setprecision(2) << acc->getBalance();
            std::cout << "  │  余额："
                      << std::left << std::setw(22) << bal_fmt.str() << "│\n";
            std::cout << "  └─────────────────────────────────┘\n";
        }
    }
    pause();
}

void handleViewLedger(LedgerSystem& sys) {
    clearScreen();
    std::cout << "\n  ═══ " << _("View Ledger", "查看账本") << " ═══\n\n";

    std::string accNum = readLine(_("  Account Number: ", "  账号："));

    Account* acc = sys.searchAccount(accNum);
    if (!acc) {
        std::cout << _("\n  [ERROR] Account [", "\n  [ERROR] 账号[")
                  << accNum << _("] not found.\n", "] 未找到。\n");
    } else {
        sys.viewLedger(accNum);
    }
    pause();
}

void handleUndo(LedgerSystem& sys) {
    clearScreen();
    std::cout << "\n  ═══ " << _("Undo Last Transaction", "撤销最后一笔交易") << " ═══\n\n";

    sys.undo();
    pause();
}

void handleDeleteAccount(LedgerSystem& sys) {
    clearScreen();
    std::cout << "\n  ═══ " << _("Delete Account", "删除账户") << " ═══\n\n";

    std::string accNum = readLine(_("  Account Number: ", "  账号："));

    Account* acc = sys.searchAccount(accNum);
    if (!acc) {
        std::cout << _("\n  [ERROR] Account [", "\n  [ERROR] 账号[")
                  << accNum << _("] not found.\n", "] 未找到。\n");
        pause();
        return;
    }

    if (g_english) {
        std::cout << "\n  ┌─────────────────────────────────┐\n";
        std::cout << "  │  Account to delete:             │\n";
        std::cout << "  ├─────────────────────────────────┤\n";
        std::cout << "  │  Account No: "
                  << std::left << std::setw(20) << acc->getAccountNumber() << "│\n";
        std::cout << "  │  User Name:  "
                  << std::left << std::setw(20) << acc->getUserName()     << "│\n";
        std::ostringstream bal_fmt;
        bal_fmt << std::fixed << std::setprecision(2) << acc->getBalance();
        std::cout << "  │  Balance:    "
                  << std::left << std::setw(20) << bal_fmt.str() << "│\n";
        std::cout << "  └─────────────────────────────────┘\n";
    } else {
        std::cout << "\n  ┌─────────────────────────────────┐\n";
        std::cout << "  │  要删除的账户：                  │\n";
        std::cout << "  ├─────────────────────────────────┤\n";
        std::cout << "  │  账号："
                  << std::left << std::setw(22) << acc->getAccountNumber() << "│\n";
        std::cout << "  │  用户名："
                  << std::left << std::setw(20) << acc->getUserName()     << "│\n";
        std::ostringstream bal_fmt;
        bal_fmt << std::fixed << std::setprecision(2) << acc->getBalance();
        std::cout << "  │  余额："
                  << std::left << std::setw(22) << bal_fmt.str() << "│\n";
        std::cout << "  └─────────────────────────────────┘\n";
    }

    if (confirm(_("\n  Delete this account?", "\n  是否删除此账户？"))) {
        sys.deleteAccount(accNum);
    } else {
        std::cout << _("  [INFO] Deletion cancelled.\n", "  [INFO] 已取消删除。\n");
    }
    pause();
}

int main() {
    LedgerSystem sys;

    while (true) {
        clearScreen();
        displayMenu();

        int choice = readChoice();

        switch (choice) {
            case 0: toggleLanguage(); break;
            case 1: handleCreateAccount(sys); break;
            case 2: handleDeposit(sys);       break;
            case 3: handleWithdraw(sys);      break;
            case 4: handleTransfer(sys);      break;
            case 5: handleSearchAccount(sys); break;
            case 6: handleViewLedger(sys);    break;
            case 7: handleUndo(sys);          break;
            case 8: handleDeleteAccount(sys); break;
            case 9:
                clearScreen();
                if (g_english) {
                    std::cout << "\n"
                              << "  ================================================\n"
                              << "                                                  \n"
                               << "          Thank you for using paypass_lite!          \n"
                               << "                    Goodbye!                       \n"
                               << "                                                  \n"
                               << "  ================================================\n"
                               << "\n";
                } else {
                    std::cout << "\n"
                              << "  ================================================\n"
                              << "                                                  \n"
                              << "             感谢使用交易轻系统！                  \n"
                              << "                   再见！                          \n"
                              << "                                                  \n"
                              << "  ================================================\n"
                              << "\n";
                }
                return 0;
        }
    }
    return 0;
}
