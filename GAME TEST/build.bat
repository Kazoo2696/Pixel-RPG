@echo off
setlocal
where cmake >nul 2>nul
if errorlevel 1 goto mingw
cmake -S . -B build
if errorlevel 1 exit /b 1
cmake --build build --config Release
if errorlevel 1 exit /b 1
if exist build\Release\LuminaQuest.exe (
  echo Built: build\Release\LuminaQuest.exe
) else (
  echo Built: build\LuminaQuest.exe
)
exit /b 0

:mingw
where g++ >nul 2>nul
if errorlevel 1 goto missing
g++ -std=c++17 -O2 -municode -mwindows src\main.cpp -lgdiplus -lgdi32 -luser32 -o LuminaQuest.exe
if errorlevel 1 exit /b 1
echo Built: LuminaQuest.exe
exit /b 0

:missing
echo No C++ toolchain found.
echo Install Visual Studio Build Tools with Desktop development with C++, or MinGW-w64.
exit /b 1
