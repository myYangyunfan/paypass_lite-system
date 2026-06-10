@echo off
setlocal
cd /d "%~dp0"

echo Checking server...
curl.exe -s -o nul http://localhost:8080/
if errorlevel 1 (
    echo [FATAL] Server not running on port 8080.
    echo Start it with: run.bat
    exit /b 1
)

set TMPDIR=%TEMP%\paypass_test

if not exist "%TMPDIR%" mkdir "%TMPDIR%"

set PASS=0
set FAIL=0

:: Helper: run test
setlocal EnableDelayedExpansion
call :test "1. Create ACC001" POST /api/accounts "{\"account_number\":\"ACC001\",\"user_name\":\"Alice\",\"phone_number\":\"13800001111\"}" 201
call :test "2. Create ACC002" POST /api/accounts "{\"account_number\":\"ACC002\",\"user_name\":\"Bob\"}" 201
call :test "3. Duplicate rejected" POST /api/accounts "{\"account_number\":\"ACC001\"}" 409
call :test "4. Deposit 1000" POST /api/accounts/ACC001/deposit "{\"amount\":1000}" 200
call :test "5. Negative deposit" POST /api/accounts/ACC001/deposit "{\"amount\":-50}" 422
call :test "6. Withdraw 200" POST /api/accounts/ACC001/withdraw "{\"amount\":200}" 200
call :test "7. Insufficient withdraw" POST /api/accounts/ACC001/withdraw "{\"amount\":999999}" 422
call :test "8. Transfer 300" POST /api/transfer "{\"from\":\"ACC001\",\"to\":\"ACC002\",\"amount\":300}" 200
call :test "9. Search ACC001" GET /api/accounts/ACC001 200
call :test "10. Ledger ACC001" GET /api/accounts/ACC001/ledger 200
call :test "11. Undo" POST /api/undo 200
call :test "12. Delete ACC002" DELETE /api/accounts/ACC002 200
call :test "13. Double delete" DELETE /api/accounts/ACC002 404
call :test "14. Bad JSON" POST /api/accounts "not-json" 400

echo.
echo ============================================
set /a TOTAL=PASS+FAIL
if %FAIL%==0 (
    echo   ALL %TOTAL% TESTS PASSED
) else (
    echo   %FAIL%/%TOTAL% TESTS FAILED
)
echo ============================================

exit /b %FAIL%

:test
set DESC=%~1
set METHOD=%~2
set URL=%~3
set BODY=%~4
set EXPECTED=%~5

if "!BODY!"=="" (
    curl.exe -s -w "%%{http_code}" -X !METHOD! http://localhost:8080!URL! -o "%TMPDIR%\r.json" > "%TMPDIR%\code.txt"
) else (
    echo !BODY! > "%TMPDIR%\b.json"
    curl.exe -s -w "%%{http_code}" -X !METHOD! http://localhost:8080!URL! -H "Content-Type: application/json" -d "@%TMPDIR%\b.json" -o "%TMPDIR%\r.json" > "%TMPDIR%\code.txt"
)

set /p CODE=<"%TMPDIR%\code.txt"
if "!CODE!"=="!EXPECTED!" (
    echo   [PASS] !DESC! - HTTP !CODE!
    set /a PASS+=1
) else (
    echo   [FAIL] !DESC! - expected !EXPECTED!, got !CODE!
    type "%TMPDIR%\r.json"
    echo.
    set /a FAIL+=1
)
exit /b
endlocal
