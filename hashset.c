#include <stdio.h>
#include <malloc.h>
#include "hashset.h"



/**
 * @brief Changes hashtable's underlying array capacity to new_capacity, used when underlying array needs to increase or shrink
 * 
 * @param h Pointer to the hashset
 * @param new_capacity the new size of the underlying array
*/
void _hashset_update_capacity(Hashset* h, unsigned new_capacity) {

    unsigned capacity_old = h -> capacity;
    void** data_new = (void**) calloc(new_capacity, sizeof(void*));
    short* dirty_bits_new = (short*) calloc(new_capacity, sizeof(short));

    for (int i = 0; i < capacity_old; ++i)
    {
        if (h -> data[i])
        {
            unsigned hashcode = h -> get_hashcode(h -> data[i]);

            unsigned pos = hashcode % (new_capacity);

            unsigned first_empty_dirty_bit_pos = new_capacity;

            short flag_collision_happened = 0;

            while (1)
            {
                if (data_new[pos])
                {
                    //* this is a redundant check, always false
                    if (h -> datapoint_equal(data_new[pos], h -> data[i]))
                    {
                        ;  // element already inside hashset
                    }
                    flag_collision_happened = 1;
                }
                else
                {
                    if (!(dirty_bits_new[pos]))
                    {
                        if (first_empty_dirty_bit_pos != new_capacity)
                        {
                            pos = first_empty_dirty_bit_pos;
                        }

                        data_new[pos] = h -> data[i];

                        if (flag_collision_happened)
                        {
                            dirty_bits_new[pos ? pos - 1 : new_capacity - 1] = 1;
                        }

                        break;
                    }
                    else
                    {
                        if (first_empty_dirty_bit_pos == new_capacity)
                        {
                            first_empty_dirty_bit_pos = pos;
                        }
                    }
                }

                pos = (pos + 1) % (new_capacity);
            }
        }
    }

    free(h -> data);
    free(h -> dirty_bits);

    h -> data = data_new;
    h -> dirty_bits = dirty_bits_new;
    h -> capacity = new_capacity;

    return;
}


/**
 * @brief Constructor for the hashset struct, takes as arguments the callbacks that perform the basic operations of the hashset's datatype, namely calculate hash, destroy, copy and check for equality
 * 
 * @param given_get_hashcode Callback that must calculate the hash for an element of the hashtable's datatype
 * @param given_datapoint_destroyer Callback that must destroy the element of the datatype, that is free the memory the datatype holds (the argument of this function as well as the the elements stored in the hashtable's array will be pointers to that datatype)
 * @param given_datapoint_copy Callbak that copies the value of an element of the hashtable's datatype, copied value will have new allocated memory and a Pointer to the allocated memory is returned
 * @param given_datapoint_equal Callback that checks for equality between 2 elements of the hashtable's datatype
 * @return Pointer to the newly created Hashset struct
*/
Hashset* hashset_init(
    unsigned (*given_get_hashcode)(void*),
    void (*given_datapoint_destroyer)(void*),
    void* (*given_datapoint_copy)(void*),
    int (*given_datapoint_equal)(void*, void*)
    ) {

    Hashset* tmp = (Hashset*) malloc(sizeof(Hashset));

    tmp -> data = (void**) calloc(DEFAULT_INITIAL_HASHSET_CAPACITY, sizeof(void*));
    tmp -> dirty_bits = (short*) calloc(DEFAULT_INITIAL_HASHSET_CAPACITY, sizeof(short));
    tmp -> size = 0;
    tmp -> capacity = DEFAULT_INITIAL_HASHSET_CAPACITY;

    for (int i = 0; i < tmp -> capacity; ++i)
    {
        tmp -> data[i] = NULL;
        tmp -> dirty_bits[i] = 0;
    }

    tmp -> get_hashcode = given_get_hashcode;
    tmp -> datapoint_destroyer = given_datapoint_destroyer;
    tmp -> datapoint_copy = given_datapoint_copy;
    tmp -> datapoint_equal = given_datapoint_equal;

    return tmp;
}


/**
 * @brief Destructed for the provided Hashset struct, frees all memory that the struct object holds
*/
void hashset_destroy(Hashset* h) {

    for (int i = 0; i < h -> capacity; ++i)
    {
        if (h -> data[i])
        {
            h -> datapoint_destroyer(h -> data[i]);
        }
    }
    free(h -> dirty_bits);
    free(h -> data);
    free(h);
    return;
}


/**
 * @brief Creates a new Hashset struct object that is a deep copy (meaning that new memory will be allocated for every element of the hashset's datatype that is stored inside the hashset) of the argument struct object
 * @param h Pointer to Hashset struct object a copy of which will be made
 * @return Pointer to newly created Hashset struct object
 */
