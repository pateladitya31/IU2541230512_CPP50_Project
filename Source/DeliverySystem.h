// DeliverySystem.h - Controller class: owns all entities and implements every menu operation.
#ifndef DELIVERYSYSTEM_H
#define DELIVERYSYSTEM_H

#include "Customer.h"
#include "DeliveryAgent.h"
#include "DynArray.h"
#include "FileManager.h"
#include "Order.h"
#include "Route.h"

#include <string>

class DeliverySystem {
private:
    DynArray<Customer>        customers;   // new Customer[capacity]
    DynArray<DeliveryAgent*>  agents;      // array of base pointers -> polymorphic objects on the heap
    DynArray<Route>           routes;
    DynArray<Order>           orders;
    FileManager               files;

    int  nextCustomerId, nextAgentId, nextRouteId, nextOrderId;
    bool dirty;                            // unsaved changes exist

    static const int PREP_MINUTES = 10;    // restaurant preparation time added to every ETA

    // ---- lookup helpers (return -1 when not found / throw in the get* versions)
    int  indexOfCustomer(int id) const;
    int  indexOfAgent(int id) const;
    int  indexOfRoute(int id) const;
    int  indexOfOrder(int id) const;
    Customer&       getCustomer(int id);
    DeliveryAgent&  getAgent(int id);
    Route&          getRoute(int id);
    Order&          getOrder(int id);

    bool hasActiveOrderFor(char kind, int id) const;   // 'C'ustomer, 'A'gent, 'R'oute
    DeliveryAgent* findBestAvailableAgent() const;
    void assignAgentToOrder(Order& order, DeliveryAgent& agent, const Route& route);

    // ---- listing / display helpers
    void listCustomers() const;
    void listAgents() const;
    void listRoutes() const;
    void listOrders() const;
    int  listOrdersByStatus(Order::Status st, const std::string& title) const;
    void showOrderDetails(const Order& o) const;
    void showCombinedOverview() const;
    std::string describeCustomer(int id) const;
    std::string describeAgent(int id) const;

    int  chooseEntity(const std::string& action, bool includeOrders);

    // ---- sub-operations used by the public menu functions
    void addCustomer();
    void addAgent();
    void addRoute();
    void updateCustomer();
    void updateAgent();
    void updateRoute();
    void deleteCustomer();
    void deleteAgent();
    void deleteRoute();
    void deleteOrder();
    void searchCustomers();
    void searchAgents();
    void searchRoutes();
    void searchOrders();

    void placeOrder();
    void assignPendingOrder();
    void pickUpOrder();
    void deliverOrder();
    void cancelOrder();
    void trackOrder();

    void clearAgents();
    void reconcile();                       // repair links after loading from disk
    void recomputeIds();
    std::string buildReport() const;
    void loadAll(bool announce);
    void saveAll();

    DeliverySystem(const DeliverySystem&);               // non-copyable: owns raw agent pointers
    DeliverySystem& operator=(const DeliverySystem&);

public:
    explicit DeliverySystem(const std::string& dataDir = "");
    ~DeliverySystem();

    void start();            // load existing records
    void shutdown();         // save and release

    // ---- the 9 menu operations
    void addRecord();
    void displayRecords();
    void searchRecord();
    void updateRecord();
    void deleteRecord();
    void mainTransaction();
    void report();
    void saveLoadMenu();

    bool hasUnsavedChanges() const { return dirty; }
    const std::string& dataFolder() const { return files.getDataDir(); }
};

#endif
