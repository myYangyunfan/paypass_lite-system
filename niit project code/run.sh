#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

GREEN='\033[0;32m'; CYAN='\033[0;36m'; YELLOW='\033[1;33m'; NC='\033[0m'

PORT="${PORT:-8080}"
SERVER_BIN="./paypass_server"

if [ ! -x "$SERVER_BIN" ]; then
    printf "${YELLOW}[INFO]${NC} Server binary not found. Running build.sh...\n"
    bash build.sh
fi

EXISTING=$(lsof -ti:"$PORT" 2>/dev/null || true)
if [ -n "$EXISTING" ]; then
    printf "${YELLOW}[INFO]${NC} Killing existing process on port $PORT (PID: $EXISTING)...\n"
    kill "$EXISTING" 2>/dev/null || true
    sleep 1
fi

printf "${CYAN}[START]${NC} Starting paypass server on http://localhost:$PORT\n"
"$SERVER_BIN" &
SERVER_PID=$!

printf "${CYAN}[WAIT]${NC}  Waiting for server to be ready..."
for i in $(seq 1 10); do
    if curl -s -o /dev/null "http://localhost:$PORT/" 2>/dev/null; then
        printf " ${GREEN}ready${NC}\n"
        break
    fi
    sleep 0.5
    printf "."
done

printf "${CYAN}[OPEN]${NC}  Opening browser...\n"
sleep 0.5

case "$(uname -s)" in
    Linux*)
        if command -v xdg-open &>/dev/null; then
            xdg-open "http://localhost:$PORT" &
        elif command -v sensible-browser &>/dev/null; then
            sensible-browser "http://localhost:$PORT" &
        else
            printf "${YELLOW}[INFO]${NC} Please open http://localhost:$PORT in your browser\n"
        fi
        ;;
    Darwin*)
        open "http://localhost:$PORT" &
        ;;
    CYGWIN*|MINGW*|MSYS*)
        cmd.exe /c start "http://localhost:$PORT" 2>/dev/null || \
        printf "${YELLOW}[INFO]${NC} Please open http://localhost:$PORT in your browser\n"
        ;;
    *)
        printf "${YELLOW}[INFO]${NC} Please open http://localhost:$PORT in your browser\n"
        ;;
esac

echo ""
printf "${GREEN}============================================${NC}\n"
printf "${GREEN}  paypass_lite is Running${NC}\n"
printf "${GREEN}============================================${NC}\n"
printf "  Web UI:      ${CYAN}http://localhost:$PORT${NC}\n"
printf "  API Base:    ${CYAN}http://localhost:$PORT/api${NC}\n"
printf "  Server PID:  %s\n" "$SERVER_PID"
printf "  Press ${YELLOW}Ctrl+C${NC} to stop the server\n"
echo ""

cleanup() {
    printf "\n${CYAN}[STOP]${NC}  Shutting down server (PID: $SERVER_PID)...\n"
    kill "$SERVER_PID" 2>/dev/null || true
    wait "$SERVER_PID" 2>/dev/null || true
    printf "${GREEN}[BYE]${NC}   Server stopped. Goodbye!\n"
}
trap cleanup EXIT INT TERM

wait "$SERVER_PID" 2>/dev/null || true
