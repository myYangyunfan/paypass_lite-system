#include "ledger_system.h"
#include "account.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_english = 1;

static void clear_screen(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

static void pause(void) {
    printf("\n  %s", g_english ? "Press Enter to continue..." : "按回车键继续...");
    char dummy[256];
    fgets(dummy, sizeof(dummy), stdin);
}

static void trim(char *s) {
    int len = (int)strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r' || s[len - 1] == ' ' || s[len - 1] == '\t')) {
        s[--len] = '\0';
    }
    int start = 0;
    while (s[start] == ' ' || s[start] == '\t') start++;
    if (start > 0) {
        memmove(s, s + start, len - start + 1);
    }
}

static void read_line(const char *prompt, char *buf, int size) {
    while (1) {
        printf("%s", prompt);
        fflush(stdout);
        if (!fgets(buf, size, stdin)) {
            buf[0] = '\0';
            continue;
        }
        trim(buf);
        if (buf[0] == '\0') {
            printf("  %s\n", g_english ? "[ERROR] Input cannot be empty." : "[ERROR] 输入不能为空。");
            continue;
        }
        return;
    }
}

static double read_amount(const char *prompt) {
    double value;
    char buf[256];
    while (1) {
        printf("%s", prompt);
        fflush(stdout);
        if (!fgets(buf, sizeof(buf), stdin)) {
            printf("  %s\n", g_english ? "[ERROR] Please enter a valid number." : "[ERROR] 请输入有效数字。");
            continue;
        }
        if (sscanf(buf, "%lf", &value) != 1) {
            printf("  %s\n", g_english ? "[ERROR] Please enter a valid number." : "[ERROR] 请输入有效数字。");
            continue;
        }
        if (value <= 0) {
            printf("  %s\n", g_english ? "[ERROR] Amount must be positive (> 0)." : "[ERROR] 金额必须为正数 (> 0)。");
            continue;
        }
        if (value > 999999999.99) {
            printf("  %s\n", g_english ? "[ERROR] Amount exceeds limit (999,999,999.99)." : "[ERROR] 金额超出限制 (999,999,999.99)。");
            continue;
        }
        return value;
    }
}

static int read_choice(void) {
    int choice;
    char buf[256];
    while (1) {
        printf("\n  %s", g_english ? "Enter your choice [0-9]: " : "请输入选项 [0-9]: ");
        fflush(stdout);
        if (!fgets(buf, sizeof(buf), stdin)) {
            printf("  %s\n", g_english ? "[ERROR] Please enter a number." : "[ERROR] 请输入数字。");
            continue;
        }
        if (sscanf(buf, "%d", &choice) != 1) {
            printf("  %s\n", g_english ? "[ERROR] Please enter a number." : "[ERROR] 请输入数字。");
            continue;
        }
        if (choice < 0 || choice > 9) {
            printf("  %s\n", g_english ? "[ERROR] Please enter 0-9." : "[ERROR] 请输入 0-9。");
            continue;
        }
        return choice;
    }
}

static int confirm(const char *prompt) {
    char buf[256];
    while (1) {
        printf("%s (y/n): ", prompt);
        fflush(stdout);
        if (!fgets(buf, sizeof(buf), stdin)) continue;
        trim(buf);
        if (strcmp(buf, "y") == 0 || strcmp(buf, "Y") == 0) return 1;
        if (strcmp(buf, "n") == 0 || strcmp(buf, "N") == 0) return 0;
        printf("  %s\n", g_english ? "Please enter 'y' or 'n'." : "请输入 'y' 或 'n'。");
    }
}

