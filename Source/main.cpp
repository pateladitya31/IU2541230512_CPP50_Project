// main.cpp - Entry point. Creates the system, shows the menu and delegates to DeliverySystem.
// No business logic lives here.
#include "DeliverySystem.h"
#include "Utils.h"

#include <cstring>
#include <iostream>

static void showMenu() {
    std::cout << "\n==============================================\n"
              << "        FOOD DELIVERY MANAGEMENT SYSTEM\n"
              << "==============================================\n"
              << "   1. Add Record\n"
              << "   2. Display Records\n"
              << "   3. Search Record\n"
              << "   4. Update Record\n"
              << "   5. Delete Record\n"
              << "   6. Main Transaction (Order Processing)\n"
              << "   7. Report\n"
              << "   8. Save / Load\n"
              << "   9. Exit\n"
              << "----------------------------------------------\n";
}

int main(int argc, char* argv[]) {
    std::string dataDir;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--echo") == 0) Input::echo = true;            // echo input (demo/testing)
        else if (std::strcmp(argv[i], "--data") == 0 && i + 1 < argc) dataDir = argv[++i];
    }

    DeliverySystem* system = nullptr;
    try {
        system = new DeliverySystem(dataDir);
        std::cout << "Starting Food Delivery Management System...\n";
        system->start();                                   // load existing records

        bool running = true;
        while (running) {
            showMenu();
            int choice = Input::integer("Enter your choice (1-9): ", 1, 9);
            try {
                switch (choice) {
                    case 1: system->addRecord(); break;
                    case 2: system->displayRecords(); break;
                    case 3: system->searchRecord(); break;
                    case 4: system->updateRecord(); break;
                    case 5: system->deleteRecord(); break;
                    case 6: system->mainTransaction(); break;
                    case 7: system->report(); break;
                    case 8: system->saveLoadMenu(); break;
                    case 9: running = false; break;
                }
            } catch (const AppException& e) {              // validation / not-found / file errors
                std::cout << "  [Error] " << e.what() << "\n";
            } catch (const std::exception& e) {            // anything unexpected (e.g. bad_alloc)
                std::cout << "  [Unexpected error] " << e.what() << "\n";
            }
        }
        std::cout << "\nSaving data before exit...\n";
        system->shutdown();
        std::cout << "Thank you for using the Food Delivery Management System. Goodbye!\n";
    } catch (const InputEnded&) {                          // stdin closed: still save safely
        std::cout << "\nInput closed. Saving data before exit...\n";
        if (system) system->shutdown();
    } catch (const AppException& e) {
        std::cout << "Fatal error: " << e.what() << "\n";
        delete system;
        return 1;
    }
    delete system;                                          // destructor frees all agent objects
    return 0;
}
