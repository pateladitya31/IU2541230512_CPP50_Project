@echo off
REM Windows build script (needs MinGW-w64 g++ on PATH). Produces ..\Executable\Project.exe
if not exist ..\Executable mkdir ..\Executable
g++ -std=c++11 -Wall -Wextra -O2 main.cpp DeliverySystem.cpp FileManager.cpp Order.cpp Route.cpp DeliveryAgent.cpp Customer.cpp Person.cpp Record.cpp Utils.cpp -o ..\Executable\Project.exe
if %errorlevel% neq 0 (echo BUILD FAILED) else (echo BUILD OK - run ..\Executable\Project.exe)
pause
