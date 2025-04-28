#include "input.h"
#include "dynamicArray.h"

//parse inputs on space or "./"
void parse_input(struct dynamicArray *tokens, char *input, size_t size) {

    //read input from stdin
    if (fgets(input, size, stdin) == NULL) {
        perror("fgets failed");
        return;
    }
    //remove newline character
    size_t len = strlen(input);
    if (len > 0 && input[len - 1] == '\n') {
        input[len - 1] = '\0';
    }

    char *token = strtok(input, " ");
    while (token != NULL) {
        add_element(tokens, token); // Add token to dynamic array
        token = strtok(NULL, " ");
    }
}