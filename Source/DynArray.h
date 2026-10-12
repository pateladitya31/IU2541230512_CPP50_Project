// DynArray.h - Generic dynamic array built on new[] / delete[]
// Purpose: store entities (Customer, Route, Order, DeliveryAgent*) without a fixed
// size limit. Capacity doubles when full. Follows the Rule of Three (deep copy).
#ifndef DYNARRAY_H
#define DYNARRAY_H

#include <stdexcept>

template <typename T>
class DynArray {
private:
    T*  data_;       // heap block:  T* items = new T[capacity]
    int size_;       // elements in use
    int capacity_;   // elements allocated

    void grow() {
        int newCap = capacity_ * 2;
        T* bigger = new T[newCap];
        for (int i = 0; i < size_; ++i) bigger[i] = data_[i];
        delete[] data_;                       // release the old block
        data_ = bigger;
        capacity_ = newCap;
    }

public:
    explicit DynArray(int initialCapacity = 4)
        : data_(nullptr), size_(0), capacity_(initialCapacity > 0 ? initialCapacity : 4) {
        data_ = new T[capacity_];
    }

    DynArray(const DynArray& other)
        : data_(nullptr), size_(other.size_), capacity_(other.capacity_) {
        data_ = new T[capacity_];
        for (int i = 0; i < size_; ++i) data_[i] = other.data_[i];
    }

    DynArray& operator=(const DynArray& other) {
        if (this != &other) {
            T* fresh = new T[other.capacity_];
            for (int i = 0; i < other.size_; ++i) fresh[i] = other.data_[i];
            delete[] data_;
            data_ = fresh;
            size_ = other.size_;
            capacity_ = other.capacity_;
        }
        return *this;
    }

    ~DynArray() { delete[] data_; }

    void add(const T& value) {
        if (size_ == capacity_) grow();
        data_[size_++] = value;
    }

    void removeAt(int index) {
        if (index < 0 || index >= size_) throw std::out_of_range("DynArray::removeAt index out of range");
        for (int i = index; i < size_ - 1; ++i) data_[i] = data_[i + 1];
        --size_;
    }

    T& operator[](int index) {
        if (index < 0 || index >= size_) throw std::out_of_range("DynArray index out of range");
        return data_[index];
    }
    const T& operator[](int index) const {
        if (index < 0 || index >= size_) throw std::out_of_range("DynArray index out of range");
        return data_[index];
    }

    int  size() const     { return size_; }
    int  capacity() const { return capacity_; }
    bool empty() const    { return size_ == 0; }
    void clear()          { size_ = 0; }   // keeps the allocated block for reuse
};

#endif
