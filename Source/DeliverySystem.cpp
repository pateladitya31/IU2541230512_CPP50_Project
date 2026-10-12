#include "DeliverySystem.h"
#include "Utils.h"

#include <cmath>
#include <iostream>
#include <sstream>
#include <vector>

using std::cout;

// ============================================================ construction
DeliverySystem::DeliverySystem(const std::string& dataDir)
    : customers(8), agents(8), routes(8), orders(16),
      files(dataDir.empty() ? FileManager::locateDataDir() : dataDir),
      nextCustomerId(1001), nextAgentId(2001), nextRouteId(3001), nextOrderId(4001), dirty(false) {}

DeliverySystem::~DeliverySystem() { clearAgents(); }

void DeliverySystem::clearAgents() {
    for (int i = 0; i < agents.size(); ++i) delete agents[i];   // each agent was created with new
    agents.clear();
}

void DeliverySystem::start() { loadAll(true); }

void DeliverySystem::shutdown() {
    try {
        saveAll();
    } catch (const AppException& e) {
        cout << "  [Error] Could not save on exit: " << e.what() << "\n";
    }
}

// ================================================================ lookups
int DeliverySystem::indexOfCustomer(int id) const {
    for (int i = 0; i < customers.size(); ++i) if (customers[i].getId() == id) return i;
    return -1;
}
int DeliverySystem::indexOfAgent(int id) const {
    for (int i = 0; i < agents.size(); ++i) if (agents[i]->getId() == id) return i;
    return -1;
}
int DeliverySystem::indexOfRoute(int id) const {
    for (int i = 0; i < routes.size(); ++i) if (routes[i].getId() == id) return i;
    return -1;
}
int DeliverySystem::indexOfOrder(int id) const {
    for (int i = 0; i < orders.size(); ++i) if (orders[i].getId() == id) return i;
    return -1;
}

Customer& DeliverySystem::getCustomer(int id) {
    int i = indexOfCustomer(id);
    if (i < 0) throw NotFoundException("Customer with ID " + Utils::toStr(id) + " was not found");
    return customers[i];
}
DeliveryAgent& DeliverySystem::getAgent(int id) {
    int i = indexOfAgent(id);
    if (i < 0) throw NotFoundException("Delivery agent with ID " + Utils::toStr(id) + " was not found");
    return *agents[i];
}
Route& DeliverySystem::getRoute(int id) {
    int i = indexOfRoute(id);
    if (i < 0) throw NotFoundException("Route with ID " + Utils::toStr(id) + " was not found");
    return routes[i];
}
Order& DeliverySystem::getOrder(int id) {
    int i = indexOfOrder(id);
    if (i < 0) throw NotFoundException("Order with ID " + Utils::toStr(id) + " was not found");
    return orders[i];
}

bool DeliverySystem::hasActiveOrderFor(char kind, int id) const {
    for (int i = 0; i < orders.size(); ++i) {
        const Order& o = orders[i];
        if (!o.isActive()) continue;
        if (kind == 'C' && o.getCustomerId() == id) return true;
        if (kind == 'A' && o.getAgentId() == id) return true;
        if (kind == 'R' && o.getRouteId() == id) return true;
    }
    return false;
}

// Best agent = available agent with the highest rating (ties: fewer deliveries, then lower ID)
DeliveryAgent* DeliverySystem::findBestAvailableAgent() const {
    DeliveryAgent* best = nullptr;
    for (int i = 0; i < agents.size(); ++i) {
        DeliveryAgent* a = agents[i];
        if (!a->isAvailable()) continue;
        if (best == nullptr || a->getRating() > best->getRating() ||
            (a->getRating() == best->getRating() && a->getDeliveries() < best->getDeliveries()))
            best = a;
    }
    return best;
}

// Polymorphic calls: calculateFee()/travelMinutes() resolve to the agent's real vehicle type.
void DeliverySystem::assignAgentToOrder(Order& order, DeliveryAgent& agent, const Route& route) {
    double fee = agent.calculateFee(route.getDistanceKm());
    double travel = route.adjustedMinutes(agent.travelMinutes(route.getDistanceKm()));
    int eta = PREP_MINUTES + static_cast<int>(std::ceil(travel));
    order.assignAgent(agent.getId(), fee, eta);
    agent.setAvailable(false);
}

