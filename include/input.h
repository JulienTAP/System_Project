#ifndef INPUT_H
#define INPUT_H

#include <stdio.h>
#include <string.h>

void read_input(char *input, size_t size);

void parse_input(char **tokens, char *input, size_t size);

#endif // INPUT_H