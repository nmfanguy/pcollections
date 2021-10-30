// basic imports
#include <iostream>
#include <unistd.h>
// PMDK imports
#include <libpmemobj++/allocator.hpp>
#include <libpmemobj++/make_persistent.hpp>
#include <libpmemobj++/transaction.hpp>
// local collection imports
#include "plist.h"
#include "pvector.h"

#define POOLSIZE ((size_t)(1024 * 1024 * 256)) // 256 MB
#define PMFILE "pool"
#define LAYOUT "LISTPOOL"

using namespace pmem;
using namespace pmem::obj;
using namespace std;

class entry;

class root {
public:
    persistent_ptr<plist<int, root>> ilist;
    persistent_ptr<pvector<double, root>> dvec;
};

int main() {
    pool<root> pop;
    auto first_access = access(PMFILE, F_OK);
    
    // if first access, create the pool file
    if (first_access)
        pop = pool<root>::create(PMFILE, LAYOUT, POOLSIZE, S_IRWXU);
    // otherwise, access the existing pool file
    else
        pop = pool<root>::open(PMFILE, LAYOUT);

    // get the root of the pool
    auto proot = pop.root();

    // if the first access, populate the list & vector w/ items
    if (first_access) {
        flat_transaction::run(pop, [&] { 
            proot->ilist = make_persistent<plist<int, root>>(pop);
            proot->ilist->push_back(0);
            proot->ilist->push_back(1);
            proot->ilist->push_back(2);

            proot->dvec = make_persistent<pvector<double, root>>(pop, 5);
            proot->dvec->insert(2, 0);
            proot->dvec->insert(4, 1);
            proot->dvec->insert(6, 2);
            proot->dvec->insert(8, 3);
            proot->dvec->insert(10, 4);
            proot->dvec->insert(12, 5);
        });

        cout << ">>> LIST <<<" << endl << endl;
        cout << *(proot->ilist) << endl;

        cout << endl << ">>> VECTOR <<<" << endl << endl;
        cout << *(proot->dvec) << endl;
    }
    // otherwise, access the existing items and check function implementations
    else {
        proot->ilist->refresh_pool(pop);
        proot->dvec->refresh_pool(pop);

        cout << ">>> LIST <<<" << endl << endl;

        cout << "Before popping" << endl;
        cout << *(proot->ilist) << endl << endl;

        int val = proot->ilist->pop_back();

        cout << "After popping" << endl;
        cout << *(proot->ilist) << endl << endl;

        proot->ilist->push_back(val);

        cout << "After pushing" << endl;
        cout << *(proot->ilist) << endl << endl;

        proot->ilist->push_front(-1);

        cout << "After pushing front" << endl;
        cout << *(proot->ilist) << endl << endl;

        proot->ilist->pop_front();

        cout << "After popping front" << endl;
        cout << (*proot->ilist) << endl << endl;

        proot->ilist->insert(7, 2);

        cout << "After inserting middle" << endl;
        cout << (*proot->ilist) << endl << endl;
        
        proot->ilist->remove(2);

        cout << "After removing middle" << endl;
        cout << (*proot->ilist) << endl << endl;

        cout << endl << ">>> VECTOR <<<" << endl << endl;

        cout << "Original" << endl;
        cout << *(proot->dvec) << endl << endl;
    }

    return 0;
}