// =============================================================== listing
void DeliverySystem::listCustomers() const {
    cout << "\n  " << Customer::header() << "\n  "; Utils::line('-', 86);
    if (customers.empty()) cout << "  (no customers)\n";
    for (int i = 0; i < customers.size(); ++i) cout << "  " << customers[i].row() << "\n";
    cout << "  Total: " << customers.size() << " customer(s)\n";
}
void DeliverySystem::listAgents() const {
    cout << "\n  " << DeliveryAgent::header() << "\n  "; Utils::line('-', 88);
    if (agents.empty()) cout << "  (no delivery agents)\n";
    for (int i = 0; i < agents.size(); ++i) cout << "  " << agents[i]->row() << "\n";   // virtual call
    cout << "  Total: " << agents.size() << " agent(s)\n";
}
void DeliverySystem::listRoutes() const {
    cout << "\n  " << Route::header() << "\n  "; Utils::line('-', 70);
    if (routes.empty()) cout << "  (no routes)\n";
    for (int i = 0; i < routes.size(); ++i) cout << "  " << routes[i].row() << "\n";
    cout << "  Total: " << routes.size() << " route(s)\n";
}
void DeliverySystem::listOrders() const {
    cout << "\n  " << Order::header() << "\n  "; Utils::line('-', 82);
    if (orders.empty()) cout << "  (no orders)\n";
    for (int i = 0; i < orders.size(); ++i) cout << "  " << orders[i].row() << "\n";
    cout << "  Total: " << orders.size() << " order(s)\n";
}

int DeliverySystem::listOrdersByStatus(Order::Status st, const std::string& title) const {
    int count = 0;
    cout << "\n  " << title << "\n  " << Order::header() << "\n  "; Utils::line('-', 82);
    for (int i = 0; i < orders.size(); ++i)
        if (orders[i].getStatus() == st) { cout << "  " << orders[i].row() << "\n"; ++count; }
    if (count == 0) cout << "  (none)\n";
    return count;
}

std::string DeliverySystem::describeCustomer(int id) const {
    int i = indexOfCustomer(id);
    return i < 0 ? Utils::toStr(id) + " (removed)" : Utils::toStr(id) + " - " + customers[i].getName();
}
std::string DeliverySystem::describeAgent(int id) const {
    if (id == 0) return "(not assigned)";
    int i = indexOfAgent(id);
    return i < 0 ? Utils::toStr(id) + " (removed)"
                 : Utils::toStr(id) + " - " + agents[i]->getName() + " [" + agents[i]->vehicleName() + "]";
}

void DeliverySystem::showOrderDetails(const Order& o) const {
    o.display();
    Utils::field("Customer", describeCustomer(o.getCustomerId()));
    Utils::field("Agent", describeAgent(o.getAgentId()));
    int r = indexOfRoute(o.getRouteId());
    Utils::field("Route", r < 0 ? Utils::toStr(o.getRouteId()) + " (removed)" : routes[r].label());
}

// Demonstrates runtime polymorphism: one Record* array holds four different object types.
void DeliverySystem::showCombinedOverview() const {
    int total = customers.size() + agents.size() + routes.size() + orders.size();
    if (total == 0) { cout << "\n  No records to show.\n"; return; }
    Record** all = new Record*[total];
    int n = 0;
    for (int i = 0; i < customers.size(); ++i) all[n++] = const_cast<Customer*>(&customers[i]);
    for (int i = 0; i < agents.size(); ++i)    all[n++] = agents[i];
    for (int i = 0; i < routes.size(); ++i)    all[n++] = const_cast<Route*>(&routes[i]);
    for (int i = 0; i < orders.size(); ++i)    all[n++] = const_cast<Order*>(&orders[i]);

    cout << "\n  COMBINED OVERVIEW (" << total << " records)\n  "; Utils::line('-', 76);
    for (int i = 0; i < total; ++i)
        cout << "  " << Utils::pad(all[i]->typeName(), 15) << all[i]->summary() << "\n";   // virtual dispatch
    delete[] all;
}

int DeliverySystem::chooseEntity(const std::string& action, bool includeOrders) {
    cout << "\n  Which record type do you want to " << action << "?\n"
         << "    1. Customer\n    2. Delivery Agent\n    3. Route\n";
    if (includeOrders) cout << "    4. Order\n";
    cout << "    0. Back\n";
    return Input::integer("  Choice: ", 0, includeOrders ? 4 : 3);
}

// ================================================================= ADD
void DeliverySystem::addRecord() {
    cout << "\n=== ADD RECORD ===";
    int c = chooseEntity("add", false);
    if (c == 1) addCustomer();
    else if (c == 2) addAgent();
    else if (c == 3) addRoute();
    if (c != 0) cout << "  (Orders are created through option 6 - Main Transaction.)\n";
}

void DeliverySystem::addCustomer() {
    std::string name = Input::name("  Customer name: ");
    std::string phone;
    while (true) {
        phone = Input::phone("  Phone (10 digits): ");
        bool dup = false;
        for (int i = 0; i < customers.size(); ++i) if (customers[i].getPhone() == phone) dup = true;
        if (!dup) break;
        cout << "  Invalid input: this phone number is already registered.\n";
    }
    std::string address = Input::text("  Address: ", 40);
    Customer c(nextCustomerId, name, phone, address);
    customers.add(c);
    ++nextCustomerId;
    dirty = true;
    cout << "  Customer added successfully with ID " << c.getId() << ".\n";
}

