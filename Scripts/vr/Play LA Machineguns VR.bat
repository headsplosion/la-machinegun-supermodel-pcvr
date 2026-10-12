@echo off
setlocal
cd /d "%~dp0"
if not exist "Saves" mkdir "Saves"
if not exist "NVRAM" mkdir "NVRAM"
set "ROM=ROMs\lamachin.zip"
if not exist "%ROM%" set "ROM=..\lamachin.zip"
if not exist "%ROM%" (
  echo lamachin.zip not found. Put it in the ROMs folder next to this file.
  pause
  exit /b 1
)
rem The headset must be connected first: Virtual Desktop streaming, Quest Link or SteamVR.
supermodel.exe "%ROM%" -vr -window -res=992,768 -no-wide-screen
if errorlevel 1 (
  echo.
  echo Supermodel stopped with an error - see Supermodel.log in this folder.
  pause
)
