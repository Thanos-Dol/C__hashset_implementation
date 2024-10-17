#ifndef RESURVIVOR__HASHSET_H
#define RESURVIVOR__HASHSET_H


#include <stdio.h>


#define DEFAULT_INITIAL_HASHSET_CAPACITY 100


typedef struct {

    void** data;
    short* dirty_bits;
    unsigned size;
    unsigned capacity;

    unsigned (*get_hashcode)(void*);
    void (*datapoint_destroyer)(void*);
    void* (*datapoint_copy)(void*);
    int (*datapoint_equal)(void*, void*);

} Hashset;


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
);

/**
 * @brief Destructed for the provided Hashset struct, frees all memory that the struct object holds
*/
void hashset_destroy(Hashset* h);

/**
 * @brief Creates a new Hashset struct object that is a deep copy (meaning that new memory will be allocated for every element of the hashset's datatype that is stored inside the hashset) of the argument struct object
 * @param h Pointer to Hashset struct object a copy of which will be made
 * @return Pointer to newly created Hashset struct object
 */
Hashset* hashset_copy(Hashset* h);

/**
 * @brief Returns number of elements inside the hashtable
*/
unsigned hashset_get_size(Hashset* h);

/**
 * @brief Returns whether the hashtable is empty (without elements) or not
 * @return int 1 if hashtable is empty, int 1 if it's not
*/
int hashset_is_empty(Hashset* h);

/**
 * @brief Performs the insert operation to the hashtable
 * 
 * @param h Pointer to the hashset
 * @param element The element to be inserted
 * @return int 1 if element is inserted, int 0 if element is already inside
*/
void hashset_insert(Hashset* h, void* element);

/**
 * @brief Performs the remove operation
 * 
 * @param h Pointer to the hashset
 * @param element the element that will be deleted
*/
void hashset_remove(Hashset* h, void* element);

/**
 * @brief Checks whether an element is inside the hashset
 * 
 * @param h Pointer to the hashset
 * @param element The element that is checked whether it's inside hashset
 * @return void* a COPY of the element if it is indeed found in the hashset, NULL if not found
*/
int hashset_find(Hashset* h, void* element);

/**
 * @brief Changes hashtable's underlying array capacity to new_capacity, used when underlying array needs to increase or shrink
 * 
 * @param h Pointer to the hashset
 * @param new_capacity the new size of the underlying array
*/
void _hashset_update_capacity(Hashset* h, unsigned new_capacity);


#endif /* RESURVIVOR__HASHSET_H */