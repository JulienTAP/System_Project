#ifndef PIPE_H
#define PIPE_H

#include "dynamicArray.h"

void execute_pipe(struct dynamicArray *commands);
char **parse_section(char *cmd);
int handle_pipes(struct dynamicArray *tokens, char *cwd);
void build_section(char *section, size_t section_size, struct dynamicArray *tokens, size_t start, size_t end);
int check_pipe_position(struct dynamicArray *tokens, int pipe_index);

#endif