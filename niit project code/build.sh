#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

RED='\033[0;31m'; GREEN='\033[0;32m'; CYAN='\033[0;36m'; NC='\033[0m'
CC="${CC:-gcc}"
CFLAGS="-std=c99 -Wall -Wextra"

log()  { printf "${CYAN}[BUILD]${NC} %s\n" "$*"; }
ok()   { printf "${GREEN}[OK]${NC}    %s\n" "$*"; }
fail() { printf "${RED}[FAIL]${NC}  %s\n" "$*"; exit 1; }

rm -f test_core paypass_lite paypass_server 2>/dev/null || true

log "Building test_core..."
$CC $CFLAGS core/*.c test_main.c -o test_core -lm || fail "test_core compilation failed"
ok "test_core"

log "Building paypass_lite..."
$CC $CFLAGS -Icore core/*.c cli/main_cli.c -o paypass_lite || fail "paypass_lite compilation failed"
ok "paypass_lite"

log "Building paypass_server..."
$CC $CFLAGS -Icore core/*.c web/main_server.c -o paypass_server || fail "paypass_server compilation failed"
ok "paypass_server"

echo ""
printf "${GREEN}============================================${NC}\n"
printf "${GREEN}  Build Complete - All Binaries Ready${NC}\n"
printf "${GREEN}============================================${NC}\n"
printf "  %-20s %s\n" "test_core"      "./test_core"
printf "  %-20s %s\n" "paypass_lite"     "./paypass_lite"
printf "  %-20s %s\n" "paypass_server"  "bash run.sh"
echo ""