static void display_menu(void) {
    printf("\n");
    printf("  +===================================================+\n");
    printf("  |              paypass_lite (交易轻)                 |\n");
    printf("  +===================================================+\n");
    printf("  |                                                   |\n");
    if (g_english) {
        printf("  |  [0] Toggle Language (中文/English)              |\n");
        printf("  |  [1] Create Account                              |\n");
        printf("  |  [2] Deposit Money                               |\n");
        printf("  |  [3] Withdraw Money                              |\n");
        printf("  |  [4] Transfer Money                              |\n");
        printf("  |  [5] Search Account                              |\n");
        printf("  |  [6] View Ledger                                 |\n");
        printf("  |  [7] Undo Transaction                            |\n");
        printf("  |  [8] Delete Account                              |\n");
        printf("  |  [9] Exit                                        |\n");
    } else {
        printf("  |  [0] 切换语言 (English/中文)                     |\n");
        printf("  |  [1] 创建账户                                    |\n");
        printf("  |  [2] 存款                                        |\n");
        printf("  |  [3] 取款                                        |\n");
        printf("  |  [4] 转账                                        |\n");
        printf("  |  [5] 查询账户                                    |\n");
        printf("  |  [6] 查看账本                                    |\n");
        printf("  |  [7] 撤销交易                                    |\n");
        printf("  |  [8] 删除账户                                    |\n");
        printf("  |  [9] 退出                                        |\n");
    }
    printf("  |                                                   |\n");
    printf("  +===================================================+\n");
}

static void handle_create_account(LedgerSystem *sys) {
    char acc_num[64], name[128], phone[32];
    clear_screen();
    printf("\n  ═══ %s ═══\n\n", g_english ? "Create Account" : "创建账户");
    read_line(g_english ? "  Account Number: " : "  账号：", acc_num, sizeof(acc_num));
    read_line(g_english ? "  User Name:      " : "  用户名：", name, sizeof(name));
    read_line(g_english ? "  Phone Number:   " : "  电话：", phone, sizeof(phone));
    ls_create_account(sys, acc_num, name, phone);
    pause();
}

static void handle_deposit(LedgerSystem *sys) {
    char acc_num[64];
    clear_screen();
    printf("\n  ═══ %s ═══\n\n", g_english ? "Deposit Money" : "存款");
    read_line(g_english ? "  Account Number: " : "  账号：", acc_num, sizeof(acc_num));
    double amount = read_amount(g_english ? "  Amount:         " : "  金额：");
    ls_deposit(sys, acc_num, amount);
    pause();
}

static void handle_withdraw(LedgerSystem *sys) {
    char acc_num[64];
    clear_screen();
    printf("\n  ═══ %s ═══\n\n", g_english ? "Withdraw Money" : "取款");
    read_line(g_english ? "  Account Number: " : "  账号：", acc_num, sizeof(acc_num));
    double amount = read_amount(g_english ? "  Amount:         " : "  金额：");
    ls_withdraw(sys, acc_num, amount);
    pause();
}

static void handle_transfer(LedgerSystem *sys) {
    char from[64], to[64];
    clear_screen();
    printf("\n  ═══ %s ═══\n\n", g_english ? "Transfer Money" : "转账");
    read_line(g_english ? "  From Account: " : "  来源账号：", from, sizeof(from));
    read_line(g_english ? "  To Account:   " : "  目标账号：", to, sizeof(to));
    double amount = read_amount(g_english ? "  Amount:       " : "  金额：");
    ls_transfer(sys, from, to, amount);
    pause();
}

static void handle_search_account(LedgerSystem *sys) {
    char acc_num[64];
    clear_screen();
    printf("\n  ═══ %s ═══\n\n", g_english ? "Search Account" : "查询账户");
    read_line(g_english ? "  Account Number: " : "  账号：", acc_num, sizeof(acc_num));
    Account *acc = ls_search_account(sys, acc_num);
    if (!acc) {
        printf("\n  %s%s%s\n", g_english ? "[ERROR] Account [" : "[ERROR] 账号[", acc_num, g_english ? "] not found." : "] 未找到。");
    } else {
        if (g_english) {
            printf("\n  ┌─────────────────────────────────┐\n");
            printf("  │  Account Details                │\n");
            printf("  ├─────────────────────────────────┤\n");
            printf("  │  Account No: %-20s│\n", acc->account_number);
            printf("  │  User Name:  %-20s│\n", acc->user_name);
            printf("  │  Phone:      %-20s│\n", acc->phone_number);
            printf("  │  Balance:    %-20.2f│\n", acc->balance);
            printf("  └─────────────────────────────────┘\n");
        } else {
            printf("\n  ┌─────────────────────────────────┐\n");
            printf("  │  账户详情                        │\n");
            printf("  ├─────────────────────────────────┤\n");
            printf("  │  账号：%-22s│\n", acc->account_number);
            printf("  │  用户名：%-20s│\n", acc->user_name);
            printf("  │  电话：%-22s│\n", acc->phone_number);
            printf("  │  余额：%-22.2f│\n", acc->balance);
            printf("  └─────────────────────────────────┘\n");
        }
    }
    pause();
}

