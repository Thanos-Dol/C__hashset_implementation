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




Hashset* hashset_init(
    unsigned (*given_get_hashcode)(void*),
    void (*given_datapoint_destroyer)(void*),
    void* (*given_datapoint_copy)(void*),
    int (*given_datapoint_equal)(void*, void*)
);

void hashset_destroy(Hashset* h);

Hashset* hashset_copy(Hashset* h);

unsigned hashset_get_size(Hashset* h);

int hashset_is_empty(Hashset* h);

void hashset_insert(Hashset* h, void* element);

void hashset_remove(Hashset* h, void* element);

int hashset_find(Hashset* h, void* element);




#endif /* RESURVIVOR__HASHSET_H */