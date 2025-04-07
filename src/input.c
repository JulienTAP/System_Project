#include "input.h"

    
void read_input(char *input, size_t size) {
    if (fgets(input, size, stdin) != NULL) {
        // Remove newline character from input
        input[strcspn(input, "\n")] = 0;
    } else {
        printf("Error reading input.\n");
        input[0] = '\0'; // Set input to an empty string in case of error
    }
}

void parse_input(char **tokens, char *input, size_t size) {
    char *token;
    size_t index = 0;

    token = strtok(input, " ");
    while (token != NULL && index < size) {
        tokens[index++] = token;
        token = strtok(NULL, " ");
    }
}