/**
 * @file dynamicArray.c
 * @brief Implementation of a dynamic string array data structure.
 *
 * Provides functions for creating, managing, and manipulating a resizable
 * array of strings. Handles memory allocation, element addition/removal,
 * and array expansion automatically.
 */

#include "dynamicArray.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Initializes a new dynamic array with specified capacity.
 *
 * Allocates memory for the array storage. Exits on allocation failure.
 *
 * @param capacity Initial capacity of the array
 * @return Initialized dynamicArray structure
 */
struct dynamicArray init_array(size_t capacity)
{
    struct dynamicArray tab;
    tab.data = (char **)malloc(capacity * sizeof(char[1024]));
    if (tab.data == NULL)
    {
        perror("malloc failed");
        exit(1);
    }
    else
    {
        tab.capacity = capacity;
        tab.size = 0;
    }
    return tab;
}

/**
 * @brief Doubles the capacity of a dynamic array.
 *
 * Creates a new storage area with double capacity and copies existing elements.
 * Frees the old storage and updates the array structure.
 *
 * @param Array Pointer to the dynamic array to expand
 */
void add_capacity(struct dynamicArray *Array)
{
    char **NewArray = (char **)calloc(2 * Array->capacity, sizeof(char *));
    if (NewArray == NULL)
    {
        perror("calloc failed");
        exit(1);
    }
    else
    {
        memcpy(NewArray, Array->data, Array->size * sizeof(char *));
        free(Array->data);
        Array->data = NewArray;
        Array->capacity *= 2;
    }
}

/**
 * @brief Adds an element to the dynamic array.
 *
 * Automatically expands the array if capacity is reached.
 *
 * @param Array Pointer to the dynamic array
 * @param element String to add to the array (pointer is stored directly)
 */
void add_element(struct dynamicArray *Array, char *element)
{
    if (Array->capacity == Array->size)
    {
        add_capacity(Array);
    }
    Array->data[Array->size] = element;
    Array->size++;
}

/**
 * @brief Removes an element from the dynamic array by index.
 *
 * Shifts remaining elements to fill the gap. Does not free the element memory.
 *
 * @param Array Pointer to the dynamic array
 * @param index Position of element to remove (0-based)
 */
void pop_element(struct dynamicArray *Array, int index)
{
    if (index > Array->size - 1 || index < 0)
    {
        perror("Index out of range");
    }
    else
    {
        for (size_t i = index; i < Array->size - 1; i++)
        {
            Array->data[i] = Array->data[i + 1];
        }
        Array->data[Array->size - 1] = NULL;
        Array->size--;
    }
}

/**
 * @brief Prints all elements in the dynamic array.
 *
 * Displays the current size followed by each element on a new line.
 *
 * @param Array Pointer to the dynamic array to print
 */
void print_elements(struct dynamicArray *Array)
{
    printf("Size : %zu\n", Array->size);
    printf("Elements :\n");
    for (int i = 0; i < Array->size; i++)
    {
        printf("%s\n", Array->data[i]);
    }
}

/**
 * @brief Checks if the dynamic array is empty.
 *
 * @param Array Pointer to the dynamic array to check
 * @return true if array has no elements, false otherwise
 */
bool is_empty(struct dynamicArray *Array)
{
    return Array->size == 0;
}