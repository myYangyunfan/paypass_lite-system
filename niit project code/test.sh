#!/usr/bin/env bash
# ============================================================
#  paypass_lite (交易轻) — Automated API Test Suite
#  Requires: paypass_server running on localhost:8080
#  Run:     bash test.sh
# ============================================================
set -euo pipefail

PORT="${1:-8080}"
BASE="http://localhost:$PORT/api"
GREEN='\033[0;32m'; RED='\033[0;31m'; CYAN='\033[0;36m'; YELLOW='\033[1;33m'; NC='\033[0m'

PASS=0; FAIL=0

header() { printf "\n${CYAN}═══ %s ═══${NC}\n" "$*"; }

check() {
    local desc="$1"; local expected="$2"; local method="$3"; local url="$4"; shift 4
    local body="${1:-}"
    local resp code
    resp=$(curl -s -w "\n%{http_code}" -X "$method" "$url" \
        ${body:+ -H "Content-Type: application/json" -d "$body"} 2>/dev/null)
    code=$(echo "$resp" | tail -1)
    local json=$(echo "$resp" | sed '$d')

    if [ "$code" = "$expected" ]; then
        printf "  ${GREEN}[PASS]${NC} %s → HTTP %s\n" "$desc" "$code"
        PASS=$((PASS + 1))
    else
        printf "  ${RED}[FAIL]${NC} %s → expected HTTP %s, got %s\n" "$desc" "$expected" "$code"
        printf "         Response: %s\n" "$(echo "$json" | head -c 200)"
        FAIL=$((FAIL + 1))
    fi
    # Return the JSON for chained tests
    echo "$json" > /tmp/paypass_test_last.json
}

# ---- Check server is running ----
if ! curl -s -o /dev/null "http://localhost:$PORT/" 2>/dev/null; then
    printf "${RED}[FATAL]${NC} Server is not running on port $PORT.\n"
    printf "Start it with: ${CYAN}bash run.sh${NC}\n"
    exit 1
fi
printf "${GREEN}[READY]${NC} Server is running on port $PORT\n"

# ============================================
#  Test Suite
# ============================================

header "1. Account Creation"
check "Create ACC001"           "201" POST "$BASE/accounts" \
    '{"account_number":"ACC001","user_name":"Alice","phone_number":"13800001111"}'
check "Create ACC002"           "201" POST "$BASE/accounts" \
    '{"account_number":"ACC002","user_name":"Bob","phone_number":"13900002222"}'
check "Duplicate ACC001"        "409" POST "$BASE/accounts" \
    '{"account_number":"ACC001","user_name":"Dup"}'
check "Missing fields"          "400" POST "$BASE/accounts" \
    '{"account_number":"X"}'
check "Empty account number"    "400" POST "$BASE/accounts" \
    '{"account_number":"","user_name":"X"}'

header "2. Deposit"
check "Deposit 1000 to ACC001"  "200" POST "$BASE/accounts/ACC001/deposit" \
    '{"amount":1000}'
check "Deposit 500 to ACC002"   "200" POST "$BASE/accounts/ACC002/deposit" \
    '{"amount":500.50}'
check "Deposit to nonexistent"  "404" POST "$BASE/accounts/GHOST/deposit" \
    '{"amount":100}'
check "Negative amount"         "422" POST "$BASE/accounts/ACC001/deposit" \
    '{"amount":-50}'
check "Zero amount"             "422" POST "$BASE/accounts/ACC001/deposit" \
    '{"amount":0}'
check "Missing amount"          "400" POST "$BASE/accounts/ACC001/deposit" \
    '{}'

header "3. Withdraw"
check "Withdraw 200 from ACC001"  "200" POST "$BASE/accounts/ACC001/withdraw" \
    '{"amount":200}'
check "Withdraw 50.75"            "200" POST "$BASE/accounts/ACC001/withdraw" \
    '{"amount":50.75}'
check "Insufficient balance"      "422" POST "$BASE/accounts/ACC001/withdraw" \
    '{"amount":999999}'
check "Withdraw from nonexistent" "404" POST "$BASE/accounts/GHOST/withdraw" \
    '{"amount":10}'

header "4. Transfer"
check "Transfer 300 A→B"        "200" POST "$BASE/transfer" \
    '{"from":"ACC001","to":"ACC002","amount":300}'
check "Self-transfer rejected"  "422" POST "$BASE/transfer" \
    '{"from":"ACC001","to":"ACC001","amount":100}'
check "Insufficient for transfer" "422" POST "$BASE/transfer" \
    '{"from":"ACC001","to":"ACC002","amount":999999}'
check "Transfer to ghost"       "422" POST "$BASE/transfer" \
    '{"from":"ACC001","to":"GHOST","amount":10}'
check "Missing fields"          "400" POST "$BASE/transfer" \
    '{"from":"X"}'

header "5. Search Account"
check "Search ACC001"           "200" GET "$BASE/accounts/ACC001"
check "Search ACC002"           "200" GET "$BASE/accounts/ACC002"
check "Search nonexistent"      "404" GET "$BASE/accounts/GHOST"

header "6. View Ledger"
check "Ledger ACC001"           "200" GET "$BASE/accounts/ACC001/ledger"
check "Ledger ACC002"           "200" GET "$BASE/accounts/ACC002/ledger"

header "7. Undo Operations"
check "Undo (should work)"      "200" POST "$BASE/undo"
check "Undo (should work)"      "200" POST "$BASE/undo"
check "Undo (still has more)"   "200" POST "$BASE/undo"
# Drain remaining undo stack
while true; do
    resp=$(curl -s -w "\n%{http_code}" -X POST "$BASE/undo" 2>/dev/null)
    code=$(echo "$resp" | tail -1)
    [ "$code" = "422" ] && break
done
check "Undo empty stack"        "422" POST "$BASE/undo"

header "8. Delete Account"
check "Delete ACC002"           "200" DELETE "$BASE/accounts/ACC002"
check "Double-delete ACC002"    "404" DELETE "$BASE/accounts/ACC002"
check "Delete nonexistent"      "404" DELETE "$BASE/accounts/GHOST"

header "9. Error Handling"
check "Bad JSON body"           "400" POST "$BASE/accounts" \
    'not-valid-json'
check "Wrong Content-Type"      "400" POST "$BASE/accounts" \
    'not json'

header "10. CORS Preflight"
cors_resp=$(curl -s -o /dev/null -w "%{http_code}" -X OPTIONS \
    "$BASE/accounts/ACC001" \
    -H "Origin: http://example.com" \
    -H "Access-Control-Request-Method: GET" 2>/dev/null)
if [ "$cors_resp" = "204" ]; then
    printf "  ${GREEN}[PASS]${NC} CORS preflight → HTTP 204\n"
    PASS=$((PASS + 1))
else
    printf "  ${RED}[FAIL]${NC} CORS preflight → expected 204, got %s\n" "$cors_resp"
    FAIL=$((FAIL + 1))
fi

# ============================================
#  Summary
# ============================================
echo ""
printf "${CYAN}============================================${NC}\n"
TOTAL=$((PASS + FAIL))
if [ "$FAIL" -eq 0 ]; then
    printf "${GREEN}  ALL %d TESTS PASSED${NC}\n" "$TOTAL"
else
    printf "${RED}  %d/%d TESTS FAILED${NC}\n" "$FAIL" "$TOTAL"
fi
printf "${CYAN}============================================${NC}\n"

exit $FAIL
