#ifndef IO_H
#define IO_H

#include <unistd.h>
#include <fcntl.h>

#include "dynamicArray.h"

int redirect_input(const char *input_source);
int redirect_output(const char *output_target);

void handle_redirection(struct dynamicArray *tokens);

#endif // IO_H