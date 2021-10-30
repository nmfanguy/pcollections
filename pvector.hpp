#include "pvector.h"

/* ========================================================================= */
/* ******************************* pvector ********************************* */
/* ========================================================================= */

/* ============================ CONSTRUCTORS =============================== */

// Create a new, empty pvector with the given capacity.
template <typename VAL_T, typename ROOT_T>
pvector<VAL_T, ROOT_T>::pvector(pool<ROOT_T> pop_in, int capacity) {
    pop = pop_in;

    // edit pmem with a transaction
    flat_transaction::run(pop, [&] {
        arr = make_persistent<VAL_T[]>(capacity);
        len = 0;
        cap = capacity;
    });
}

/* ========================== OPERATOR OVERLOADS =========================== */

// Print this vector to the given output stream.
template <typename VAL_T, typename ROOT_T>
std::ostream& operator<<(std::ostream& os, const pvector<VAL_T, ROOT_T>& v) {
    os << "[";

    for (int i = 0; i < v.len - 1; i++) {
        os << v.arr[i] << ", ";
    }

    if (v.len > 0)
        os << v.arr[v.len - 1];

    os << "]";

    return os;
}

/* ============================== PUSH/POP ================================= */

// Insert the given value into the vector at the given index.
template <typename VAL_T, typename ROOT_T>
void pvector<VAL_T, ROOT_T>::insert(const VAL_T& val, int idx) {
    flat_transaction::run(pop, [&] {
        if (idx < cap) {
            arr[idx] = val;
        }
        else {
            // reallocate and such
            resize(cap + 1);
            arr[idx] = val;
        }

        len++;
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

// Resize the underlying array to the new given capacity. If the given capacity is less than the
// current length, values will be lost.
template <typename VAL_T, typename ROOT_T>
void pvector<VAL_T, ROOT_T>::resize(int new_cap) {
    // we will allocate & free pmem, so use a transaction
    flat_transaction::run(pop, [&] {
        // allocate the new array w/ appropriate capacity
        persistent_ptr<VAL_T[]> new_arr = make_persistent<VAL_T[]>(new_cap);

        // move all the items over
        for (int i = 0; i < len && i < new_cap; i++) {
            new_arr[i] = arr[i];
        }

        // delete the old array
        delete_persistent<VAL_T[]>(arr, cap);

        // set our array to the new one
        arr = new_arr;

        // update the length if we shrunk the vector
        if (len > new_cap)
            len = new_cap;

        // and always update the capacity
        cap = new_cap;
    });
}