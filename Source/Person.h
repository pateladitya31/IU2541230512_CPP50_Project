// Person.h - Abstract intermediate class shared by Customer and DeliveryAgent
#ifndef PERSON_H
#define PERSON_H

#include "Record.h"
#include <string>

class Person : public Record {
protected:
    std::string name;
    std::string phone;
    std::string address;

public:
    Person();
    Person(int id, const std::string& name, const std::string& phone, const std::string& address);
    virtual ~Person();

    const std::string& getName() const    { return name; }
    const std::string& getPhone() const   { return phone; }
    const std::string& getAddress() const { return address; }

    void setName(const std::string& v);       // validated setters (encapsulation)
    void setPhone(const std::string& v);
    void setAddress(const std::string& v);
};

#endif
