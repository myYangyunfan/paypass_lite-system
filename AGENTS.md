# AGENTS.md

## Project Overview

C++14 paypass_lite (交易轻) ledger system with hand-written data structures (Hash Table, Doubly Linked List, Stack). Educational project for NIIT course.

## Build & Run Commands

```bash
# Linux/WSL
bash build.sh              # Build all binaries
./test_core                # Run unit tests
bash run.sh                # Start web server + open browser
bash test.sh               # API integration tests (server must be running)

# Windows (requires g++ in PATH, e.g. MinGW-w64)
build.bat                  # Build all binaries
test_core.exe              # Run unit tests
run.bat                    # Start web server + open browser
test.bat                   # API integration tests
```

## Project Structure

- `core/` — Core library (Hash Table, Account, Transaction, Stack, LedgerSystem facade)
- `cli/` — Terminal CLI (`main_cli.cpp`) — bilingual (English/Chinese)
- `web/` — HTTP server (`main_server.cpp`) + static frontend (`static/`) — bilingual GUI
- `docs/MANUAL.md` — Bilingual user manual (English + Chinese)
- `test_main.cpp` — Unit tests (custom framework, not gtest)
- Scripts: `build.sh`/`build.bat`, `run.sh`/`run.bat`, `test.sh`/`test.bat`, `test.ps1`

## Key Architecture Facts

- **Facade pattern**: `LedgerSystem` class wraps all business logic; direct HashTable/Stack access is internal
- **Atomic transfers**: All validations complete before any balance changes — no partial failure
- **No STL containers**: All data structures are hand-written (Hash Table size=101 prime, Doubly Linked List, fixed-size Stack capacity=1000)
- **Self-contained HTTP server**: POSIX sockets on Linux, Winsock2 on Windows, zero third-party dependencies, includes minimal HTTP/1.1 and JSON parsers
- **Destruction chain**: `LedgerSystem → HashTable → Account → DoublyLinkedList → Transaction` (all heap-allocated, no leaks)

## Build Details

- Compiler: `g++` (GCC 5+), flags: `-std=c++14 -Wall -Wextra`
- CLI and server require `-Icore` to find headers
- Test binary: `core/*.cpp test_main.cpp` (no `-Icore` needed)
- Server on Windows: link with `-lws2_32` (Winsock2), use `-static` to avoid missing DLLs
- Platform: Linux/WSL/Windows (MinGW-w64) all supported

## Testing

- Unit tests: `./test_core` or `test_core.exe` (custom `VERIFY` macro, not gtest)
- API tests: `bash test.sh` / `test.ps1` — runs curl tests against localhost:8080
- Expected output: `ALL TESTS PASSED (0 failures)`
- Valgrind available: `valgrind --leak-check=full ./test_core` (Linux only)

## Development Workflow

When adding features:
1. Core logic → `core/ledger_system.h/.cpp`
2. CLI handler → `cli/main_cli.cpp`
3. API route → `web/main_server.cpp`
4. Frontend → `web/static/index.html` + `app.js`
5. Rebuild → `bash build.sh` (Linux) or `build.bat` (Windows)
