# paypass_lite (交易轻) — User Manual / 使用说明书

---

## 1. Overview / 概述

**paypass_lite (交易轻)** is a C++14 console/Web application that manages accounts and transactions (deposit, withdraw, transfer) with undo support. All data structures (Hash Table, Doubly Linked List, Stack) are hand-written — no STL containers.

**paypass_lite (交易轻)** 是一个 C++14 控制台/Web 应用程序，用于管理账户和交易（存款、取款、转账）并支持撤销操作。所有数据结构（哈希表、双向链表、栈）均为手写实现，不使用 STL 容器。

**Bilingual support / 双语支持**: Press `[0]` in CLI or click the **中文/English** button in the Web GUI to switch between English and Chinese.

---

## 2. Build / 编译

### Windows (MinGW-w64)

```batch
build.bat
```

Produces / 生成: `paypass_lite.exe`, `paypass_server.exe`, `test_core.exe`

### Linux / WSL

```bash
bash build.sh
```

Produces: `paypass_lite`, `paypass_server`, `test_core`

---

## 3. CLI Usage / 命令行使用说明

```bash
paypass_lite.exe      # Windows
./paypass_lite        # Linux/WSL
```

### Menu / 菜单

```
+===================================================+
|               paypass_lite (交易轻)                |
+===================================================+
|                                                   |
|  [0] Toggle Language (中文/English)              |
|  [1] Create Account                              |
|  [2] Deposit Money                               |
|  [3] Withdraw Money                              |
|  [4] Transfer Money                              |
|  [5] Search Account                              |
|  [6] View Ledger                                 |
|  [7] Undo Transaction                            |
|  [8] Delete Account                              |
|  [9] Exit                                        |
|                                                   |
+===================================================+
```

- Enter the number and press **Enter** to select / 输入数字后按**回车**选择
- Press `[0]` at any menu to switch language / 在主菜单按 `[0]` 切换语言

### Error messages from the core library (`[ERROR]`, `[OK]`, `[UNDO]`) remain in English. Only menu items, prompts, and labels are translated.

### 命令说明 / Commands

| Option | Action / 功能 | Description / 说明 |
|--------|--------------|-------------------|
| `[1]` | Create Account / 创建账户 | Enter account number, user name, phone |
| `[2]` | Deposit / 存款 | Add money to an account |
| `[3]` | Withdraw / 取款 | Remove money from an account |
| `[4]` | Transfer / 转账 | Move money between two accounts |
| `[5]` | Search / 查询 | Look up account details |
| `[6]` | View Ledger / 查看账本 | Show all transactions for an account |
| `[7]` | Undo / 撤销 | Reverse the most recent transaction |
| `[8]` | Delete / 删除 | Remove an account permanently |
| `[9]` | Exit / 退出 | Quit the application |

---

## 4. Web GUI Usage / Web 界面使用说明

```bash
run.bat            # Windows — starts server + opens browser
bash run.sh        # Linux/WSL
```

Or manually / 或手动运行:

```bash
paypass_server.exe        # Windows
./paypass_server          # Linux/WSL
```

Then open / 然后打开: http://localhost:8080

### Features / 功能

- **Sidebar navigation** / 侧边栏导航 — click any panel to switch
- **Language toggle** / 语言切换 — click the **中文/English** button at the bottom of the sidebar
- **Toast notifications** / 弹出通知 — success/error messages appear in the top-right corner
- **Transaction table** / 交易记录表 — ledger view shows all transactions with color-coded types
- **Server messages** are translated when switching language / 服务器返回的消息会随语言切换而翻译

### Panels / 面板

| Panel / 面板 | Purpose / 用途 |
|-------------|---------------|
| Create Account / 创建账户 | Register a new user |
| Deposit / 存款 | Add funds |
| Withdraw / 取款 | Remove funds |
| Transfer / 转账 | Move funds between accounts |
| Search / 查询 | View account details |
| View Ledger / 查看账本 | Show transaction history |
| Undo / 撤销 | Reverse last transaction |
| Delete / 删除 | Remove an account |

---

## 5. API Reference / API 参考

All endpoints expect/produce JSON. Base URL: `http://localhost:8080/api`

所有接口均使用 JSON 格式。基础地址：`http://localhost:8080/api`

### `POST /api/accounts` — Create Account / 创建账户

```json
// Request / 请求
{ "account_number": "ACC001", "user_name": "Alice", "phone_number": "13800001111" }
// Response / 响应 (201)
{ "success": true, "message": "Created", "account": { "account_number": "ACC001", "user_name": "Alice", "phone_number": "13800001111", "balance": 0.0 } }
```

### `POST /api/accounts/:id/deposit` — Deposit / 存款

```json
{ "amount": 100.00 }
```

### `POST /api/accounts/:id/withdraw` — Withdraw / 取款

```json
{ "amount": 50.00 }
```

### `POST /api/transfer` — Transfer / 转账

```json
{ "from": "ACC001", "to": "ACC002", "amount": 30.00 }
```

### `GET /api/accounts/:id` — Search / 查询账户

Returns account details / 返回账户详情.

### `GET /api/accounts/:id/ledger` — View Ledger / 查看账本

Returns account info + transaction list / 返回账户信息及交易列表.

### `POST /api/undo` — Undo / 撤销

Reverses the most recent transaction / 撤销最近一笔交易.

### `DELETE /api/accounts/:id` — Delete Account / 删除账户

Permanently removes the account / 永久删除账户.

### Error Response Format / 错误响应格式

```json
{ "success": false, "error": "Error message", "code": 404 }
```

### Common HTTP Status Codes / 常见状态码

| Code | Meaning / 含义 |
|------|---------------|
| 200 | Success / 成功 |
| 201 | Created / 已创建 |
| 400 | Bad Request (missing fields, invalid JSON) / 请求错误 |
| 404 | Not Found / 未找到 |
| 409 | Conflict (account exists) / 冲突（账户已存在） |
| 422 | Unprocessable (insufficient balance, self-transfer, etc.) / 无法处理 |

---

## 6. Testing / 测试

### Unit Tests / 单元测试

```bash
test_core.exe           # Windows
./test_core             # Linux/WSL
```

Expected output / 预期输出: `ALL TESTS PASSED (0 failures)`

### API Integration Tests / 集成测试

Start the server first / 先启动服务器:

```batch
start paypass_server.exe
```

Then run / 然后运行:

```batch
test.bat                # Windows
bash test.sh            # Linux/WSL
```

---

## 7. Project Structure / 项目结构

```
niit project code/
├── core/                   # Core library (Hash Table, Account, Transaction, Stack, LedgerSystem)
├── cli/main_cli.cpp        # Command-line interface (bilingual)
├── web/
│   ├── main_server.cpp     # HTTP server (bilingual API)
│   └── static/             # Web frontend (bilingual GUI)
│       ├── index.html
│       ├── app.js          # i18n translation engine
│       └── style.css
├── test_main.cpp           # Unit tests
├── docs/MANUAL.md          # This manual / 本说明书
├── build.bat / build.sh    # Build scripts
├── run.bat / run.sh        # Run scripts
└── test.bat / test.sh      # Test scripts
```

---

## 8. License / 许可

Educational project for NIIT course. All code is original with hand-written data structures.
NIIT 课程教育项目。所有代码均为原创，数据结构手写实现。
