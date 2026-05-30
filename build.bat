@echo off
chcp 65001 >nul

REM ============================================================
REM paypass-lite Windows 构建脚本
REM 功能：编译 CLI、Web 服务器和测试程序
REM ============================================================

echo.
echo ==========================================
echo   paypass-lite 项目构建脚本
echo ==========================================
echo.

REM 设置编译器参数
set "CXX=g++"
set "CXXFLAGS=-std=c++14 -Wall -Wextra"
set "OUTPUT_DIR=bin"

REM ============================================================
REM 步骤1：清理旧的构建输出
REM ============================================================
echo [1/4] 清理旧的构建文件...
if exist "%OUTPUT_DIR%" (
    echo        删除旧目录: %OUTPUT_DIR%
    rmdir /s /q "%OUTPUT_DIR%"
    if errorlevel 1 (
        echo [错误] 无法删除旧目录 %OUTPUT_DIR%
        pause
        exit /b 1
    )
)
echo        清理完成。
echo.

REM ============================================================
REM 步骤2：创建输出目录
REM ============================================================
echo [2/4] 创建输出目录...
mkdir "%OUTPUT_DIR%"
if errorlevel 1 (
    echo [错误] 无法创建目录 %OUTPUT_DIR%
    pause
    exit /b 1
)
echo        目录创建成功: %OUTPUT_DIR%\
echo.

REM ============================================================
REM 步骤3：编译 CLI 程序
REM ============================================================
echo [3/4] 开始编译...
echo.
echo        --- 编译 paypass-cli.exe ---
%CXX% %CXXFLAGS% core\*.cpp cli\main_cli.cpp -o %OUTPUT_DIR%\paypass-cli.exe
if errorlevel 1 (
    echo.
    echo [错误] paypass-cli.exe 编译失败！
    pause
    exit /b 1
)
echo        编译成功: %OUTPUT_DIR%\paypass-cli.exe
echo.

REM ============================================================
REM 步骤4：编译 Web 服务器程序
REM ============================================================
echo        --- 编译 paypass-server.exe ---
%CXX% %CXXFLAGS% core\*.cpp web\main_server.cpp -o %OUTPUT_DIR%\paypass-server.exe
if errorlevel 1 (
    echo.
    echo [错误] paypass-server.exe 编译失败！
    pause
    exit /b 1
)
echo        编译成功: %OUTPUT_DIR%\paypass-server.exe
echo.

REM ============================================================
REM 步骤5：编译测试程序
REM ============================================================
echo        --- 编译 test.exe ---
%CXX% %CXXFLAGS% core\*.cpp test_main.cpp -o %OUTPUT_DIR%\test.exe
if errorlevel 1 (
    echo.
    echo [错误] test.exe 编译失败！
    pause
    exit /b 1
)
echo        编译成功: %OUTPUT_DIR%\test.exe
echo.

REM ============================================================
REM 构建完成总结
REM ============================================================
echo [4/4] 构建完成！
echo.
echo ==========================================
echo   构建成功的文件：
echo     - %OUTPUT_DIR%\paypass-cli.exe
echo     - %OUTPUT_DIR%\paypass-server.exe
echo     - %OUTPUT_DIR%\test.exe
echo ==========================================
echo.

pause
