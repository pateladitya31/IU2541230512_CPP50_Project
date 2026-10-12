#include "Order.h"
#include "Utils.h"

#include <iostream>

Order::Order()
    : Record(), customerId(0), agentId(0), routeId(0), items("None"), foodAmount(0), deliveryFee(0),
      etaMinutes(0), customerRating(0), status(Status::Placed), createdAt("") {}

Order::Order(int id_, int cust, int route, const std::string& itemsText, double amount, const std::string& created)
    : Record(id_), customerId(cust), agentId(0), routeId(route), items("None"), foodAmount(0), deliveryFee(0),
      etaMinutes(0), customerRating(0), status(Status::Placed), createdAt(created) {
    if (cust <= 0 || route <= 0) throw ValidationException("Order needs a valid customer and route");
    Validate::text(itemsText, "Items", 80);
    Validate::range(amount, 1.0, 50000.0, "Food amount");
    items = itemsText;
    foodAmount = amount;
}

Order::~Order() {}

bool Order::isActive() const {
    return status == Status::Placed || status == Status::Assigned || status == Status::PickedUp;
}

void Order::assignAgent(int agent, double fee, int eta) {
    if (status != Status::Placed)
        throw OperationException("Only a PLACED order can be assigned (current status: " + statusText() + ")");
    if (agent <= 0) throw ValidationException("Invalid agent ID");
    agentId = agent;
    deliveryFee = fee;
    etaMinutes = eta;
    status = Status::Assigned;
}

void Order::unassign() {
    agentId = 0;
    deliveryFee = 0;
    etaMinutes = 0;
    status = Status::Placed;
}

void Order::markPickedUp() {
    if (status != Status::Assigned)
        throw OperationException("Only an ASSIGNED order can be picked up (current status: " + statusText() + ")");
    status = Status::PickedUp;
}

void Order::markDelivered(int rating) {
    if (status != Status::PickedUp)
        throw OperationException("Only a PICKED_UP order can be delivered (current status: " + statusText() + ")");
    Validate::range(rating, 1, 5, "Rating");
    customerRating = rating;
    status = Status::Delivered;
}

void Order::cancel() {
    if (!isActive())
        throw OperationException("Order is already " + statusText() + " and cannot be cancelled");
    status = Status::Cancelled;
}

std::string Order::statusToString(Status s) {
    switch (s) {
        case Status::Placed:    return "PLACED";
        case Status::Assigned:  return "ASSIGNED";
        case Status::PickedUp:  return "PICKED_UP";
        case Status::Delivered: return "DELIVERED";
        case Status::Cancelled: return "CANCELLED";
    }
    return "UNKNOWN";
}

Order::Status Order::statusFromString(const std::string& s) {
    if (s == "PLACED")    return Status::Placed;
    if (s == "ASSIGNED")  return Status::Assigned;
    if (s == "PICKED_UP") return Status::PickedUp;
    if (s == "DELIVERED") return Status::Delivered;
    if (s == "CANCELLED") return Status::Cancelled;
    throw ValidationException("unknown order status '" + s + "'");
}

void Order::display() const {
    std::cout << "\n  ------------ ORDER ------------\n";
    Utils::field("Order ID", Utils::toStr(id));
    Utils::field("Status", statusText());
    Utils::field("Created", createdAt);
    Utils::field("Customer ID", Utils::toStr(customerId));
    Utils::field("Agent ID", agentId == 0 ? "(not assigned)" : Utils::toStr(agentId));
    Utils::field("Route ID", Utils::toStr(routeId));
    Utils::field("Items", items);
    Utils::field("Food amount", Utils::money(foodAmount));
    Utils::field("Delivery fee", Utils::money(deliveryFee));
    Utils::field("Total", Utils::money(getTotal()));
    Utils::field("ETA", agentId == 0 ? "-" : Utils::toStr(etaMinutes) + " min");
    Utils::field("Rating", customerRating == 0 ? "-" : Utils::toStr(customerRating) + " / 5");
}

std::string Order::header() {
    return Utils::pad("ID", 6) + " " + Utils::pad("Cust", 6) + " " + Utils::pad("Agent", 6) + " " +
           Utils::pad("Route", 6) + " " + Utils::pad("Status", 10) + " " + Utils::pad("Food", 9) + " " +
           Utils::pad("Fee", 7) + " " + Utils::pad("Total", 9) + " Created";
}

std::string Order::row() const {
    return Utils::pad(Utils::toStr(id), 6) + " " + Utils::pad(Utils::toStr(customerId), 6) + " " +
           Utils::pad(agentId == 0 ? "-" : Utils::toStr(agentId), 6) + " " +
           Utils::pad(Utils::toStr(routeId), 6) + " " + Utils::pad(statusText(), 10) + " " +
           Utils::pad(Utils::num(foodAmount, 2), 9) + " " + Utils::pad(Utils::num(deliveryFee, 2), 7) + " " +
           Utils::pad(Utils::num(getTotal(), 2), 9) + " " + createdAt;
}

std::string Order::summary() const {
    return "Order #" + Utils::toStr(id) + " [" + statusText() + "] " + Utils::money(getTotal());
}

std::string Order::toFileLine() const {
    return Utils::toStr(id) + "|" + Utils::toStr(customerId) + "|" + Utils::toStr(agentId) + "|" +
           Utils::toStr(routeId) + "|" + items + "|" + Utils::num(foodAmount, 2) + "|" +
           Utils::num(deliveryFee, 2) + "|" + statusText() + "|" + createdAt + "|" +
           Utils::toStr(etaMinutes) + "|" + Utils::toStr(customerRating);
}

Order Order::fromFileLine(const std::string& line) {
    std::vector<std::string> f = Utils::split(line, '|');
    if (f.size() != 11) throw ValidationException("expected 11 fields, found " + Utils::toStr(f.size()));
    Order o(Utils::toInt(f[0], "ID"), Utils::toInt(f[1], "Customer ID"), Utils::toInt(f[3], "Route ID"),
            f[4], Utils::toDouble(f[5], "Food amount"), f[8]);
    int agent = Utils::toInt(f[2], "Agent ID");
    double fee = Utils::toDouble(f[6], "Fee");
    int eta = Utils::toInt(f[9], "ETA");
    int rating = Utils::toInt(f[10], "Rating");
    Status st = statusFromString(f[7]);
    if (agent < 0 || fee < 0 || eta < 0 || rating < 0 || rating > 5)
        throw ValidationException("negative or out-of-range numeric field");
    if (st != Status::Placed && agent == 0 && st != Status::Cancelled)
        throw ValidationException("order in status " + f[7] + " must have an agent");
    o.agentId = agent;
    o.deliveryFee = fee;
    o.etaMinutes = eta;
    o.customerRating = rating;
    o.status = st;
    return o;
}
