#ifndef _PHASHTABLE_H
#define _PHASHTABLE_H

#include <iostream>
#include <libpmemobj++/make_persistent.hpp>
#include <libpmemobj++/persistent_ptr.hpp>
#include <libpmemobj++/transaction.hpp>
#include <stdexcept>
#include "../pvector/pvector.h"
#include "../plist/plist.h"

using namespace pmem;
using namespace pmem::obj;

template <typename KEY_T, typename VAL_T>
struct pair {
    KEY_T key;
    VAL_T val;

    pair(KEY_T key_in, VAL_T val_in) { 
        key = key_in; 
        val = val_in;
    }
};

template<typename KEY_T, typename VAL_T, typename ROOT_T>
class phashtable {
private:
    pvector<plist<pair<KEY_T, VAL_T>, ROOT_T>, ROOT_T> data;

public:
};

#include "phashtable.hpp"

#endif