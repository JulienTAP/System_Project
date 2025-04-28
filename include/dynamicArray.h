#ifndef DYNAMICARRAY_H
#define DYNAMICARRAY_H

#include <stdio.h>
#include <stdlib.h>

struct dynamicArray
{ // dynamic array to store char token
    char **data;
    size_t size;
    size_t capacity;
};

struct dynamicArray init_array(size_t capacity);

void add_capacity(struct dynamicArray *Array);

void pop_element(struct dynamicArray *Array, int index);

void add_element(struct dynamicArray *Array, char *element);

void print_elements(struct dynamicArray *Array);

#endif
