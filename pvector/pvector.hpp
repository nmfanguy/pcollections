#include "pvector.h"

/* ========================================================================= */
/* ******************************* pvector ********************************* */
/* ========================================================================= */

/* ============================ CONSTRUCTORS =============================== */

// Create a new, empty pvector with no capacity. 
template <typename VAL_T, typename ROOT_T>
pvector<VAL_T, ROOT_T>::pvector(pool<ROOT_T> pop_in) {
    pop = pop_in;

    // we have no capacity, so set to nullptr instead of allocating
    flat_transaction::run(pop, [&] {
        arr = nullptr;
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
    if (idx < 0 || idx > len) 
        throw std::out_of_range("Cannot insert past the range of the vector.");
    
    // we edit the pmem
    flat_transaction::run(pop, [&] {
        // resize array to 1 larger if we are at capacity
        if (len >= cap)
            resize(cap + 1);   

        // iterate backwards, moving items forward until we hit the target index
        for (int i = len; i >= 0; i--) {
            // if we have hit insertion index, set item to be the given value
            if (i == idx) {
                arr[i] = val;
                // we break as we do not need to edit the items before this in the list
                break;
            }
            // otherwise, make current item be the one that used to be one previous 
            else {
              arr[i] = arr[i - 1];
            }
        }

        // we inserted an item, so increase the length
        len++;
    });
}

// Remove the item at the given index and return the removed value.
template <typename VAL_T, typename ROOT_T>
VAL_T pvector<VAL_T, ROOT_T>::remove(int idx) {
    if (idx < 0 || idx >= len)
        throw std::out_of_range("Cannot remove past the range of the vector.");

    auto val = arr[idx];

    // we will edit the pmem
    flat_transaction::run(pop, [&] {
        len--;

        // starting at removal index, step over each item
        for (int i = idx; i < len; i++) {
            // move all items down one
            arr[i] = arr[i + 1];
        }
    });

    return val;
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

// Remove and deallocate all the items in the vector.
template <typename VAL_T, typename ROOT_T>
void pvector<VAL_T, ROOT_T>::clear() {
    flat_transaction::run(pop, [&] {
        delete_persistent<VAL_T[]>(arr, cap);

        arr = nullptr;
        len = 0;
        cap = 0;
    });
}

// Completely destroy this object and its allocated memory.
template <typename VAL_T, typename ROOT_T>
void pvector<VAL_T, ROOT_T>::destroy() {
    clear();

    flat_transaction::run(pop, [&] {
        delete_persistent<pvector<VAL_T, ROOT_T>>(this);
    });
}