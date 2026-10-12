#include "Record.h"
#include "Utils.h"

Record::Record() : id(0) {}

Record::Record(int id_) : id(0) { setId(id_); }

Record::~Record() {}

void Record::setId(int newId) {
    if (newId <= 0) throw ValidationException("ID must be a positive number");
    id = newId;
}
