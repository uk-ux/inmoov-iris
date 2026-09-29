@echo off
title InMoov voice launcher
cd /d U:\inmoov

echo [1/4] UI server on port 8765...
netstat -ano | findstr ":8765" | findstr "LISTENING" >nul
if errorlevel 1 start "InMoov UI server" cmd /k py -m http.server 8765 --bind 127.0.0.1 --directory control_ui

echo [2/4] Ollama LLM service...
tasklist /FI "IMAGENAME eq ollama.exe" 2>nul | find /I "ollama.exe" >nul
if errorlevel 1 (
  if exist "%LOCALAPPDATA%\Programs\Ollama\ollama.exe" (
    start "Ollama" /min "%LOCALAPPDATA%\Programs\Ollama\ollama.exe" serve
  ) else (
    start "Ollama" /min ollama serve
  )
)

echo [3/4] Voice backend (whisper + piper + mms + qwen)...
netstat -ano | findstr ":8766" | findstr "LISTENING" >nul
if errorlevel 1 (
  start "InMoov voice server" cmd /k voice\venv\Scripts\python.exe voice\server.py
) else (
  echo        already running - skipped
)

echo [4/4] Opening the orb page...
timeout /t 4 /nobreak >nul
start "" http://127.0.0.1:8765/orb.html

echo.
echo The voice server needs ~30 seconds to load its models the first time.
echo The page will say "voice server offline" until then - it connects by itself.
echo.
echo KEEP the "InMoov voice server" and "InMoov UI server" windows OPEN.
echo Closing them stops everything.
timeout /t 10 >nul
