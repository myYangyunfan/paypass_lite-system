@echo off
setlocal
cd /d "%~dp0"
set CXX=g++
set FLAGS=-std=c++14 -Wall -Wextra -static

echo [BUILD] Building test_core...
%CXX% %FLAGS% core\*.cpp test_main.cpp -o test_core.exe
if errorlevel 1 (echo [FAIL] test_core & exit /b 1)
echo [OK]    test_core.exe

echo [BUILD] Building paypass_lite...
%CXX% %FLAGS% -Icore core\*.cpp cli\main_cli.cpp -o paypass_lite.exe
if errorlevel 1 (echo [FAIL] paypass_lite & exit /b 1)
echo [OK]    paypass_lite.exe

echo [BUILD] Building paypass_server...
%CXX% %FLAGS% -Icore core\*.cpp web\main_server.cpp -o paypass_server.exe -lws2_32
if errorlevel 1 (echo [FAIL] paypass_server & exit /b 1)
echo [OK]    paypass_server.exe

echo.
echo ============================================
echo   Build Complete - All Binaries Ready
echo ============================================
echo   test_core.exe       .\test_core.exe
echo   paypass_lite.exe      .\paypass_lite.exe
echo   paypass_server.exe   run.bat
echo.
endlocal
