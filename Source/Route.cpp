#include "Route.h"
#include "Utils.h"

#include <iostream>

Route::Route() : Record(), source("Unknown"), destination("Unknown"), distanceKm(1.0), trafficFactor(1.0) {}

Route::Route(int id_, const std::string& src, const std::string& dst, double km, double traffic)
    : Record(id_), source("Unknown"), destination("Unknown"), distanceKm(1.0), trafficFactor(1.0) {
    setSource(src);
    setDestination(dst);
    setDistanceKm(km);
    setTrafficFactor(traffic);
}

Route::~Route() {}

void Route::setSource(const std::string& v)      { Validate::text(v, "Source", 24); source = v; }
void Route::setDestination(const std::string& v) { Validate::text(v, "Destination", 24); destination = v; }
void Route::setDistanceKm(double v)    { Validate::range(v, 0.5, 50.0, "Distance (km)"); distanceKm = v; }
void Route::setTrafficFactor(double v) { Validate::range(v, 1.0, 3.0, "Traffic factor"); trafficFactor = v; }

void Route::display() const {
    std::cout << "\n  ------------ ROUTE ------------\n";
    Utils::field("ID", Utils::toStr(id));
    Utils::field("From", source);
    Utils::field("To", destination);
    Utils::field("Distance", Utils::num(distanceKm, 1) + " km");
    Utils::field("Traffic factor", Utils::num(trafficFactor, 2) + "x");
}

std::string Route::header() {
    return Utils::pad("ID", 6) + " " + Utils::pad("From", 24) + " " + Utils::pad("To", 24) + " " +
           Utils::pad("Km", 6) + " Traffic";
}

std::string Route::row() const {
    return Utils::pad(Utils::toStr(id), 6) + " " + Utils::pad(source, 24) + " " + Utils::pad(destination, 24) +
           " " + Utils::pad(Utils::num(distanceKm, 1), 6) + " " + Utils::num(trafficFactor, 2) + "x";
}

std::string Route::summary() const {
    return "Route #" + Utils::toStr(id) + " " + label() + " (" + Utils::num(distanceKm, 1) + " km)";
}

std::string Route::toFileLine() const {
    return Utils::toStr(id) + "|" + source + "|" + destination + "|" + Utils::num(distanceKm, 1) + "|" +
           Utils::num(trafficFactor, 2);
}

Route Route::fromFileLine(const std::string& line) {
    std::vector<std::string> f = Utils::split(line, '|');
    if (f.size() != 5) throw ValidationException("expected 5 fields, found " + Utils::toStr(f.size()));
    return Route(Utils::toInt(f[0], "ID"), f[1], f[2], Utils::toDouble(f[3], "Distance"),
                 Utils::toDouble(f[4], "Traffic"));
}
