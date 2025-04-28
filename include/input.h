#ifndef INPUT_H
#define INPUT_H

#include <stdio.h>
#include <string.h>

#include "dynamicArray.h"

void parse_input(struct dynamicArray *tab, char *input, size_t size);

void print_token(struct dynamicArray* tab, size_t size);

#endif // INPUT_H