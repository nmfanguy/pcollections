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

// Insert the given value at the back of the vector, reallocating if necessary.
template <typename VAL_T, typename ROOT_T>
void pvector<VAL_T, ROOT_T>::push_back(const VAL_T& val) {
    flat_transaction::run(pop, [&] {
        if (len < cap) {
            arr[len] = val;
        }
        else {
            // reallocate and such
            resize(cap + 1);
            arr[len] = val;
        }

        len++;
    });
}

// Remove and return the value at the back of the vector.
template <typename VAL_T, typename ROOT_T>
VAL_T pvector<VAL_T, ROOT_T>::pop_back() {
    if (len == 0)
        throw std::out_of_range("Cannot pop the back of an empty vector.");

    len--;
    return arr[len];
}

// Insert the given value at the given index, reallocating if necessary.
template <typename VAL_T, typename ROOT_T>
void pvector<VAL_T, ROOT_T>::insert(const VAL_T& val, int idx) {
    // we edit the pmem
    flat_transaction::run(pop, [&] {
        auto new_cap = cap;
        // get more space if we need it
        if (len >= cap) {
            new_cap++;
        }

        persistent_ptr<VAL_T[]> new_arr = make_persistent<VAL_T[]>(new_cap);

        int offset = 0;
        for (int i = 0; i < len + 1; i++) {
            if (i == idx) {
                new_arr[i] = val;
                offset = 1;
            }

            new_arr[i + offset] = arr[i];
        }

        len++;

        delete_persistent<VAL_T[]>(arr, cap);

        arr = new_arr;
    });
}

/* =============================== GET/SET ================================= */

// Get the length of the vector.
template <typename VAL_T, typename ROOT_T>
int pvector<VAL_T, ROOT_T>::get_length() const {
    return len;
}

// Get the capacity of the vector.
template <typename VAL_T, typename ROOT_T>
int pvector<VAL_T, ROOT_T>::get_capacity() const {
    return cap;
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

// Shrink the vector's capacity to its current size, removing unused allocated space.
template <typename VAL_T, typename ROOT_T>
void pvector<VAL_T, ROOT_T>::shrink() {
    resize(len);
}