#include "pstring.h"

/* ========================================================================= */
/* ******************************* pstring ********************************* */
/* ========================================================================= */

/* ============================ CONSTRUCTORS =============================== */

// Create a new, empty pstring.
template <typename ROOT_T>
pstring<ROOT_T>::pstring(pool<ROOT_T> pop_in) {
    pop = pop_in;

    flat_transaction::run(pop, [&] {
        arr = nullptr;
        len = 0;
        cap = 0;
    });
}

// Create a new pstring of the given C-string.
template <typename ROOT_T>
pstring<ROOT_T>::pstring(pool<ROOT_T> pop_in, const char* str_in) {
    pop = pop_in;

    flat_transaction::run(pop, [&] {
        len = strlen(str_in);
        cap = len + 1;
        arr = make_persistent<char[]>(cap);

        // manually copy the chars over to pmem
        for (int i = 0; i < len; i++) {
            arr[i] = str_in[i];
        }
        arr[len] = '\0';
    });
}

/* ========================== OPERATOR OVERLOADS =========================== */

// Get the character at the given index.
template <typename ROOT_T>
char pstring<ROOT_T>::operator[](int idx) {
    if (idx < 0 || idx >= len)
        throw std::out_of_range("Cannot access character outside range of string");
    
    return arr[idx];
}

// Output the pstring to the given output stream.
template <typename ROOT_T>
std::ostream& operator<<(std::ostream& os, const pstring<ROOT_T>& ps) {
    for (int i = 0; i < ps.len; i++)
        os << ps.arr[i];

    return os;
}

// Concatenate the other string onto this one.
template <typename ROOT_T>
pstring<ROOT_T>& pstring<ROOT_T>::operator+=(const pstring<ROOT_T>& other) {
    // we edit the pmem
    flat_transaction::run(pop, [&] {
        // the new capacity is the two lens + '\0'
        auto new_cap = len + other.len + 1;

        // allocate new space for the bigger string
        auto new_str = make_persistent<char[]>(new_cap);

        // copy over the current array to the new one
        for (int i = 0; i < len; i++) {
            new_str[i] = arr[i];
        }

        // copy over the other array to the end of the new one
        for (int i = 0; i < other.len; i++) {
            new_str[i + len] = other.arr[i];
        }

        // insert the null-terminator 
        new_str[len] = '\0';

        // delete the old array
        delete_persistent<char[]>(arr, cap);
        
        // and update these values
        arr = new_str;
        len = new_cap - 1;
        cap = new_cap;
    });

    return *this;
}

/* =============================== GET/SET ================================= */

// Get the number of characters in the current pstring.
template <typename ROOT_T>
int pstring<ROOT_T>::get_length() const {
    return len;
}

// Get the size of the allocated storage for the pstring.
template <typename ROOT_T>
int pstring<ROOT_T>::get_capacity() const {
    return cap;
}

// Get whether or not the current pstring is empty.
template <typename ROOT_T>
bool pstring<ROOT_T>::is_empty() const {
    return len == 0;
}

/* ================================ MISC. ================================== */

// Refresh the reference to the pool that this object lives in. Must
// be done when loading this string from an existing pool.
template <typename ROOT_T>
void pstring<ROOT_T>::refresh_pool(pool<ROOT_T> new_pop) {
    flat_transaction::run(new_pop, [&] {
        pop = new_pop;
    });
}
