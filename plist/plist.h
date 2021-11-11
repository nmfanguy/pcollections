#ifndef _PLIST_H
#define _PLIST_H

#include <libpmemobj++/make_persistent.hpp>
#include <libpmemobj++/persistent_ptr.hpp>
#include <libpmemobj++/transaction.hpp>
#include <iostream>
#include <stdexcept>

using namespace pmem;
using namespace pmem::obj;

// forward declaration of classes
template <typename VAL_T, typename ROOT_T>
class pnode;

template <typename VAL_T, typename ROOT_T>
class plist;

// we must declare this friend function ahead so the generics work as expected
template <typename VAL_T, typename ROOT_T>
std::ostream& operator<<(std::ostream&, const plist<VAL_T, ROOT_T>&);

template <typename VAL_T, typename ROOT_T>
class pnode {
private:
    p<VAL_T> val;
    persistent_ptr<pnode<VAL_T, ROOT_T>> next;
    pool<ROOT_T> pop;

public:
    // Constructor
    pnode(const VAL_T&, pool<ROOT_T>);

    // Getters/Setters
    void set_value(const VAL_T&);
    VAL_T get_value() const;
    void set_next(persistent_ptr<pnode<VAL_T, ROOT_T>>);
    persistent_ptr<pnode<VAL_T, ROOT_T>> get_next() const;

    // Misc.
    void refresh_pool(pool<ROOT_T>);
};

template <typename VAL_T, typename ROOT_T>
class plist {
private:
    persistent_ptr<pnode<VAL_T, ROOT_T>> head;
    persistent_ptr<pnode<VAL_T, ROOT_T>> tail;
    p<int> len;
    pool<ROOT_T> pop;

public:
    // Constructor
    explicit plist(pool<ROOT_T>);
    // required for phashtable implementation -- DO NOT CALL DIRECTLY
    plist() = default;

    // Operator Overloads
    VAL_T operator[](int) const;
    friend std::ostream& operator<< <>(std::ostream&, const plist<VAL_T, ROOT_T>&);

    // Push/Pop
    void push_back(const VAL_T&);
    VAL_T pop_back();

    void push_front(const VAL_T&);
    VAL_T pop_front();

    void insert(const VAL_T&, int);
    VAL_T remove(int);

    // Get/Set
    int get_length() const;
    bool is_empty() const;
    
    // Misc.
    void clear();
    void refresh_pool(pool<ROOT_T>);
    void destroy();
};

#include "plist.hpp"

#endif