#include "pvector.h"

/* ========================================================================= */
/* ******************************* pvector ********************************* */
/* ========================================================================= */

/* ============================ CONSTRUCTORS =============================== */

// Create a new, empty pvector.
template <typename VAL_T, typename ROOT_T>
pvector<VAL_T, ROOT_T>::pvector(pool<ROOT_T> pop_in) {
    pop = pop_in;

    // edit the pmem with a transaction
    flat_transaction::run(pop, [&] {
        arr = make_persistent<p<VAL_T>[]>();
        len = 0;
        cap = 0;
    });
}

// Create a new, empty pvector with the given capacity.
template <typename VAL_T, typename ROOT_T>
pvector<VAL_T, ROOT_T>::pvector(pool<ROOT_T> pop_in, int capacity) {
    pop = pop_in;

    // edit pmem with a transaction
    flat_transaction::run(pop, [&] {
        arr = make_persistent<p<VAL_T>[capacity]>();
        len = 0;
        cap = capacity;
    });
}

/* ================================ MISC. ================================== */

// Refresh the reference to the pool that this vector lives in. Must be called
// when using a pvector from an existing file (e.g. not just created at runtime).
template <typename VAL_T, typename ROOT_T>
void pvector<VAL_T, ROOT_T>::refresh_pool(pool<ROOT_T> new_pop) {
    flat_transaction::run(new_pop, [&] {
        pop = new_pop;
    });
}