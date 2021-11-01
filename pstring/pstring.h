#ifndef _PSTRING_H
#define _PSTRING_

#include <iostream>
#include <libpmemobj++/make_persistent.hpp>
#include <libpmemobj++/persistent_ptr.hpp>
#include <libpmemobj++/transaction.hpp>
#include <stdexcept>

using namespace pmem;
using namespace pmem::obj;

// forward declaration
template <typename ROOT_T>
class pstring;

// forward declare the friend function so generics work
template <typename ROOT_T>
std::ostream& operator<<(std::ostream&, const pstring<ROOT_T>&);

template <typename ROOT_T>
class pstring {
private:
    // will be a standard null-terminated C-style string
    persistent_ptr<char[]> arr;
    // number of actual characters
    p<int> len;
    // numbers of characters + \0 -- always len + 1
    p<int> cap;

    void resize(int);

public:
    // Constructors
    pstring(pool<ROOT_T>);
    pstring(pool<ROOT_T>, char*);
    pstring(pool<ROOT_T>, int);

    // Operator Overloads
    std::ostream& operator<< <>(std::ostream&, const pstring<ROOT_T>&);

};

#include "pstring.hpp"

#endif