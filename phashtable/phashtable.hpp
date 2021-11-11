#include "phashtable.h"

/* ========================================================================= */
/* ***************************** phashtable ******************************** */
/* ========================================================================= */

/* ============================ CONSTRUCTORS =============================== */

// Construct a new, empty phashtable with default length.
template <typename KEY_T, typename VAL_T, typename ROOT_T>
phashtable<KEY_T, VAL_T, ROOT_T>::phashtable(pool<ROOT_T> pop_in) {
    pop = pop_in;

    flat_transaction::run(pop, [&] {
        hash_function = make_persistent<std::hash<KEY_T>>();
        len = default_capacity;
        data = make_persistent<pvector<plist<ppair<KEY_T, VAL_T>, ROOT_T>, ROOT_T>>(pop, len);

        // set all the lists to have the right pool reference too
        for (int i = 0; i < len; i++) {
            data[i].refresh_pool(pop);
        }
    });
}

/* ========================== OPERATOR OVERLOADS =========================== */

/* ============================== PUSH/POP ================================= */

/* =============================== GET/SET ================================= */

/* ================================ MISC. ================================== */

// Update the reference to the current pmem pool object.
template <typename KEY_T, typename VAL_T, typename ROOT_T>
void phashtable<KEY_T, VAL_T, ROOT_T>::refresh_pool(pool<ROOT_T> new_pop) {
    pop = new_pop;
} 

// Hash the given index, getting the index of the vector.
template <typename KEY_T, typename VAL_T, typename ROOT_T>
int phashtable<KEY_T, VAL_T, ROOT_T>::hash(const KEY_T& key) const {
    return hash_function(key) % len;
}

// Get the largest prime less than or equal to the given number.
template <typename KEY_T, typename VAL_T, typename ROOT_T>
unsigned long phashtable<KEY_T, VAL_T, ROOT_T>::prime_below(unsigned long n) {
    // error out on very small or very large values
    if (n > max_prime || n <= 1) {
        throw std::exception("Given value is too large or too small.");
    }

    // shortcut to this if the given value is known to be prime
    if (n == max_prime) {
        return max_prime;
    }

    std::vector<bool> v(n + 1);
    set_primes(v);

    while (n > 2) {
        if (v[n])
            return n;

        n--;
    }

    // if all else fails, return the smallest prime number
    return 2;
}

// Set the value to true for all indices that are prime numbers.
template <typename KEY_T, typename VAL_T, typename ROOT_T>
void phashtable<KEY_T, VAL_T, ROOT_T>::set_primes(std::vector<bool>& v) {
    v[0] = false;
    v[1] = false;

    int n = v.capacity();

    for (int i = 2; i < n; i++) {
        v[i] = true;
    }

    for (int i = 2; i * i < n; i++) {
        if (v[i]) {
            for (int j = i + i; j < n; j++) {
                v[j] = false;
            }
        }
    }
}