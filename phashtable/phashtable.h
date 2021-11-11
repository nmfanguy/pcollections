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

static const unsigned int max_prime = 1301081;
static const unsigned int default_capacity = 11;

// the pair struct for a Key-Value pair that is the basis for the hash table
template <typename KEY_T, typename VAL_T>
struct ppair {
    p<KEY_T> key;
    p<VAL_T> val;
};

template <typename KEY_T, typename VAL_T>
class hashnode {
private:
    ppair<KEY_T, VAL_T> pair;
    persistent_ptr<hashnode<KEY_T, VAL_T>> next;
};

template <typename KEY_T, typename VAL_T, typename ROOT_T>
class hashlist {
private:
    persistent_ptr<hashnode<KEY_T, VAL_T>> head;
    persistent_ptr<hashnode<KEY_T, VAL_T>> tail;
    p<int> len;
    pool<ROOT_T> pop;

public:
    void refresh_pool(pool<ROOT_T> pop_in) {
        pop = pop_in;
    }
};

template<typename KEY_T, typename VAL_T, typename ROOT_T>
class phashtable {
private:
    //persistent_ptr<pvector<plist<ppair<KEY_T, VAL_T>, ROOT_T>, ROOT_T>> data;
    persistent_ptr<pvector<hashlist<KEY_T, VAL_T, ROOT_T>, ROOT_T>> data;
    p<int> len;
    pool<ROOT_T> pop;
    persistent_ptr<std::hash<KEY_T>> hash_function;

    // helper functions
    void rehash();
    int hash(const KEY_T&) const;
    unsigned long prime_below(unsigned long);
    void set_primes(std::vector<unsigned long>&);

public:
    // Constructors
    phashtable(pool<ROOT_T>);

    // Operator Overloads

    // Push/Pop

    // Get/Set
    int get_length() const;

    // Misc.
    void refresh_pool(pool<ROOT_T>);
    void destroy();
};

#include "phashtable.hpp"

#endif