void DeliverySystem::addAgent() {
    std::string name = Input::name("  Agent name: ");
    std::string phone;
    while (true) {
        phone = Input::phone("  Phone (10 digits): ");
        bool dup = false;
        for (int i = 0; i < agents.size(); ++i) if (agents[i]->getPhone() == phone) dup = true;
        if (!dup) break;
        cout << "  Invalid input: this phone number is already registered.\n";
    }
    std::string area = Input::text("  Base area: ", 40);
    cout << "  Vehicle:  1. Bike   2. Scooter   3. Car\n";
    char code = DeliveryAgent::codeFromChoice(Input::integer("  Vehicle choice: ", 1, 3));
    DeliveryAgent* a = DeliveryAgent::create(code, nextAgentId, name, phone, area, true,
                                             DeliveryAgent::DEFAULT_RATING, 0);
    agents.add(a);
    ++nextAgentId;
    dirty = true;
    cout << "  Delivery agent added successfully with ID " << a->getId() << " (" << a->vehicleName() << ").\n";
}

void DeliverySystem::addRoute() {
    std::string src = Input::text("  Pickup area: ", 24);
    std::string dst = Input::text("  Drop area: ", 24);
    double km = Input::number("  Distance in km (0.5 - 50): ", 0.5, 50.0);
    double tf = Input::number("  Traffic factor (1.0 free - 3.0 heavy): ", 1.0, 3.0);
    Route r(nextRouteId, src, dst, km, tf);
    routes.add(r);
    ++nextRouteId;
    dirty = true;
    cout << "  Route added successfully with ID " << r.getId() << ".\n";
}

// =============================================================== DISPLAY
void DeliverySystem::displayRecords() {
    cout << "\n=== DISPLAY RECORDS ===\n"
         << "    1. Customers\n    2. Delivery Agents\n    3. Routes\n    4. Orders\n"
         << "    5. Combined overview (all record types)\n    0. Back\n";
    switch (Input::integer("  Choice: ", 0, 5)) {
        case 1: listCustomers(); break;
        case 2: listAgents(); break;
        case 3: listRoutes(); break;
        case 4: listOrders(); break;
        case 5: showCombinedOverview(); break;
        default: break;
    }
}

// =============================================================== SEARCH
void DeliverySystem::searchRecord() {
    cout << "\n=== SEARCH RECORD ===";
    switch (chooseEntity("search", true)) {
        case 1: searchCustomers(); break;
        case 2: searchAgents(); break;
        case 3: searchRoutes(); break;
        case 4: searchOrders(); break;
        default: break;
    }
}

void DeliverySystem::searchCustomers() {
    cout << "  Search by:  1. ID   2. Name / phone keyword\n";
    if (Input::integer("  Choice: ", 1, 2) == 1) {
        getCustomer(Input::integer("  Customer ID: ", 1, 999999)).display();
        return;
    }
    std::string key = Input::line("  Keyword: ");
    int found = 0;
    for (int i = 0; i < customers.size(); ++i)
        if (Utils::containsIgnoreCase(customers[i].getName(), key) || customers[i].getPhone().find(key) != std::string::npos) {
            customers[i].display(); ++found;
        }
    cout << (found ? "\n  " + Utils::toStr(found) + " match(es) found.\n" : "  No matching customer found.\n");
}

void DeliverySystem::searchAgents() {
    cout << "  Search by:  1. ID   2. Name / area / vehicle keyword\n";
    if (Input::integer("  Choice: ", 1, 2) == 1) {
        getAgent(Input::integer("  Agent ID: ", 1, 999999)).display();
        return;
    }
    std::string key = Input::line("  Keyword: ");
    int found = 0;
    for (int i = 0; i < agents.size(); ++i)
        if (Utils::containsIgnoreCase(agents[i]->getName(), key) || Utils::containsIgnoreCase(agents[i]->getAddress(), key) ||
            Utils::containsIgnoreCase(agents[i]->vehicleName(), key)) {
            agents[i]->display(); ++found;
        }
    cout << (found ? "\n  " + Utils::toStr(found) + " match(es) found.\n" : "  No matching agent found.\n");
}

void DeliverySystem::searchRoutes() {
    cout << "  Search by:  1. ID   2. Area keyword\n";
    if (Input::integer("  Choice: ", 1, 2) == 1) {
        getRoute(Input::integer("  Route ID: ", 1, 999999)).display();
        return;
    }
    std::string key = Input::line("  Keyword: ");
    int found = 0;
    for (int i = 0; i < routes.size(); ++i)
        if (Utils::containsIgnoreCase(routes[i].getSource(), key) || Utils::containsIgnoreCase(routes[i].getDestination(), key)) {
            routes[i].display(); ++found;
        }
    cout << (found ? "\n  " + Utils::toStr(found) + " match(es) found.\n" : "  No matching route found.\n");
}

