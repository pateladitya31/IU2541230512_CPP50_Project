=====================================================================
 FOOD DELIVERY MANAGEMENT SYSTEM
 Enrollment No. : IU2541230512        Project Code : CPP50
 Language       : C++11 or newer (object-oriented, console application)
=====================================================================

1. WHAT THE PROJECT DOES
   A menu-driven system that manages customers, delivery agents, delivery
   routes and food orders. Placing an order automatically assigns the best
   available agent, calculates a vehicle-specific delivery fee and ETA, and
   tracks the order until it is delivered (with a rating) or cancelled.
   All data is stored in text files and survives closing the program.

2. FOLDER STRUCTURE
   Source/        All .h / .cpp files, Makefile, build.bat
   Data/          customers.txt, agents.txt, routes.txt, orders.txt (sample data)
   Diagrams/      Use Case, Class (full + 2 detail parts), Sequence 1 and 2 (PNG)
   Executable/    Compiled program (Project)
   Screenshots/   15 console screenshots of the running program
   README.txt     This file
   ProjectReport.pdf   Full project report (14 sections)

3. HOW TO BUILD
   Linux / macOS / MinGW with make:
        cd Source
        make                      -> creates ../Executable/Project

   Windows (MinGW-w64 g++ on PATH):
        cd Source
        build.bat                 -> creates ..\Executable\Project.exe

   Any platform (manual):
        cd Source
        g++ -std=c++11 -Wall -Wextra -O2 *.cpp -o ../Executable/Project

   Needs any C++11 compiler (GCC 4.8+/MinGW, Clang, MSVC 2015+). It does NOT
   use <filesystem>, so old MinGW / Dev-C++ compilers work. Compiles with
   zero warnings under -Wall -Wextra -pedantic (C++11, 14 and 17).

   NOTE: the Executable folder contains a Linux (x86-64) build made while
   preparing this submission. A Windows Project.exe is produced by running
   Source\build.bat on a Windows machine with MinGW-w64 (or Visual Studio
   Developer Prompt: cl /EHsc *.cpp /Fe:..\Executable\Project.exe).

4. HOW TO RUN
        cd Executable
        ./Project                 (Windows:  Project.exe)

   The program finds the Data folder automatically (./Data or ../Data) and
   creates it if missing. Optional switches:
        --data <folder>   use a different data folder
        --echo            echo typed input (used for scripted demos / screenshots)

5. MAIN MENU
   1 Add Record            Customer, Delivery Agent (Bike/Scooter/Car), Route
   2 Display Records       Tables + combined polymorphic overview
   3 Search Record         By ID / keyword / status / customer
   4 Update Record         Edit fields; change an agent's vehicle type
   5 Delete Record         With confirmation and business-rule checks
   6 Main Transaction      Place order, assign, pick up, deliver+rate, cancel, track
   7 Report                Revenue, status counts, top performers, agent table
   8 Save / Load           Write or reload all data files
   9 Exit                  Saves automatically and quits

6. SAMPLE DATA (Data folder)
   6 customers (IDs 1001-1006), 5 agents (2001-2005: 2 bikes, 2 scooters,
   1 car), 5 routes (3001-3005) and 7 orders (4001-4007) in different states.
   New IDs continue automatically (1007, 2006, 3006, 4008, ...).
   To start with an empty system, delete the four .txt files in Data.

7. OOP CONCEPTS (where to look)
   Inheritance            Record -> Person -> Customer / DeliveryAgent -> Bike/Scooter/CarAgent
                          Record -> Route, Order
   Runtime polymorphism   DeliveryAgent::calculateFee / travelMinutes (virtual);
                          Record* array in DeliverySystem::showCombinedOverview
   Dynamic memory         DynArray<T> (new[] / delete[]), new BikeAgent/..., Record** array
   Exceptions             Utils.h (AppException hierarchy), caught in main.cpp
   File handling          FileManager.h / .cpp (atomic save, validated load)

8. TESTING
   50 scripted test cases (every menu option, validation, error paths,
   persistence, corrupted files) - 50 passed. Memory safety was checked with
   AddressSanitizer + UndefinedBehaviorSanitizer: no errors, no leaks.
   See Section 12 of ProjectReport.pdf.

9. FILE FORMAT (pipe-separated, '#' lines are comments)
   customers.txt  id|name|phone|address|loyaltyPoints|ordersPlaced
   agents.txt     vehicle(B/S/C)|id|name|phone|baseArea|available|rating|deliveries
   routes.txt     id|source|destination|distanceKm|trafficFactor
   orders.txt     id|customerId|agentId|routeId|items|foodAmount|deliveryFee|status|createdAt|etaMin|rating
