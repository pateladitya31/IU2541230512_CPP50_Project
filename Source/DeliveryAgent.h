// DeliveryAgent.h - Abstract delivery agent + three concrete vehicle types.
// Runtime polymorphism: fee and travel-time rules are virtual and differ per vehicle.
#ifndef DELIVERYAGENT_H
#define DELIVERYAGENT_H

#include "Person.h"

class DeliveryAgent : public Person {
protected:
    bool   available;
    double rating;          // running average of customer ratings (1-5)
    int    deliveriesDone;

public:
    static const double DEFAULT_RATING;

    DeliveryAgent();
    DeliveryAgent(int id, const std::string& name, const std::string& phone,
                  const std::string& baseArea, bool available = true,
                  double rating = 4.0, int deliveries = 0);
    virtual ~DeliveryAgent();

    // ---- polymorphic behaviour (overridden by each vehicle type)
    virtual double      calculateFee(double km) const = 0;
    virtual double      travelMinutes(double km) const = 0;
    virtual char        vehicleCode() const = 0;
    virtual std::string vehicleName() const = 0;

    bool   isAvailable() const    { return available; }
    double getRating() const      { return rating; }
    int    getDeliveries() const  { return deliveriesDone; }
    void   setAvailable(bool v)   { available = v; }
    void   recordDelivery(int customerRating);   // updates running average

    void        display() const override;
    std::string row() const override;
    std::string summary() const override;
    std::string toFileLine() const override;
    std::string typeName() const override { return "Delivery Agent"; }

    static std::string header();

    // Factory: builds the right derived object from a vehicle code (B / S / C)
    static DeliveryAgent* create(char code, int id, const std::string& name, const std::string& phone,
                                 const std::string& baseArea, bool available, double rating, int deliveries);
    static DeliveryAgent* fromFileLine(const std::string& line);
    static char codeFromChoice(int choice);      // 1->B, 2->S, 3->C
};

class BikeAgent : public DeliveryAgent {
public:
    BikeAgent(int id, const std::string& name, const std::string& phone, const std::string& area,
              bool available = true, double rating = 4.0, int deliveries = 0);
    ~BikeAgent();
    double      calculateFee(double km) const override;     // Rs.20 + Rs.6/km
    double      travelMinutes(double km) const override;    // 22 km/h
    char        vehicleCode() const override { return 'B'; }
    std::string vehicleName() const override { return "Bike"; }
};

class ScooterAgent : public DeliveryAgent {
public:
    ScooterAgent(int id, const std::string& name, const std::string& phone, const std::string& area,
                 bool available = true, double rating = 4.0, int deliveries = 0);
    ~ScooterAgent();
    double      calculateFee(double km) const override;     // Rs.25 + Rs.8/km
    double      travelMinutes(double km) const override;    // 28 km/h
    char        vehicleCode() const override { return 'S'; }
    std::string vehicleName() const override { return "Scooter"; }
};

class CarAgent : public DeliveryAgent {
public:
    CarAgent(int id, const std::string& name, const std::string& phone, const std::string& area,
             bool available = true, double rating = 4.0, int deliveries = 0);
    ~CarAgent();
    double      calculateFee(double km) const override;     // Rs.40 + Rs.12/km
    double      travelMinutes(double km) const override;    // 35 km/h
    char        vehicleCode() const override { return 'C'; }
    std::string vehicleName() const override { return "Car"; }
};

#endif