void DeliverySystem::searchOrders() {
    cout << "  Search by:  1. Order ID   2. Status   3. Customer ID\n";
    int m = Input::integer("  Choice: ", 1, 3);
    if (m == 1) {
        showOrderDetails(getOrder(Input::integer("  Order ID: ", 1, 999999)));
    } else if (m == 2) {
        cout << "  1. PLACED  2. ASSIGNED  3. PICKED_UP  4. DELIVERED  5. CANCELLED\n";
        static const Order::Status all[5] = { Order::Status::Placed, Order::Status::Assigned, Order::Status::PickedUp,
                                              Order::Status::Delivered, Order::Status::Cancelled };
        Order::Status st = all[Input::integer("  Status: ", 1, 5) - 1];
        listOrdersByStatus(st, "Orders with status " + Order::statusToString(st));
    } else {
        int cid = Input::integer("  Customer ID: ", 1, 999999);
        int found = 0;
        cout << "\n  " << Order::header() << "\n  "; Utils::line('-', 82);
        for (int i = 0; i < orders.size(); ++i)
            if (orders[i].getCustomerId() == cid) { cout << "  " << orders[i].row() << "\n"; ++found; }
        cout << (found ? "  " + Utils::toStr(found) + " order(s) found.\n" : "  No orders for that customer.\n");
    }
}

// =============================================================== UPDATE
void DeliverySystem::updateRecord() {
    cout << "\n=== UPDATE RECORD ===";
    int c = chooseEntity("update", false);
    if (c == 1) updateCustomer();
    else if (c == 2) updateAgent();
    else if (c == 3) updateRoute();
    if (c != 0) cout << "  (Order status changes are done through option 6 - Main Transaction.)\n";
}

void DeliverySystem::updateCustomer() {
    if (customers.empty()) throw OperationException("There are no customers to update");
    listCustomers();
    Customer& c = getCustomer(Input::integer("  Customer ID to update: ", 1, 999999));
    cout << "  Field:  1. Name   2. Phone   3. Address\n";
    switch (Input::integer("  Choice: ", 1, 3)) {
        case 1: c.setName(Input::name("  New name: ")); break;
        case 2: {
            std::string p = Input::phone("  New phone: ");
            for (int i = 0; i < customers.size(); ++i)
                if (customers[i].getId() != c.getId() && customers[i].getPhone() == p)
                    throw OperationException("That phone number belongs to another customer");
            c.setPhone(p);
            break;
        }
        default: c.setAddress(Input::text("  New address: ", 40)); break;
    }
    dirty = true;
    cout << "  Customer updated successfully.";
    c.display();
}

void DeliverySystem::updateAgent() {
    if (agents.empty()) throw OperationException("There are no delivery agents to update");
    listAgents();
    int id = Input::integer("  Agent ID to update: ", 1, 999999);
    int idx = indexOfAgent(id);
    if (idx < 0) throw NotFoundException("Delivery agent with ID " + Utils::toStr(id) + " was not found");
    cout << "  Field:  1. Name   2. Phone   3. Base area   4. Vehicle type   5. Availability\n";
    int f = Input::integer("  Choice: ", 1, 5);
    if ((f == 4 || f == 5) && hasActiveOrderFor('A', id))
        throw OperationException("This agent is on an active order; try again after the delivery");
    DeliveryAgent* a = agents[idx];
    switch (f) {
        case 1: a->setName(Input::name("  New name: ")); break;
        case 2: {
            std::string p = Input::phone("  New phone: ");
            for (int i = 0; i < agents.size(); ++i)
                if (agents[i]->getId() != id && agents[i]->getPhone() == p)
                    throw OperationException("That phone number belongs to another agent");
            a->setPhone(p);
            break;
        }
        case 3: a->setAddress(Input::text("  New base area: ", 40)); break;
        case 4: {
            cout << "  Vehicle:  1. Bike   2. Scooter   3. Car\n";
            char code = DeliveryAgent::codeFromChoice(Input::integer("  Vehicle choice: ", 1, 3));
            if (code == a->vehicleCode()) throw OperationException("Agent already uses that vehicle");
            // the derived type changes, so build a new object, copy the data, delete the old one
            DeliveryAgent* fresh = DeliveryAgent::create(code, a->getId(), a->getName(), a->getPhone(),
                                                         a->getAddress(), a->isAvailable(), a->getRating(),
                                                         a->getDeliveries());
            delete a;
            agents[idx] = fresh;
            a = fresh;
            break;
        }
        default: a->setAvailable(Input::yesNo("  Mark agent as available for orders?")); break;
    }
    dirty = true;
    cout << "  Delivery agent updated successfully.";
    a->display();
}

