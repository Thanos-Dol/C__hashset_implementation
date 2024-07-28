#include <stdio.h>
#include <malloc.h>
#include "hashset.h"




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


unsigned hashset_get_size(Hashset* h) {

    return h -> size;
}


int hashset_is_empty(Hashset* h) {

    return !(h -> size);
}


void hashset_insert(Hashset* h, void* element) {

    unsigned hashcode = h -> get_hashcode(element);

    unsigned pos = hashcode % (h -> capacity);

    unsigned first_empty_dirty_bit_pos = h -> capacity;

    short flag_collision_happened = 0;

    while (1)
    {
        if (h -> data[pos])
        {
            if (h -> datapoint_equal(h -> data[pos], element))
            {
                return;  // element already inside hashset
            }
            flag_collision_happened = 1;
        }
        else
        {
            if (!(h -> dirty_bits[pos]))
            {
                if (first_empty_dirty_bit_pos != h -> capacity)
                {
                    pos = first_empty_dirty_bit_pos;
                }

                h -> data[pos] = h -> datapoint_copy(element);

                if (flag_collision_happened)
                {
                    h -> dirty_bits[pos ? pos - 1 : h -> capacity - 1] = 1;
                }

                h -> size += 1;

                if (h -> size == h -> capacity / 2U)
                {
                    _hashset_update_capacity(h, (h -> capacity) * 2U);
                }

                return;
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
                if (!(h -> dirty_bits[pos]) && h -> dirty_bits[pos ? pos - 1 : h -> capacity - 1])
                {
                    h -> dirty_bits[pos ? pos - 1 : h -> capacity - 1] = 0;
                }
                h -> size -= 1;
                if ((h -> size < (h -> capacity / 5U)) && (h -> capacity > DEFAULT_INITIAL_HASHSET_CAPACITY))
                {
                    _hashset_update_capacity(h, (h -> capacity) / 2U);
                }
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