Hashset* hashset_copy(Hashset* h) {

    Hashset* h_copy = (Hashset*) malloc(sizeof(Hashset));

    h_copy -> data = (void**) calloc(h -> capacity, sizeof(void*));
    h_copy -> size = h -> size;
    h_copy -> capacity = h -> capacity;
    h_copy -> get_hashcode = h -> get_hashcode;
    h_copy -> datapoint_destroyer = h -> datapoint_destroyer;
    h_copy -> datapoint_copy = h -> datapoint_copy;

    for (int i = 0; i < h -> capacity; ++i)
    {
        if (h -> data[i])
        {
            h_copy -> data[i] = h -> datapoint_copy(h -> data[i]);
        }
        else
        {
            h_copy -> data[i] = NULL;
        }

        h_copy -> dirty_bits[i] = h -> dirty_bits[i];
    }

    return h_copy;
}


/**
 * @brief Returns number of elements inside the hashtable
*/
unsigned hashset_get_size(Hashset* h) {

    return h -> size;
}


/**
 * @brief Returns whether the hashtable is empty (without elements) or not
 * @return int 1 if hashtable is empty, int 1 if it's not
*/
int hashset_is_empty(Hashset* h) {

    return !(h -> size);
}


/**
 * @brief Performs the insert operation to the hashtable
 * 
 * @param h Pointer to the hashset
 * @param element The element to be inserted
 * @return int 1 if element is inserted, int 0 if element is already inside
*/
int hashset_insert(Hashset* h, void* element) {

    unsigned hashcode = h -> get_hashcode(element);

    unsigned pos = hashcode % (h -> capacity);

    // this is used in case a collision happends and when while searching if element is inside after the first collision position an empty array position is passed which is marked with a dirty bit, later this position will be used for the insertion of the new element
    unsigned first_empty_dirty_bit_pos = h -> capacity;

    // if a collision has happened, meaning the hashed value of element lands in a position where another element is stored dirty bits might need to be updated
    short flag_collision_happened = 0;

    while (1)
    {
        if (h -> data[pos])
        {
            if (h -> datapoint_equal(h -> data[pos], element))
            {
                return 0;  // element already inside hashset
            }
            h -> dirty_bits[pos] = 1; // set dirty bit on this position, TODO : explain why it needs to be set
        }
        else
        {
            if (!(h -> dirty_bits[pos]))
            {
                // if an empty position has been found with a dirty bit set in the previous positions of the table while incrementally searching for the element, then insertion will occur on that position
                if (first_empty_dirty_bit_pos != h -> capacity)
                {
                    pos = first_empty_dirty_bit_pos;
                }

                h -> data[pos] = h -> datapoint_copy(element);
                h -> size += 1;

                if (h -> size == h -> capacity / 2U)
                {
                    _hashset_update_capacity(h, (h -> capacity) * 2U);
                }

                return 1;
            }
            else
            {
                if (first_empty_dirty_bit_pos == h -> capacity)
                {
                    first_empty_dirty_bit_pos = pos;
                }
            }
        }

        pos = (pos + 1) % (h -> capacity);
    }
}


/**
 * @brief Performs the remove operation
 * 
 * @param h Pointer to the hashset
 * @param element the element that will be deleted
*/
void hashset_remove(Hashset* h, void* element) {

    unsigned hashcode = h -> get_hashcode(element);

    unsigned pos = hashcode % (h -> capacity);

    while (1)
    {
        if (h -> data[pos])
        {
            if (h -> datapoint_equal(h -> data[pos], element))
            {
                h -> datapoint_destroyer(h -> data[pos]);
                h -> data[pos] = NULL;

                while (!(h -> data[pos]) && !(h -> dirty_bits[pos]) && (h -> dirty_bits[pos ? pos - 1 : h -> capacity - 1]))
                {
                    h -> dirty_bits[pos ? pos - 1 : h -> capacity - 1] = 0;
                    pos = pos ? pos - 1 : h -> capacity - 1;
                }

                h -> size -= 1;

                if ((h -> size < (h -> capacity / 5U)) && (h -> capacity > DEFAULT_INITIAL_HASHSET_CAPACITY))
                {
                    _hashset_update_capacity(h, (h -> capacity) / 2U);
                }
            }
            if (!(h -> dirty_bits[pos]))
            {
                return;
            }
        }
        else
        {
            if (!(h -> dirty_bits[pos]))
            {
                return;
            }
        }

        pos = (pos + 1) % (h -> capacity);
    }
}


/**
 * @brief Checks whether an element is inside the hashset
 * 
 * @param h Pointer to the hashset
 * @param element The element that is checked whether it's inside hashset
 * @return int 1 on success, 0 on failure
*/
int hashset_find(Hashset* h, void* element) {

    unsigned hashcode = h -> get_hashcode(element);

    unsigned pos = hashcode % (h -> capacity);

    while (1)
    {
        if (h -> data[pos])
        {
            if (h -> datapoint_equal(h -> data[pos], element))
            {
                return 1;
            }
            if (!(h -> dirty_bits[pos]))
            {
                return 0;
            }
        }
        else
        {
            if (!(h -> dirty_bits[pos]))
            {
                return 0;
            }
        }

        pos = (pos + 1) % (h -> capacity);
    }
}