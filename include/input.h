#ifndef INPUT_H
#define INPUT_H

#include <stdio.h>
#include <string.h>

void read_input(char *input, size_t size);

void parse_input(char **tokens, char *input, size_t size);

void handle_cd(char *tokens[100]);
void handle_ls(char *tokens[100]);
void handle_run(char *tokens[100]);

#endif // INPUT_H