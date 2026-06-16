@echo off
cd /d "%~dp0"

if not exist paypass_server.exe (
    echo [INFO] Building server first...
    call build.bat
    if errorlevel 1 (echo [FAIL] Build failed & pause & exit /b 1)
)

taskkill /F /IM paypass_server.exe >nul 2>&1

echo [START] Starting paypass_lite server...
echo.
echo   Open http://localhost:8080 in your browser.
echo   Close this window to stop the server.
echo.
start "paypass_lite Server" paypass_server.exe
