#include "plist.h"

/* ========================================================================= */
/* ******************************** pnode ********************************** */
/* ========================================================================= */

/* ============================ CONSTRUCTORS =============================== */

// Construct a new pnode with the given VAL_T value within the given pmem pool.
template <typename VAL_T, typename ROOT_T>
pnode<VAL_T, ROOT_T>::pnode(const VAL_T& val_in, pool<ROOT_T> pop_in) {
    // set this pnode's parent pool for later transactions to use
    pop = pop_in;

    // and safely edit the pmem of this pnode to have given values
    flat_transaction::run(pop, [&] {
        val = val_in;
        next = nullptr;
    });
}

/* ========================== GETTERS/SETTERS ============================== */

// Set the value of this pnode to the new VAL_T value.
template <typename VAL_T, typename ROOT_T>
void pnode<VAL_T, ROOT_T>::set_value(const VAL_T& new_val) {
    // editing pmem, so use a transaction
    flat_transaction::run(pop, [&] { 
        val = new_val;
    });
}

// Get the current value of this pnode.
template <typename VAL_T, typename ROOT_T>
VAL_T pnode<VAL_T, ROOT_T>::get_value() const {
    return val;
}

// Set the pointer to the next item in the plist to the given pointer.
template <typename VAL_T, typename ROOT_T>
void pnode<VAL_T, ROOT_T>::set_next(persistent_ptr<pnode<VAL_T, ROOT_T>> new_next) {
    // editing pmem, so use a transaction
    flat_transaction::run(pop, [&] { 
        next = new_next;
    });
}

// Get the current pointer to the next item in the plist.
template <typename VAL_T, typename ROOT_T>
persistent_ptr<pnode<VAL_T, ROOT_T>> pnode<VAL_T, ROOT_T>::get_next() const {
    return next;
}

/* ================================ MISC. ================================== */

// Refresh the current pool object that the pnode stores. Must be called when loading
// an existing pool file from disk.
template <typename VAL_T, typename ROOT_T>
void pnode<VAL_T, ROOT_T>::refresh_pool(pool<ROOT_T> new_pop) {
    // run a transaction in the new pool
    flat_transaction::run(new_pop, [&] {
        pop = new_pop;
    });
}

/* ========================================================================= */
/* ******************************** plist *********************************** */
/* ========================================================================= */

/* ============================ CONSTRUCTORS =============================== */

// Construct a new plist within the given pmem pool.
template <typename VAL_T, typename ROOT_T>
plist<VAL_T, ROOT_T>::plist(pool<ROOT_T> pop_in) {
    // set this plist's parent pool for later use
    pop = pop_in;

    // and edit the newly acquired pmem to default values
    flat_transaction::run(pop, [&] {
        head = nullptr;
        tail = nullptr;
        len = 0;
    });
}

/* ========================== OPERATOR OVERLOADS =========================== */

// Overload the [] operator to get the item at the given index.
template <typename VAL_T, typename ROOT_T>
VAL_T plist<VAL_T, ROOT_T>::operator[](int idx) const {
    if (idx >= len)
        throw std::out_of_range("Given index was greater than the size of the plist.");

    auto current = head;
    int i = 0;

    // step through all the pnodes until we hit target index
    while (i < idx && current != nullptr) {
        current = current->get_next();
        i++;
    }

    // return the value of the target index
    return current->get_value();
}

// Print the items in the plist to the given output stream, separated by commas.
template <typename VAL_T, typename ROOT_T>
std::ostream& operator<<(std::ostream& os, const plist<VAL_T, ROOT_T>& l) {
    os << "[";

    auto current = l.head;
    int i = 0;

    // step through the whole plist except the last element and print value plus comma
    while (i < l.len - 1 && current != nullptr) {
        os << current->get_value() << ", ";

        current = current->get_next();
        i++;
    }

    // do not print a comma after the last item
    if (l.len > 0) {
        os << current->get_value();
    }

    os << "]";

    return os;
}

/* ============================== PUSH/POP ================================= */

// Add the given value to the end of the plist.
template <typename VAL_T, typename ROOT_T>
void plist<VAL_T, ROOT_T>::push_back(const VAL_T& val) {
    // we are editing the actual memory, so run a transaction
    flat_transaction::run(pop, [&] {
        // allocate the new pnode
        auto n = make_persistent<pnode<VAL_T, ROOT_T>>(val, pop);

        // if this is the first push, the head & tail both point to the same pnode
        if (len == 0) {
            head = n;
            tail = n;
        }
        // otherwise, update the tail
        else {
            tail->set_next(n);
            tail = n;
        }

        len++;
    });
}

// Remove and return the value at the back of the plist.
template <typename VAL_T, typename ROOT_T>
VAL_T plist<VAL_T, ROOT_T>::pop_back() {
    if (len == 0)
        throw std::out_of_range("Cannot pop the back of an empty plist.");

    // store the return value upfront
    VAL_T val = tail->get_value();

    // set up iterator variables
    auto current = head;
    int i = 0;

    // we will be deleting some pmem, so use a transaction
    flat_transaction::run(pop, [&] {
        // step through the plist until we have hit second to last item
        while (i < len - 1 && current != nullptr) {
            current = current->get_next();
            i++;
        }

        // delete the old tail persistent memory
        delete_persistent<pnode<VAL_T, ROOT_T>>(tail);

        // update the tail to point to the correct pnode now
        tail = current;
        tail->set_next(nullptr);

        len--;
    });

    // return the value we pulled upfront
    return val;
}