void DeliverySystem::updateRoute() {
    if (routes.empty()) throw OperationException("There are no routes to update");
    listRoutes();
    Route& r = getRoute(Input::integer("  Route ID to update: ", 1, 999999));
    cout << "  Field:  1. Pickup area   2. Drop area   3. Distance   4. Traffic factor\n";
    switch (Input::integer("  Choice: ", 1, 4)) {
        case 1: r.setSource(Input::text("  New pickup area: ", 24)); break;
        case 2: r.setDestination(Input::text("  New drop area: ", 24)); break;
        case 3: r.setDistanceKm(Input::number("  New distance (km): ", 0.5, 50.0)); break;
        default: r.setTrafficFactor(Input::number("  New traffic factor: ", 1.0, 3.0)); break;
    }
    dirty = true;
    cout << "  Route updated successfully.";
    r.display();
}

// =============================================================== DELETE
void DeliverySystem::deleteRecord() {
    cout << "\n=== DELETE RECORD ===";
    switch (chooseEntity("delete", true)) {
        case 1: deleteCustomer(); break;
        case 2: deleteAgent(); break;
        case 3: deleteRoute(); break;
        case 4: deleteOrder(); break;
        default: break;
    }
}

void DeliverySystem::deleteCustomer() {
    if (customers.empty()) throw OperationException("There are no customers to delete");
    listCustomers();
    int id = Input::integer("  Customer ID to delete: ", 1, 999999);
    getCustomer(id);
    if (hasActiveOrderFor('C', id)) throw OperationException("Customer has an active order; cancel or complete it first");
    if (!Input::yesNo("  Delete customer " + describeCustomer(id) + "?")) { cout << "  Deletion cancelled.\n"; return; }
    customers.removeAt(indexOfCustomer(id));
    dirty = true;
    cout << "  Customer deleted successfully.\n";
}

void DeliverySystem::deleteAgent() {
    if (agents.empty()) throw OperationException("There are no delivery agents to delete");
    listAgents();
    int id = Input::integer("  Agent ID to delete: ", 1, 999999);
    getAgent(id);
    if (hasActiveOrderFor('A', id)) throw OperationException("Agent is on an active order; finish or cancel it first");
    if (!Input::yesNo("  Delete agent " + describeAgent(id) + "?")) { cout << "  Deletion cancelled.\n"; return; }
    int idx = indexOfAgent(id);
    delete agents[idx];            // free the heap object first ...
    agents.removeAt(idx);          // ... then remove the dangling pointer from the array
    dirty = true;
    cout << "  Delivery agent deleted successfully.\n";
}

void DeliverySystem::deleteRoute() {
    if (routes.empty()) throw OperationException("There are no routes to delete");
    listRoutes();
    int id = Input::integer("  Route ID to delete: ", 1, 999999);
    Route& r = getRoute(id);
    if (hasActiveOrderFor('R', id)) throw OperationException("Route is used by an active order; finish or cancel it first");
    if (!Input::yesNo("  Delete route " + r.label() + "?")) { cout << "  Deletion cancelled.\n"; return; }
    routes.removeAt(indexOfRoute(id));
    dirty = true;
    cout << "  Route deleted successfully.\n";
}

void DeliverySystem::deleteOrder() {
    if (orders.empty()) throw OperationException("There are no orders to delete");
    listOrders();
    int id = Input::integer("  Order ID to delete: ", 1, 999999);
    Order& o = getOrder(id);
    if (o.isActive()) throw OperationException("Order is still active; cancel it first (Main Transaction > Cancel)");
    if (!Input::yesNo("  Delete order " + Utils::toStr(id) + "?")) { cout << "  Deletion cancelled.\n"; return; }
    orders.removeAt(indexOfOrder(id));
    dirty = true;
    cout << "  Order deleted successfully.\n";
}

// ====================================================== MAIN TRANSACTION
void DeliverySystem::mainTransaction() {
    while (true) {
        cout << "\n=== MAIN TRANSACTION: ORDER PROCESSING ===\n"
             << "    1. Place new order (auto-assigns best available agent)\n"
             << "    2. Assign agent to a waiting order\n"
             << "    3. Mark order as picked up\n"
             << "    4. Mark order as delivered (+ rating)\n"
             << "    5. Cancel order\n"
             << "    6. Track an order\n"
             << "    0. Back to main menu\n";
        int c = Input::integer("  Choice: ", 0, 6);
        if (c == 0) return;
        try {
            switch (c) {
                case 1: placeOrder(); break;
                case 2: assignPendingOrder(); break;
                case 3: pickUpOrder(); break;
                case 4: deliverOrder(); break;
                case 5: cancelOrder(); break;
                default: trackOrder(); break;
            }
        } catch (const AppException& e) {
            cout << "  [Error] " << e.what() << "\n";
        }
    }
}

