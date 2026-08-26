@echo off
set "RENODE_HOME=E:\ProgramFiles\Renode\renode_1.16.1+20260825giteedd64f2e-portable"
set "DEMO=%~dp0"
python "%DEMO%build_hello.py"
"%RENODE_HOME%\renode.exe" --disable-gui -P -1 --plain --hide-analyzers -e "i @E:/Codes/CAndC++/1-8Simulate/demo-stm32f401rc/stm32f401rc.resc; emulation RunFor ""00:00:00.05""; q"
echo ---- uart.log ----
type "%DEMO%uart.log"
