#include "Person.h"
#include "Utils.h"

Person::Person() : Record(), name("Unknown"), phone("0000000000"), address("Not set") {}

Person::Person(int id_, const std::string& name_, const std::string& phone_, const std::string& address_)
    : Record(id_) {
    setName(name_);
    setPhone(phone_);
    setAddress(address_);
}

Person::~Person() {}

void Person::setName(const std::string& v)    { Validate::name(v); name = v; }
void Person::setPhone(const std::string& v)   { Validate::phone(v); phone = v; }
void Person::setAddress(const std::string& v) { Validate::text(v, "Address", 40); address = v; }