void DeliverySystem::placeOrder() {
    if (customers.empty()) throw OperationException("Add at least one customer first");
    if (routes.empty()) throw OperationException("Add at least one route first");
    listCustomers();
    Customer& cust = getCustomer(Input::integer("  Customer ID: ", 1, 999999));
    listRoutes();
    Route& route = getRoute(Input::integer("  Route ID: ", 1, 999999));
    std::string items = Input::text("  Items (e.g. 2x Paneer Wrap, 1x Lassi): ", 80);
    double amount = Input::number("  Food amount in Rs. (1 - 50000): ", 1.0, 50000.0);

    Order order(nextOrderId, cust.getId(), route.getId(), items, amount, Utils::now());
    DeliveryAgent* agent = findBestAvailableAgent();
    if (agent != nullptr) assignAgentToOrder(order, *agent, route);
    orders.add(order);
    ++nextOrderId;
    cust.incrementOrders();
    dirty = true;

    cout << "\n  Order " << order.getId() << " placed for " << cust.getName() << ".\n";
    if (agent != nullptr) {
        cout << "  Assigned to " << agent->getName() << " (" << agent->vehicleName() << ")\n"
             << "  Delivery fee : " << Utils::money(order.getDeliveryFee()) << "\n"
             << "  Order total  : " << Utils::money(order.getTotal()) << "\n"
             << "  Estimated delivery time: " << order.getEta() << " minutes\n";
    } else {
        cout << "  No agent is free right now. The order is PLACED and waiting - use option 2 to assign it later.\n";
    }
}

void DeliverySystem::assignPendingOrder() {
    if (listOrdersByStatus(Order::Status::Placed, "Orders waiting for an agent") == 0) return;
    Order& o = getOrder(Input::integer("  Order ID to assign: ", 1, 999999));
    if (o.getStatus() != Order::Status::Placed)
        throw OperationException("Order " + Utils::toStr(o.getId()) + " is " + o.statusText() + ", not waiting for an agent");
    DeliveryAgent* agent = findBestAvailableAgent();
    if (agent == nullptr) throw OperationException("No delivery agent is currently available");
    assignAgentToOrder(o, *agent, getRoute(o.getRouteId()));
    dirty = true;
    cout << "  Order " << o.getId() << " assigned to " << agent->getName() << " (" << agent->vehicleName()
         << "). Fee " << Utils::money(o.getDeliveryFee()) << ", total " << Utils::money(o.getTotal())
         << ", ETA " << o.getEta() << " min.\n";
}

void DeliverySystem::pickUpOrder() {
    if (listOrdersByStatus(Order::Status::Assigned, "Orders ready for pickup") == 0) return;
    Order& o = getOrder(Input::integer("  Order ID picked up: ", 1, 999999));
    o.markPickedUp();
    dirty = true;
    cout << "  Order " << o.getId() << " is now PICKED_UP and on its way.\n";
}

void DeliverySystem::deliverOrder() {
    if (listOrdersByStatus(Order::Status::PickedUp, "Orders out for delivery") == 0) return;
    Order& o = getOrder(Input::integer("  Order ID delivered: ", 1, 999999));
    if (o.getStatus() != Order::Status::PickedUp)
        throw OperationException("Order " + Utils::toStr(o.getId()) + " is " + o.statusText() + ", not out for delivery");
    int rating = Input::integer("  Customer rating for the delivery (1-5): ", 1, 5);
    o.markDelivered(rating);

    int ai = indexOfAgent(o.getAgentId());
    if (ai >= 0) { agents[ai]->recordDelivery(rating); agents[ai]->setAvailable(true); }
    int points = static_cast<int>(o.getTotal() / 10.0);
    int ci = indexOfCustomer(o.getCustomerId());
    if (ci >= 0) customers[ci].addPoints(points);
    dirty = true;

    cout << "  Order " << o.getId() << " DELIVERED. Total collected: " << Utils::money(o.getTotal()) << "\n";
    if (ai >= 0) cout << "  Agent " << agents[ai]->getName() << " is available again (rating now "
                      << Utils::num(agents[ai]->getRating(), 2) << ").\n";
    if (ci >= 0) cout << "  Customer " << customers[ci].getName() << " earned " << points << " loyalty points.\n";
}

void DeliverySystem::cancelOrder() {
    int count = 0;
    cout << "\n  Active orders\n  " << Order::header() << "\n  "; Utils::line('-', 82);
    for (int i = 0; i < orders.size(); ++i)
        if (orders[i].isActive()) { cout << "  " << orders[i].row() << "\n"; ++count; }
    if (count == 0) { cout << "  (none)\n"; return; }
    Order& o = getOrder(Input::integer("  Order ID to cancel: ", 1, 999999));
    if (!o.isActive())
        throw OperationException("Order " + Utils::toStr(o.getId()) + " is already " + o.statusText());
    if (!Input::yesNo("  Cancel order " + Utils::toStr(o.getId()) + "?")) { cout << "  Cancellation aborted.\n"; return; }
    o.cancel();
    int ai = indexOfAgent(o.getAgentId());
    if (ai >= 0) agents[ai]->setAvailable(true);       // release the agent
    dirty = true;
    cout << "  Order " << o.getId() << " cancelled" << (ai >= 0 ? " and the agent has been released.\n" : ".\n");
}

