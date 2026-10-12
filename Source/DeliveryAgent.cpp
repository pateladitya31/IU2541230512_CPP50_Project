#include "DeliveryAgent.h"
#include "Utils.h"

#include <iostream>

const double DeliveryAgent::DEFAULT_RATING = 4.0;

DeliveryAgent::DeliveryAgent() : Person(), available(true), rating(DEFAULT_RATING), deliveriesDone(0) {}

DeliveryAgent::DeliveryAgent(int id_, const std::string& name_, const std::string& phone_,
                             const std::string& area_, bool avail, double rate, int deliveries)
    : Person(id_, name_, phone_, area_), available(avail), rating(DEFAULT_RATING), deliveriesDone(0) {
    Validate::range(rate, 1.0, 5.0, "Rating");
    if (deliveries < 0) throw ValidationException("Deliveries cannot be negative");
    rating = rate;
    deliveriesDone = deliveries;
}

DeliveryAgent::~DeliveryAgent() {}

void DeliveryAgent::recordDelivery(int customerRating) {
    Validate::range(customerRating, 1, 5, "Rating");
    // the first rated delivery replaces the default rating; later ones are averaged
    rating = (rating * deliveriesDone + customerRating) / (deliveriesDone + 1);
    ++deliveriesDone;
}

void DeliveryAgent::display() const {
    std::cout << "\n  ------- DELIVERY AGENT -------\n";
    Utils::field("ID", Utils::toStr(id));
    Utils::field("Name", name);
    Utils::field("Phone", phone);
    Utils::field("Base area", address);
    Utils::field("Vehicle", vehicleName());
    Utils::field("Status", available ? "Available" : "Busy / Off duty");
    Utils::field("Rating", deliveriesDone == 0 ? "New (no rated deliveries)" : Utils::num(rating, 2) + " / 5");
    Utils::field("Deliveries", Utils::toStr(deliveriesDone));
    Utils::field("Fee model", "Rs." + Utils::num(calculateFee(0), 0) + " base + per-km charge (e.g. 5 km = " +
                               Utils::money(calculateFee(5)) + ")");
}

std::string DeliveryAgent::header() {
    return Utils::pad("ID", 6) + " " + Utils::pad("Name", 22) + " " + Utils::pad("Phone", 11) + " " +
           Utils::pad("Base Area", 18) + " " + Utils::pad("Vehicle", 8) + " " + Utils::pad("Status", 9) + " " +
           Utils::pad("Rating", 6) + " Trips";
}

std::string DeliveryAgent::row() const {
    return Utils::pad(Utils::toStr(id), 6) + " " + Utils::pad(name, 22) + " " + Utils::pad(phone, 11) + " " +
           Utils::pad(address, 18) + " " + Utils::pad(vehicleName(), 8) + " " +
           Utils::pad(available ? "Available" : "Busy", 9) + " " +
           Utils::pad(deliveriesDone == 0 ? "New" : Utils::num(rating, 2), 6) + " " + Utils::toStr(deliveriesDone);
}

std::string DeliveryAgent::summary() const {
    return "Agent #" + Utils::toStr(id) + " " + name + " (" + vehicleName() + ", " +
           (available ? "available" : "busy") + ")";
}

std::string DeliveryAgent::toFileLine() const {
    return std::string(1, vehicleCode()) + "|" + Utils::toStr(id) + "|" + name + "|" + phone + "|" +
           address + "|" + (available ? "1" : "0") + "|" + Utils::num(rating, 2) + "|" +
           Utils::toStr(deliveriesDone);
}

char DeliveryAgent::codeFromChoice(int choice) {
    switch (choice) {
        case 1: return 'B';
        case 2: return 'S';
        case 3: return 'C';
        default: throw ValidationException("Unknown vehicle choice");
    }
}

DeliveryAgent* DeliveryAgent::create(char code, int id, const std::string& name, const std::string& phone,
                                     const std::string& area, bool avail, double rate, int deliveries) {
    switch (code) {
        case 'B': return new BikeAgent(id, name, phone, area, avail, rate, deliveries);
        case 'S': return new ScooterAgent(id, name, phone, area, avail, rate, deliveries);
        case 'C': return new CarAgent(id, name, phone, area, avail, rate, deliveries);
        default:  throw ValidationException(std::string("unknown vehicle code '") + code + "'");
    }
}

DeliveryAgent* DeliveryAgent::fromFileLine(const std::string& line) {
    std::vector<std::string> f = Utils::split(line, '|');
    if (f.size() != 8) throw ValidationException("expected 8 fields, found " + Utils::toStr(f.size()));
    if (f[0].size() != 1) throw ValidationException("vehicle code must be one letter");
    if (f[5] != "0" && f[5] != "1") throw ValidationException("availability must be 0 or 1");
    return create(f[0][0], Utils::toInt(f[1], "ID"), f[2], f[3], f[4], f[5] == "1",
                  Utils::toDouble(f[6], "Rating"), Utils::toInt(f[7], "Deliveries"));
}

// ------------------------------------------------------------------ BikeAgent
BikeAgent::BikeAgent(int id, const std::string& n, const std::string& p, const std::string& a,
                     bool av, double r, int d) : DeliveryAgent(id, n, p, a, av, r, d) {}
BikeAgent::~BikeAgent() {}
double BikeAgent::calculateFee(double km) const   { return 20.0 + 6.0 * km; }
double BikeAgent::travelMinutes(double km) const  { return km / 22.0 * 60.0; }

// --------------------------------------------------------------- ScooterAgent
ScooterAgent::ScooterAgent(int id, const std::string& n, const std::string& p, const std::string& a,
                           bool av, double r, int d) : DeliveryAgent(id, n, p, a, av, r, d) {}
ScooterAgent::~ScooterAgent() {}
double ScooterAgent::calculateFee(double km) const  { return 25.0 + 8.0 * km; }
double ScooterAgent::travelMinutes(double km) const { return km / 28.0 * 60.0; }

// ------------------------------------------------------------------- CarAgent
CarAgent::CarAgent(int id, const std::string& n, const std::string& p, const std::string& a,
                   bool av, double r, int d) : DeliveryAgent(id, n, p, a, av, r, d) {}
CarAgent::~CarAgent() {}
double CarAgent::calculateFee(double km) const  { return 40.0 + 12.0 * km; }
double CarAgent::travelMinutes(double km) const { return km / 35.0 * 60.0; }
