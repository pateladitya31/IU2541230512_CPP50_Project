#include "Customer.h"
#include "Utils.h"

#include <iostream>

Customer::Customer() : Person(), loyaltyPoints(0), ordersPlaced(0) {}

Customer::Customer(int id_, const std::string& name_, const std::string& phone_,
                   const std::string& address_, int points, int orders)
    : Person(id_, name_, phone_, address_), loyaltyPoints(0), ordersPlaced(0) {
    if (points < 0 || orders < 0) throw ValidationException("Points and orders cannot be negative");
    loyaltyPoints = points;
    ordersPlaced = orders;
}

Customer::~Customer() {}

void Customer::addPoints(int pts) {
    if (pts < 0) throw ValidationException("Points to add cannot be negative");
    loyaltyPoints += pts;
}

std::string Customer::tier() const {
    if (loyaltyPoints >= 300) return "Gold";
    if (loyaltyPoints >= 100) return "Silver";
    return "Bronze";
}

void Customer::display() const {
    std::cout << "\n  ---------- CUSTOMER ----------\n";
    Utils::field("ID", Utils::toStr(id));
    Utils::field("Name", name);
    Utils::field("Phone", phone);
    Utils::field("Address", address);
    Utils::field("Orders placed", Utils::toStr(ordersPlaced));
    Utils::field("Loyalty", Utils::toStr(loyaltyPoints) + " pts (" + tier() + ")");
}

std::string Customer::header() {
    return Utils::pad("ID", 6) + " " + Utils::pad("Name", 22) + " " + Utils::pad("Phone", 11) + " " +
           Utils::pad("Address", 24) + " " + Utils::pad("Points", 6) + " " + Utils::pad("Orders", 6) + " Tier";
}

std::string Customer::row() const {
    return Utils::pad(Utils::toStr(id), 6) + " " + Utils::pad(name, 22) + " " + Utils::pad(phone, 11) + " " +
           Utils::pad(address, 24) + " " + Utils::pad(Utils::toStr(loyaltyPoints), 6) + " " +
           Utils::pad(Utils::toStr(ordersPlaced), 6) + " " + tier();
}

std::string Customer::summary() const {
    return "Customer #" + Utils::toStr(id) + " " + name + " (" + tier() + ", " +
           Utils::toStr(loyaltyPoints) + " pts)";
}

std::string Customer::toFileLine() const {
    return Utils::toStr(id) + "|" + name + "|" + phone + "|" + address + "|" +
           Utils::toStr(loyaltyPoints) + "|" + Utils::toStr(ordersPlaced);
}

Customer Customer::fromFileLine(const std::string& line) {
    std::vector<std::string> f = Utils::split(line, '|');
    if (f.size() != 6) throw ValidationException("expected 6 fields, found " + Utils::toStr(f.size()));
    return Customer(Utils::toInt(f[0], "ID"), f[1], f[2], f[3],
                    Utils::toInt(f[4], "Points"), Utils::toInt(f[5], "Orders"));
}
