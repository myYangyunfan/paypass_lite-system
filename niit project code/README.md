# paypass_lite (交易轻)

> NIIT C Programming & Data Structures — Case Study 3  
> 交易轻 (paypass_lite)

A complete financial transaction processing engine built in C++, featuring hand-written data structures (Hash Table, Doubly Linked List, Stack), a terminal CLI, a RESTful API server, and a modern single-page web frontend.

---

## Table of Contents

- [Project Structure](#project-structure)
- [Quick Start](#quick-start)
- [Phase 1: Core Library](#phase-1-core-library)
- [Phase 2: Terminal CLI](#phase-2-terminal-cli)
- [Phase 3: REST API Server](#phase-3-rest-api-server)
- [Phase 4: Web Frontend](#phase-4-web-frontend)
- [API Reference](#api-reference)
- [Scripts](#scripts)
- [Development](#development)

---

## Project Structure

```
D:\niit project\
├── README.md                     ← This document
├── build.sh                      ← Compile everything
├── run.sh                        ← Start server + open browser
├── test.sh                       ← Automated API test suite
│
├── core/                         ← Phase 1: Core Library
│   ├── transaction.h / .cpp      ← Transaction node + DoublyLinkedList
│   ├── stack.h / .cpp            ← Array-based UndoStack
│   ├── account.h / .cpp          ← Account class with embedded ledger
│   ├── hashtable.h / .cpp        ← Hash table (chaining, size=101)
│   └── ledger_system.h / .cpp    ← Facade: all business logic
│
├── cli/                          ← Phase 2: Terminal CLI
│   └── main_cli.cpp              ← Interactive console interface
│
├── web/                          ← Phase 3+4: Web Server & Frontend
│   ├── main_server.cpp           ← HTTP server + REST API (self-contained)
│   └── static/
│       ├── index.html            ← Single-page web app
│       ├── style.css             ← Alipay blue theme
│       └── app.js                ← Frontend logic (Fetch API)
│
├── test_main.cpp                 ← Phase 1 unit tests
├── test_core                     ← Compiled core test binary
├── paypass_lite                    ← Compiled CLI binary
└── paypass_server                 ← Compiled server binary
```

---

## Quick Start

### Prerequisites

- **Linux** (or WSL on Windows) with GCC 5+ (C++14 support)
- **curl** (for API testing)
- A modern web browser (Chrome/Firefox/Edge)

### One-Command Setup

```bash
# Build everything
bash build.sh

# Run core unit tests
./test_core

# Launch interactive CLI
./paypass_lite

# Start web server (opens browser automatically)
bash run.sh
```

---

## Phase 1: Core Library

### Architecture

```
┌─────────────────────────────────────┐
│          LedgerSystem (Facade)      │
│  ┌──────────┐  ┌────────────────┐   │
│  │HashTable │  │  UndoStack     │   │
│  │(accounts)│  │  (undo history)│   │
│  └────┬─────┘  └────────────────┘   │
│       │                              │
│  ┌────▼──────────────────────────┐  │
│  │        Account                │  │
│  │  ┌────────────────────────┐   │  │
│  │  │ DoublyLinkedList       │   │  │
│  │  │ (transaction history)  │   │  │
│  │  └────────────────────────┘   │  │
│  └───────────────────────────────┘  │
└─────────────────────────────────────┘
```

### Data Structures (Hand-Written, Zero STL)

| Structure | Implementation | Purpose |
|---|---|---|
| **Hash Table** | Chain addressing, static size 101 (prime), djb2 hash | O(1) average account lookup |
| **Doubly Linked List** | Head/tail sentinel nodes, O(1) pushBack | Per-account transaction history |
| **Stack** | Fixed-size array (capacity 1000) | Undo operation history |

### Key Design Decisions

- **Atomic transfers**: All validations (account existence, balance sufficiency) complete before any balance changes — partial failure impossible
- **Undo via Stack**: Each push to the undo stack records operation type, accounts, amount, and transaction ID; undo reverses the balance and removes the transaction record
- **No memory leaks**: Every `new` has a corresponding `delete` in the destructor chain: `LedgerSystem → HashTable → Account → DoublyLinkedList → Transaction`

### Build & Test

```bash
g++ -std=c++14 -Wall -Wextra core/*.cpp test_main.cpp -o test_core
./test_core
# Expected: ALL TESTS PASSED (0 failures)
```

---

## Phase 2: Terminal CLI

### Menu

```
+===================================================+
|               paypass_lite (交易轻)                |
+===================================================+
|  [1] Create Account                               |
|  [2] Deposit Money                                |
|  [3] Withdraw Money                               |
|  [4] Transfer Money                               |
|  [5] Search Account                               |
|  [6] View Ledger                                  |
|  [7] Undo Transaction                             |
|  [8] Delete Account                               |
|  [9] Exit                                         |
+===================================================+
```

### Input Validation

- Numbers only → non-numeric input triggers retry without crash
- Amount ≤ 0 → rejected with `[ERROR] Amount must be positive (> 0)`
- Empty input → rejected with `[ERROR] Input cannot be empty`
- Cross-platform clear screen (`cls` on Windows, `clear` on Linux)

### Build & Run

```bash
g++ -std=c++14 -Wall -Wextra -Icore core/*.cpp cli/main_cli.cpp -o paypass_lite
./paypass_lite
```

---

## Phase 3: REST API Server

### Self-Contained Implementation

The server is implemented in a single C++ file using only POSIX sockets — **zero third-party dependencies**. Includes:

- **Minimal HTTP/1.1 parser**: parses method, path, headers, body, query strings
- **Minimal JSON parser**: recursive-descent, supports object/array/string/number/bool/null
- **CORS support**: all responses include `Access-Control-Allow-Origin: *`

### Build

```bash
g++ -std=c++14 -Wall -Wextra -Icore core/*.cpp web/main_server.cpp -o paypass_server
```

---

## Phase 4: Web Frontend

### UI Layout

```
┌──────────────┬───────────────────────────────────┐
│   Sidebar    │        Content Panel              │
│   (220px)    │                                   │
│              │  ┌─────────────────────────────┐  │
│  A paypass_lite    │  │  Form Card                  │  │
│   Ledger     │  │                             │  │
│  ─────────── │  │  [Input fields + Submit]    │  │
│  + Create    │  └─────────────────────────────┘  │
│  ↓ Deposit   │                                   │
│  ↑ Withdraw  │  ┌─────────────────────────────┐  │
│  ⇄ Transfer  │  │  Result Card                │  │
│  ⌕ Search    │  │  (account detail / ledger   │  │
│  ☰ Ledger    │  │   table / transfer summary) │  │
│  ↩ Undo      │  └─────────────────────────────┘  │
│  ✕ Delete    │                                   │
│              │                                   │
└──────────────┴───────────────────────────────────┘
```

### Tech Stack

- **Pure vanilla**: HTML5 + CSS3 + Vanilla JS — no frameworks (React/Vue)
- **Alipay Blue theme** (inspired by Alipay): `#1677FF` primary, `#0958D9` hover, card-based design
- **Responsive**: Flexbox + Grid, 768px breakpoint for mobile
- **Toast notifications**: Green (success) / Red (error), auto-dismiss 3s

### Features

| Feature | Implementation |
|---|---|
| Async operations | Fetch API, no page reloads |
| Form validation | HTML5 `required`, `type="number"`, `min` attributes |
| Button states | Spinner + disabled during requests |
| XSS protection | HTML entity escaping on all dynamic content |
| Currency format | `¥1,000.00` via `Intl.NumberFormat` |
| Empty ledger | "No transactions yet." italic display |
| Network errors | Caught and displayed as red Toast |

---

## API Reference

Base URL: `http://localhost:8080`

### 1. Create Account

```
POST /api/accounts
Content-Type: application/json

{
  "account_number": "ACC001",
  "user_name": "Alice",
  "phone_number": "13800001111"
}
```

**Success (201):**
```json
{"success":true,"message":"Created","account":{"account_number":"ACC001","user_name":"Alice","phone_number":"13800001111","balance":0}}
```

### 2. Deposit

```
POST /api/accounts/{id}/deposit
Content-Type: application/json

{"amount": 1000.00}
```

### 3. Withdraw

```
POST /api/accounts/{id}/withdraw
Content-Type: application/json

{"amount": 500.00}
```

### 4. Transfer

```
POST /api/transfer
Content-Type: application/json

{"from": "ACC001", "to": "ACC002", "amount": 300.00}
```

### 5. Search Account

```
GET /api/accounts/{id}
```

### 6. View Ledger

```
GET /api/accounts/{id}/ledger
```

**Response:**
```json
{
  "account_number":"ACC001","user_name":"Alice","balance":700,
  "transactions":[
    {"txn_id":1,"type":"DEPOSIT","amount":1000,"from":null,"to":"ACC001","timestamp":"2026-05-28 10:00:00"}
  ],
  "transaction_count":1
}
```

### 7. Undo Transaction

```
POST /api/undo
```

### 8. Delete Account

```
DELETE /api/accounts/{id}
```

### Error Codes

| HTTP Status | Meaning |
|---|---|
| 200 | Success |
| 201 | Created |
| 400 | Bad Request (invalid JSON, missing fields) |
| 404 | Not Found (account doesn't exist) |
| 409 | Conflict (duplicate account) |
| 422 | Unprocessable Entity (insufficient balance, negative amount, nothing to undo) |

---

## Scripts

### `build.sh` — Compile Everything

```bash
bash build.sh
```

Compiles:
- `test_core` — Core library unit tests
- `paypass_lite` — Terminal CLI application
- `paypass_server` — Web server with REST API + static files

Cleans old binaries first. Exits on first compilation error.

### `run.sh` — Start Server + Open Browser

```bash
bash run.sh
```

1. Checks if `paypass_server` exists (builds if missing)
2. Kills any existing server on port 8080
3. Starts the server in the background
4. Opens the default browser to `http://localhost:8080`
5. Prints server log output to terminal
6. Press `Ctrl+C` to stop the server

### `test.sh` — Automated API Test Suite

```bash
bash test.sh
```

Runs 20+ automated curl tests covering:
- Account creation, duplicate detection
- Deposit / Withdraw (success + insufficient balance)
- Transfer (success + atomicity verification)
- Search / Ledger view
- Undo operations
- Delete account
- Error scenarios (bad JSON, negative amounts, missing fields)
- CORS preflight

Requires: `paypass_server` running on port 8080.

---

## Development

### Compilation Flags

All binaries use:
```
-std=c++14 -Wall -Wextra
```

The CLI and server also need `-Icore` to find the core library headers.

### Memory Safety

Valgrind (if available):
```bash
valgrind --leak-check=full ./test_core
```

The destructor chain ensures all heap allocations are freed:
`LedgerSystem → HashTable → Account → DoublyLinkedList → Transaction`

### Adding New Features

1. Add the core logic to `core/ledger_system.h/.cpp`
2. Add CLI handler in `cli/main_cli.cpp`
3. Add API route in `web/main_server.cpp`
4. Add frontend panel in `web/static/index.html` + binding in `app.js`
5. Run `bash build.sh` to recompile

---

## License

Educational project for NIIT C Programming & Data Structures course.
