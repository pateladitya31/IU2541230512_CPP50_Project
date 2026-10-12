// Customer.h - A person who places food orders
#ifndef CUSTOMER_H
#define CUSTOMER_H

#include "Person.h"

class Customer : public Person {
private:
    int loyaltyPoints;
    int ordersPlaced;

public:
    Customer();
    Customer(int id, const std::string& name, const std::string& phone, const std::string& address,
             int loyaltyPoints = 0, int ordersPlaced = 0);
    ~Customer();

    int getLoyaltyPoints() const { return loyaltyPoints; }
    int getOrdersPlaced() const  { return ordersPlaced; }
    void addPoints(int pts);
    void incrementOrders() { ++ordersPlaced; }
    std::string tier() const;                  // Bronze / Silver / Gold

    void        display() const override;
    std::string row() const override;
    std::string summary() const override;
    std::string toFileLine() const override;
    std::string typeName() const override { return "Customer"; }

    static std::string header();
    static Customer fromFileLine(const std::string& line);
};

#endif
