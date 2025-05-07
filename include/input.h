#ifndef INPUT_H
#define INPUT_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "dynamicArray.h"
#include "structures.h"

void parse_input(struct dynamicArray *tokens, char *input, size_t size);

job *create_job_from_tokens(struct dynamicArray *tokens, int *fds);

bool is_background_job(struct dynamicArray *tokens);

#endif // INPUT_H