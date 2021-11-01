#ifndef _PVECTOR_H
#define _PVECTOR_H

#include <libpmemobj++/make_persistent.hpp>
#include <libpmemobj++/persistent_ptr.hpp>
#include <libpmemobj++/transaction.hpp>
#include <iostream>
#include <stdexcept>

using namespace pmem;
using namespace pmem::obj;

// forward declare class
template <typename VAL_T, typename ROOT_T>
class pvector;

// forward declare the friend function so generics work
template <typename VAL_T, typename ROOT_T>
std::ostream& operator<<(std::ostream&, const pvector<VAL_T, ROOT_T>&);

template <typename VAL_T, typename ROOT_T>
class pvector {
private:
    persistent_ptr<VAL_T[]> arr;
    p<int> len;
    p<int> cap;
    pool<ROOT_T> pop;

    void resize(int);

public:
    // Constructors
    pvector(pool<ROOT_T>);
    pvector(pool<ROOT_T>, int);

    // Operator Overloads
    friend std::ostream& operator<< <>(std::ostream&, const pvector<VAL_T, ROOT_T>&);

    // Push/Pop
    void push_back(const VAL_T&);
    VAL_T pop_back();

    void insert(const VAL_T&, int);
    VAL_T remove(int);

    // Get/Set
    int get_length() const;
    int get_capacity() const;

    // Misc.
    void refresh_pool(pool<ROOT_T>);
    void shrink();
};

#include "pvector.hpp"

#endif
