#!/usr/bin/env bash
# ============================================================
#  paypass_lite (交易轻) — Build Script
#  Compiles core tests, CLI, and web server
# ============================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

RED='\033[0;31m'; GREEN='\033[0;32m'; CYAN='\033[0;36m'; NC='\033[0m'
CXX="${CXX:-g++}"
CXXFLAGS="-std=c++14 -Wall -Wextra"

log()  { printf "${CYAN}[BUILD]${NC} %s\n" "$*"; }
ok()   { printf "${GREEN}[OK]${NC}    %s\n" "$*"; }
fail() { printf "${RED}[FAIL]${NC}  %s\n" "$*"; exit 1; }

# Clean old binaries (ignore permission errors)
rm -f test_core paypass_lite paypass_server 2>/dev/null || true

# ---- 1. Core Unit Tests ----
log "Building test_core..."
$CXX $CXXFLAGS core/*.cpp test_main.cpp -o test_core || fail "test_core compilation failed"
ok "test_core"

# ---- 2. Terminal CLI ----
log "Building paypass_lite..."
$CXX $CXXFLAGS -Icore core/*.cpp cli/main_cli.cpp -o paypass_lite || fail "paypass_lite compilation failed"
ok "paypass_lite"

# ---- 3. Web Server ----
log "Building paypass_server..."
$CXX $CXXFLAGS -Icore core/*.cpp web/main_server.cpp -o paypass_server || fail "paypass_server compilation failed"
ok "paypass_server"

# ---- Summary ----
echo ""
printf "${GREEN}============================================${NC}\n"
printf "${GREEN}  Build Complete — All Binaries Ready${NC}\n"
printf "${GREEN}============================================${NC}\n"
printf "  %-20s %s\n" "test_core"      "./test_core"
printf "  %-20s %s\n" "paypass_lite"     "./paypass_lite"
printf "  %-20s %s\n" "paypass_server"  "bash run.sh"
echo ""
