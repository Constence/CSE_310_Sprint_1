@echo off
echo Building Booked backend...
g++ -std=c++17 main.cpp -o booked_server.exe -lws2_32

if %errorlevel% neq 0 (
  echo.
  echo Build failed. Make sure MSYS2 MinGW g++ is installed and added to PATH.
  pause
  exit /b
)

echo.
echo Starting Booked backend...
booked_server.exe
pause
