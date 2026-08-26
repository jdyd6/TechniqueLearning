@echo off
setlocal

rem 使用确定的工具目录，避免依赖当前终端的临时 PATH。
set "RUBY_HOME=E:\ProgramFiles\Ruby34"
set "GCC_HOME=E:\Program Files\msys64\ucrt64"

if not exist "%RUBY_HOME%\bin\ceedling.bat" (
    echo Ceedling not found: %RUBY_HOME%\bin\ceedling.bat
    exit /b 1
)

if not exist "%GCC_HOME%\bin\gcc.exe" (
    echo GCC not found: %GCC_HOME%\bin\gcc.exe
    exit /b 1
)

set "PATH=%RUBY_HOME%\bin;%GCC_HOME%\bin;%PATH%"
call "%RUBY_HOME%\bin\ceedling.bat" test:all
exit /b %ERRORLEVEL%