// Add the given value to the front of the plist.
template <typename VAL_T, typename ROOT_T>
void plist<VAL_T, ROOT_T>::push_front(const VAL_T& val) {
    // editing memory, so run a transaction
    flat_transaction::run(pop, [&] {
        // allocate the new pnode
        auto n = make_persistent<pnode<VAL_T, ROOT_T>>(val, pop);

        // if this is the first push, head & tail both point to the same pnode
        if (len == 0) {
            head = n;
            tail = n;
        }
        // otherwise, update the head
        else {
            auto old_head = head;
            head = n;
            head->set_next(old_head);
        }

        len++;
    });
}

// Remove and return the value at the front of the plist.
template <typename VAL_T, typename ROOT_T>
VAL_T plist<VAL_T, ROOT_T>::pop_front() {
  if (len == 0)
    throw std::out_of_range("Cannot pop the front of an empty plist.");

  // store the return value upfront
  VAL_T val = head->get_value();

  // we will be deleting some pmem, so use a transaction
  flat_transaction::run(pop, [&] {
    auto new_head = head->get_next();

    // delete the old head persistent memory
    delete_persistent<pnode<VAL_T, ROOT_T>>(head);

    // update the head to point to the correct pnode now
    head = new_head;

    len--;
  });

  // return the value we pulled upfront
  return val;
}

// Insert the given value at the given index into the list.
template <typename VAL_T, typename ROOT_T>
void plist<VAL_T, ROOT_T>::insert(const VAL_T& val, int idx) {
    if (idx < 0 || idx > len) 
        throw std::out_of_range("Given index is outside the range of the list.");

    // we edit pmem, so use a transaction
    flat_transaction::run(pop, [&] {
        auto n = make_persistent<pnode<VAL_T, ROOT_T>>(val, pop);
        
        // if there are no existing nodes, head & tail both point to the new node
        if (len == 0) {
            head = n;
            tail = n;
        }
        else {
            auto current = head;
            int i = 0;

            // get node right before the chosen index
            while (i < idx - 1 && current != nullptr) {
                current = current->get_next();
                i++;
            }

            // save node's new next value
            auto new_next = current->get_next();

            // set current to point to the new node
            current->set_next(n);

            // set the new node to point current's previous next element
            n->set_next(new_next);
        }

        len++;
    });
}

// Remove the node at the given index, returning the value of the item removed.
template <typename VAL_T, typename ROOT_T>
VAL_T plist<VAL_T, ROOT_T>::remove(int idx) {
    if (idx < 0 || idx >= len)
        throw std::out_of_range("Cannot remove beyond range of the list.");

    VAL_T val;

    // we edit pmem
    flat_transaction::run(pop, [&] {
        auto current = head;
        int i = 0;

        // get the node right before the node to delete
        while (i < idx - 1 && current != nullptr) {
            current = current->get_next();
            i++;
        }

        // get node to delete and store its value
        auto to_delete = current->get_next();
        val = to_delete->get_value();

        // get the new next for current
        auto new_next = to_delete->get_next();

        // reorder the list to omit the deleted node
        current->set_next(new_next);
        to_delete->set_next(nullptr);

        // delete the node
        delete_persistent<pnode<VAL_T, ROOT_T>>(to_delete);

        len--;
    });

    return val;
}

/* =============================== GET/SET ================================= */

// Get the current number of items in the plist.
template <typename VAL_T, typename ROOT_T>
int plist<VAL_T, ROOT_T>::get_length() const {
    return len;
}

// Get whether or not the list is completely empty.
template <typename VAL_T, typename ROOT_T>
bool plist<VAL_T, ROOT_T>::is_empty() const {
    return len == 0;
}

/* ================================ MISC. ================================== */

// Completely clear the list, removing all elements.
template <typename VAL_T, typename ROOT_T>
void plist<VAL_T, ROOT_T>::clear() {
    auto current = head;
    int i = 0;

    // we edit pmem
    flat_transaction::run(pop, [&] {
        // step through and delete all nodes
        while (i < len && current != nullptr) {
            // get the next item and store it
            auto next = current->get_next();

            // delete the current node
            delete_persistent<pnode<VAL_T, ROOT_T>>(current);

            // and set the current to be the stored node
            current = next;

            i++;
        }
    });

    len = 0;
}

// Refresh the current pool object that the plist stores. Must be called when loading
// an existing pool file from disk.
template <typename VAL_T, typename ROOT_T>
void plist<VAL_T, ROOT_T>::refresh_pool(pool<ROOT_T> new_pop) {
    auto current = head;
    int i = 0;

    // refresh the pool on every pnode object this plist stores
    while (i < len && current != nullptr) {
        current->refresh_pool(new_pop);
        current = current->get_next();
        i++;
    }

    // finally, run a transaction in the new pool to update this pool variable
    flat_transaction::run(new_pop, [&] {
        pop = new_pop;
    });
}