void DeliverySystem::trackOrder() {
    if (orders.empty()) throw OperationException("There are no orders yet");
    showOrderDetails(getOrder(Input::integer("  Order ID to track: ", 1, 999999)));
}

// ================================================================ REPORT
std::string DeliverySystem::buildReport() const {
    std::ostringstream o;
    int cnt[5] = {0, 0, 0, 0, 0};
    double food = 0, fees = 0, cancelledValue = 0, ratingSum = 0, etaSum = 0;
    int delivered = 0, rated = 0;
    std::vector<double> spend(customers.size(), 0.0);
    std::vector<double> earn(agents.size(), 0.0);
    std::vector<int> routeUse(routes.size(), 0);

    for (int i = 0; i < orders.size(); ++i) {
        const Order& ord = orders[i];
        cnt[static_cast<int>(ord.getStatus())]++;
        int ri = indexOfRoute(ord.getRouteId());
        if (ri >= 0) routeUse[ri]++;
        if (ord.getStatus() == Order::Status::Delivered) {
            ++delivered;
            food += ord.getFoodAmount();
            fees += ord.getDeliveryFee();
            etaSum += ord.getEta();
            if (ord.getRating() > 0) { ratingSum += ord.getRating(); ++rated; }
            int ci = indexOfCustomer(ord.getCustomerId());
            if (ci >= 0) spend[ci] += ord.getTotal();
            int ai = indexOfAgent(ord.getAgentId());
            if (ai >= 0) earn[ai] += ord.getDeliveryFee();
        } else if (ord.getStatus() == Order::Status::Cancelled) {
            cancelledValue += ord.getFoodAmount();
        }
    }
    int availableAgents = 0;
    for (int i = 0; i < agents.size(); ++i) if (agents[i]->isAvailable()) ++availableAgents;

    o << "==============================================================\n"
      << "        FOOD DELIVERY MANAGEMENT - SUMMARY REPORT\n"
      << "        Generated: " << Utils::now() << "\n"
      << "==============================================================\n\n"
      << "1. RECORDS\n"
      << "   Customers        : " << customers.size() << "\n"
      << "   Delivery agents  : " << agents.size() << " (" << availableAgents << " available, "
      << agents.size() - availableAgents << " busy/off duty)\n"
      << "   Routes           : " << routes.size() << "\n"
      << "   Orders           : " << orders.size() << "\n\n"
      << "2. ORDERS BY STATUS\n";
    for (int s = 0; s < 5; ++s)
        o << "   " << Utils::pad(Order::statusToString(static_cast<Order::Status>(s)), 10) << ": " << cnt[s] << "\n";

    o << "\n3. REVENUE (delivered orders only)\n"
      << "   Food sales       : " << Utils::money(food) << "\n"
      << "   Delivery fees    : " << Utils::money(fees) << "\n"
      << "   Total collected  : " << Utils::money(food + fees) << "\n"
      << "   Average order    : " << (delivered ? Utils::money((food + fees) / delivered) : std::string("n/a")) << "\n"
      << "   Value cancelled  : " << Utils::money(cancelledValue) << " (food amount)\n\n"
      << "4. SERVICE QUALITY\n"
      << "   Average rating   : " << (rated ? Utils::num(ratingSum / rated, 2) + " / 5" : std::string("n/a")) << "\n"
      << "   Average ETA      : " << (delivered ? Utils::num(etaSum / delivered, 1) + " min" : std::string("n/a")) << "\n\n"
      << "5. TOP PERFORMERS\n";

    int bc = -1; for (int i = 0; i < customers.size(); ++i) if (spend[i] > 0 && (bc < 0 || spend[i] > spend[bc])) bc = i;
    int ba = -1; for (int i = 0; i < agents.size(); ++i) if (agents[i]->getDeliveries() > 0 && (ba < 0 || agents[i]->getDeliveries() > agents[ba]->getDeliveries())) ba = i;
    int br = -1; for (int i = 0; i < routes.size(); ++i) if (routeUse[i] > 0 && (br < 0 || routeUse[i] > routeUse[br])) br = i;
    o << "   Top customer     : " << (bc >= 0 ? customers[bc].getName() + " (" + Utils::money(spend[bc]) + ")" : std::string("n/a")) << "\n"
      << "   Top agent        : " << (ba >= 0 ? agents[ba]->getName() + " (" + Utils::toStr(agents[ba]->getDeliveries()) + " deliveries)" : std::string("n/a")) << "\n"
      << "   Busiest route    : " << (br >= 0 ? routes[br].label() + " (" + Utils::toStr(routeUse[br]) + " orders)" : std::string("n/a")) << "\n\n"
      << "6. AGENT PERFORMANCE\n   "
      << Utils::pad("Agent", 22) << " " << Utils::pad("Vehicle", 8) << " " << Utils::pad("Trips", 5) << " "
      << Utils::pad("Rating", 6) << " Fees earned (listed deliveries)\n   " << std::string(66, '-') << "\n";
    if (agents.empty()) o << "   (no agents)\n";
    for (int i = 0; i < agents.size(); ++i)
        o << "   " << Utils::pad(agents[i]->getName(), 22) << " " << Utils::pad(agents[i]->vehicleName(), 8) << " "
          << Utils::pad(Utils::toStr(agents[i]->getDeliveries()), 5) << " "
          << Utils::pad(agents[i]->getDeliveries() ? Utils::num(agents[i]->getRating(), 2) : "New", 6) << " "
          << Utils::money(earn[i]) << "\n";
    o << "\n7. ORDERS NEEDING ATTENTION\n";
    int attention = 0;
    for (int i = 0; i < orders.size(); ++i)
        if (orders[i].isActive()) { o << "   " << orders[i].summary() << " - customer " << orders[i].getCustomerId() << "\n"; ++attention; }
    if (!attention) o << "   (no active orders)\n";
    o << "\n==============================================================\n";
    return o.str();
}

