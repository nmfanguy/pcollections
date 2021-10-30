#ifndef _PVECTOR_H
#define _PVECTOR_H

#include <libpmemobj++/make_persistent.hpp>
#include <libpmemobj++/persistent_ptr.hpp>
#include <libpmemobj++/transaction.hpp>
#include <iostream>
#include <stdexcept>

using namespace pmem;
using namespace pmem::obj;

template <typename VAL_T, typename ROOT_T>
class pvector {
private:
    persistent_ptr<p<VAL_T>[]> arr;
    p<int> len;
    p<int> cap;
    pool<ROOT_T> pop;

public:
    // Constructors
    explicit pvector(pool<ROOT_T>);
    pvector(pool<ROOT_T>, int);

    // Operator Overloads

    // Push/Pop

    // Get/Set

    // Misc.
    void refresh_pool(pool<ROOT_T>);
};

#include "pvector.hpp"

#endif
