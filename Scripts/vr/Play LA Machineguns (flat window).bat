@echo off
setlocal
cd /d "%~dp0"
set "ROM=ROMs\lamachin.zip"
if not exist "%ROM%" set "ROM=..\lamachin.zip"
supermodel.exe "%ROM%" -window -res=992,768