void DeliverySystem::report() {
    cout << "\n=== REPORT ===\n";
    std::string text = buildReport();
    cout << text;
    if (Input::yesNo("  Save this report to " + std::string(FileManager::REPORT_FILE) + " in the data folder?")) {
        files.writeText(FileManager::REPORT_FILE, text);
        cout << "  Report saved to " << files.getDataDir() << "/" << FileManager::REPORT_FILE << ".\n";
    }
}

// ============================================================ SAVE / LOAD
void DeliverySystem::saveLoadMenu() {
    cout << "\n=== SAVE / LOAD ===\n"
         << "    1. Save all records to disk\n    2. Load records from disk (discards unsaved changes)\n    0. Back\n";
    int c = Input::integer("  Choice: ", 0, 2);
    if (c == 1) saveAll();
    else if (c == 2) {
        if (dirty && !Input::yesNo("  You have unsaved changes that will be lost. Continue?")) {
            cout << "  Load cancelled.\n";
            return;
        }
        loadAll(true);
    }
}

void DeliverySystem::saveAll() {
    files.saveCustomers(customers);
    files.saveAgents(agents);
    files.saveRoutes(routes);
    files.saveOrders(orders);
    dirty = false;
    cout << "  Data saved to '" << files.getDataDir() << "': " << customers.size() << " customers, "
         << agents.size() << " agents, " << routes.size() << " routes, " << orders.size() << " orders.\n";
}

void DeliverySystem::loadAll(bool announce) {
    customers.clear();
    clearAgents();
    routes.clear();
    orders.clear();

    LoadResult rc = files.loadCustomers(customers);
    LoadResult ra = files.loadAgents(agents);
    LoadResult rr = files.loadRoutes(routes);
    LoadResult ro = files.loadOrders(orders);
    reconcile();
    recomputeIds();
    dirty = false;

    if (!announce) return;
    if (!rc.found && !ra.found && !rr.found && !ro.found) {
        cout << "  No saved data found in '" << files.getDataDir() << "' - starting with an empty system.\n";
        return;
    }
    cout << "  Loaded from '" << files.getDataDir() << "': " << rc.loaded << " customers, " << ra.loaded
         << " agents, " << rr.loaded << " routes, " << ro.loaded << " orders.\n";
    const LoadResult* all[4] = { &rc, &ra, &rr, &ro };
    for (int k = 0; k < 4; ++k)
        for (const std::string& w : all[k]->warnings) cout << "  [Warning] skipped invalid record - " << w << "\n";
}

// Repair cross-references after loading: orders whose agent vanished go back to PLACED,
// and agents that are on an active order are marked busy.
void DeliverySystem::reconcile() {
    for (int i = 0; i < orders.size(); ++i) {
        Order& o = orders[i];
        bool inFlight = (o.getStatus() == Order::Status::Assigned || o.getStatus() == Order::Status::PickedUp);
        if (inFlight && indexOfAgent(o.getAgentId()) < 0) {
            cout << "  [Note] Order " << o.getId() << " referenced a missing agent and was reset to PLACED.\n";
            o.unassign();
        }
    }
    for (int i = 0; i < agents.size(); ++i)
        if (hasActiveOrderFor('A', agents[i]->getId())) agents[i]->setAvailable(false);
}

void DeliverySystem::recomputeIds() {
    int mc = 1000, ma = 2000, mr = 3000, mo = 4000;
    for (int i = 0; i < customers.size(); ++i) if (customers[i].getId() > mc) mc = customers[i].getId();
    for (int i = 0; i < agents.size(); ++i)    if (agents[i]->getId() > ma) ma = agents[i]->getId();
    for (int i = 0; i < routes.size(); ++i)    if (routes[i].getId() > mr) mr = routes[i].getId();
    for (int i = 0; i < orders.size(); ++i)    if (orders[i].getId() > mo) mo = orders[i].getId();
    nextCustomerId = mc + 1; nextAgentId = ma + 1; nextRouteId = mr + 1; nextOrderId = mo + 1;
}