static void handle_view_ledger(LedgerSystem *sys) {
    char acc_num[64];
    clear_screen();
    printf("\n  ═══ %s ═══\n\n", g_english ? "View Ledger" : "查看账本");
    read_line(g_english ? "  Account Number: " : "  账号：", acc_num, sizeof(acc_num));
    Account *acc = ls_search_account(sys, acc_num);
    if (!acc) {
        printf("\n  %s%s%s\n", g_english ? "[ERROR] Account [" : "[ERROR] 账号[", acc_num, g_english ? "] not found." : "] 未找到。");
    } else {
        ls_view_ledger(sys, acc_num);
    }
    pause();
}

static void handle_undo(LedgerSystem *sys) {
    clear_screen();
    printf("\n  ═══ %s ═══\n\n", g_english ? "Undo Last Transaction" : "撤销最后一笔交易");
    ls_undo(sys);
    pause();
}

static void handle_delete_account(LedgerSystem *sys) {
    char acc_num[64];
    clear_screen();
    printf("\n  ═══ %s ═══\n\n", g_english ? "Delete Account" : "删除账户");
    read_line(g_english ? "  Account Number: " : "  账号：", acc_num, sizeof(acc_num));
    Account *acc = ls_search_account(sys, acc_num);
    if (!acc) {
        printf("\n  %s%s%s\n", g_english ? "[ERROR] Account [" : "[ERROR] 账号[", acc_num, g_english ? "] not found." : "] 未找到。");
        pause();
        return;
    }
    if (g_english) {
        printf("\n  ┌─────────────────────────────────┐\n");
        printf("  │  Account to delete:             │\n");
        printf("  ├─────────────────────────────────┤\n");
        printf("  │  Account No: %-20s│\n", acc->account_number);
        printf("  │  User Name:  %-20s│\n", acc->user_name);
        printf("  │  Balance:    %-20.2f│\n", acc->balance);
        printf("  └─────────────────────────────────┘\n");
    } else {
        printf("\n  ┌─────────────────────────────────┐\n");
        printf("  │  要删除的账户：                  │\n");
        printf("  ├─────────────────────────────────┤\n");
        printf("  │  账号：%-22s│\n", acc->account_number);
        printf("  │  用户名：%-20s│\n", acc->user_name);
        printf("  │  余额：%-22.2f│\n", acc->balance);
        printf("  └─────────────────────────────────┘\n");
    }
    if (confirm(g_english ? "\n  Delete this account?" : "\n  是否删除此账户？")) {
        ls_delete_account(sys, acc_num);
    } else {
        printf("  %s\n", g_english ? "[INFO] Deletion cancelled." : "[INFO] 已取消删除。");
    }
    pause();
}

int main(void) {
    LedgerSystem sys;
    ls_init(&sys);

    while (1) {
        clear_screen();
        display_menu();
        int choice = read_choice();
        switch (choice) {
            case 0: g_english = !g_english; break;
            case 1: handle_create_account(&sys); break;
            case 2: handle_deposit(&sys); break;
            case 3: handle_withdraw(&sys); break;
            case 4: handle_transfer(&sys); break;
            case 5: handle_search_account(&sys); break;
            case 6: handle_view_ledger(&sys); break;
            case 7: handle_undo(&sys); break;
            case 8: handle_delete_account(&sys); break;
            case 9:
                clear_screen();
                if (g_english) {
                    printf("\n");
                    printf("  ================================================\n");
                    printf("                                                  \n");
                    printf("          Thank you for using paypass_lite!          \n");
                    printf("                    Goodbye!                       \n");
                    printf("                                                  \n");
                    printf("  ================================================\n");
                    printf("\n");
                } else {
                    printf("\n");
                    printf("  ================================================\n");
                    printf("                                                  \n");
                    printf("             感谢使用交易轻系统！                  \n");
                    printf("                   再见！                          \n");
                    printf("                                                  \n");
                    printf("  ================================================\n");
                    printf("\n");
                }
                ls_destroy(&sys);
                return 0;
        }
    }
    ls_destroy(&sys);
    return 0;
}
