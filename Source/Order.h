// Order.h - A food delivery order and its life-cycle (state machine)
//   PLACED -> ASSIGNED -> PICKED_UP -> DELIVERED      (CANCELLED from any active state)
#ifndef ORDER_H
#define ORDER_H

#include "Record.h"
#include <string>

class Order : public Record {
public:
    enum class Status { Placed, Assigned, PickedUp, Delivered, Cancelled };

private:
    int         customerId;
    int         agentId;        // 0 = not assigned yet
    int         routeId;
    std::string items;
    double      foodAmount;
    double      deliveryFee;
    int         etaMinutes;
    int         customerRating; // 0 = not rated
    Status      status;
    std::string createdAt;

public:
    Order();
    Order(int id, int customerId, int routeId, const std::string& items,
          double foodAmount, const std::string& createdAt);
    ~Order();

    int    getCustomerId() const { return customerId; }
    int    getAgentId() const    { return agentId; }
    int    getRouteId() const    { return routeId; }
    const std::string& getItems() const     { return items; }
    double getFoodAmount() const { return foodAmount; }
    double getDeliveryFee() const { return deliveryFee; }
    double getTotal() const      { return foodAmount + deliveryFee; }
    int    getEta() const        { return etaMinutes; }
    int    getRating() const     { return customerRating; }
    Status getStatus() const     { return status; }
    const std::string& getCreatedAt() const { return createdAt; }
    bool   isActive() const;

    // ---- life-cycle transitions (throw OperationException when not allowed)
    void assignAgent(int agentId, double fee, int etaMinutes);
    void unassign();                        // used by data reconciliation
    void markPickedUp();
    void markDelivered(int rating);
    void cancel();

    static std::string statusToString(Status s);
    static Status      statusFromString(const std::string& s);
    std::string        statusText() const { return statusToString(status); }

    void        display() const override;
    std::string row() const override;
    std::string summary() const override;
    std::string toFileLine() const override;
    std::string typeName() const override { return "Order"; }

    static std::string header();
    static Order fromFileLine(const std::string& line);
};

#endif
