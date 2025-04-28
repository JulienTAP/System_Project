#ifndef INPUT_H
#define INPUT_H

#include <stdio.h>
#include <string.h>

#include "dynamicArray.h"

void parse_input(struct dynamicArray *tokens, char *input, size_t size);

#endif // INPUT_H