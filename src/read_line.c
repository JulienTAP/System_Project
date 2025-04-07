#include "read_line.h"

    
void read_line(char *input, size_t size) {
    printf("> ");
    if (fgets(input, size, stdin) != NULL) {
        // Remove newline character from input
        input[strcspn(input, "\n")] = 0;
    } else {
        printf("Error reading input.\n");
        input[0] = '\0'; // Set input to an empty string in case of error
    }
}