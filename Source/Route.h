// Route.h - A delivery route between a pickup area and a drop area
#ifndef ROUTE_H
#define ROUTE_H

#include "Record.h"
#include <string>

class Route : public Record {
private:
    std::string source;
    std::string destination;
    double distanceKm;
    double trafficFactor;     // 1.0 = free flow, 2.0 = heavy traffic

public:
    Route();
    Route(int id, const std::string& source, const std::string& destination,
          double distanceKm, double trafficFactor);
    ~Route();

    const std::string& getSource() const      { return source; }
    const std::string& getDestination() const { return destination; }
    double getDistanceKm() const   { return distanceKm; }
    double getTrafficFactor() const { return trafficFactor; }

    void setSource(const std::string& v);
    void setDestination(const std::string& v);
    void setDistanceKm(double v);
    void setTrafficFactor(double v);

    std::string label() const { return source + " -> " + destination; }
    double adjustedMinutes(double baseMinutes) const { return baseMinutes * trafficFactor; }

    void        display() const override;
    std::string row() const override;
    std::string summary() const override;
    std::string toFileLine() const override;
    std::string typeName() const override { return "Route"; }

    static std::string header();
    static Route fromFileLine(const std::string& line);
};

#endif
