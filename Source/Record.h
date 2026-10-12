// Record.h - Abstract base class for every stored entity (enables runtime polymorphism)
#ifndef RECORD_H
#define RECORD_H

#include <string>

class Record {
protected:
    int id;

public:
    Record();
    explicit Record(int id);
    virtual ~Record();                         // virtual: safe delete via base pointer

    int  getId() const { return id; }
    void setId(int newId);

    // Pure virtual interface - every entity must provide its own behaviour
    virtual void        display() const = 0;       // detailed card
    virtual std::string row() const = 0;           // one table row
    virtual std::string summary() const = 0;       // one-line description
    virtual std::string toFileLine() const = 0;    // '|' delimited persistence line
    virtual std::string typeName() const = 0;
};

#endif
