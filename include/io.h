#ifndef IO_H
#define IO_H

#include <unistd.h>
#include <fcntl.h>

#include "dynamicArray.h"

int *handle_redirection(struct dynamicArray *tokens);

#endif // IO_H