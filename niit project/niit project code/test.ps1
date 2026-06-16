# API Integration Tests — run via PowerShell

$host.ui.RawUI.WindowTitle = "paypass_lite API Tests"

# Check server
try {
    $r = curl.exe -s -o "nul" http://localhost:8080/
    if ($LASTEXITCODE -ne 0) { throw "no server" }
} catch {
    Write-Host "[FATAL] Server not running on port 8080." -ForegroundColor Red
    Write-Host "Start it with: run.bat" -ForegroundColor Red
    exit 1
}

$tmp = Join-Path $env:TEMP "paypass_test_ps"
New-Item -ItemType Directory -Path $tmp -Force | Out-Null

$pass = 0
$fail = 0

function Test-Api($desc, $method, $url, $body, $expected) {
    $rfile = Join-Path $tmp "r.json"
    $bfile = Join-Path $tmp "b.json"
    $cfile = Join-Path $tmp "code.txt"

    if ($body) {
        Set-Content -Path $bfile -Value $body -NoNewline -Encoding Ascii
        curl.exe -s -w "%{http_code}" -X $method "http://localhost:8080$url" -H "Content-Type: application/json" -d "@$bfile" -o "$rfile" > "$cfile" 2>$null
    } else {
        curl.exe -s -w "%{http_code}" -X $method "http://localhost:8080$url" -o "$rfile" > "$cfile" 2>$null
    }

    $code = Get-Content $cfile -Raw | ForEach-Object { $_.Trim() }

    if ($code -eq $expected) {
        Write-Host "  [PASS] $desc - HTTP $code" -ForegroundColor Green
        $script:pass++
    } else {
        Write-Host "  [FAIL] $desc - expected $expected, got $code" -ForegroundColor Red
        $body = Get-Content $rfile -Raw
        Write-Host "    $body" -ForegroundColor DarkRed
        $script:fail++
    }
}

Write-Host "Running API tests..." -ForegroundColor Cyan

Test-Api "1. Create ACC001"        POST  "/api/accounts"                     '{"account_number":"ACC001","user_name":"Alice","phone_number":"13800001111"}' 201
Test-Api "2. Create ACC002"        POST  "/api/accounts"                     '{"account_number":"ACC002","user_name":"Bob"}'                              201
Test-Api "3. Duplicate rejected"   POST  "/api/accounts"                     '{"account_number":"ACC001","user_name":"Alice"}'                            409
Test-Api "4. Deposit 1000"         POST  "/api/accounts/ACC001/deposit"      '{"amount":1000}'                                                            200
Test-Api "5. Negative deposit"     POST  "/api/accounts/ACC001/deposit"      '{"amount":-50}'                                                             422
Test-Api "6. Withdraw 200"         POST  "/api/accounts/ACC001/withdraw"     '{"amount":200}'                                                             200
Test-Api "7. Insufficient withdraw" POST  "/api/accounts/ACC001/withdraw"    '{"amount":999999}'                                                          422
Test-Api "8. Transfer 300"         POST  "/api/transfer"                     '{"from":"ACC001","to":"ACC002","amount":300}'                               200
Test-Api "9. Search ACC001"        GET   "/api/accounts/ACC001"              $null                                                                       200
Test-Api "10. Ledger ACC001"       GET   "/api/accounts/ACC001/ledger"       $null                                                                       200
Test-Api "11. Undo"                POST  "/api/undo"                         $null                                                                       200
Test-Api "12. Delete ACC002"       DELETE "/api/accounts/ACC002"             $null                                                                       200
Test-Api "13. Double delete"       DELETE "/api/accounts/ACC002"             $null                                                                       404
Test-Api "14. Bad JSON"            POST  "/api/accounts"                     "not-json"                                                                  400

$total = $pass + $fail
Write-Host ""
Write-Host "=" x 44 -ForegroundColor Cyan
if ($fail -eq 0) {
    Write-Host "  ALL $total TESTS PASSED" -ForegroundColor Green
} else {
    Write-Host "  $fail/$total TESTS FAILED" -ForegroundColor Red
}
Write-Host "=" x 44 -ForegroundColor Cyan
exit $fail
