#include <stdio.h>
#include <string.h>

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

int main() {
    char input[256];

    printf("Simple Terminal. Type something and press Enter (type 'exit' to quit):\n");

    while (1) {
        read_line(input, sizeof(input));

        if (strcmp(input, "exit") == 0) {
            printf("Exiting terminal.\n");
            break;
        }

        printf("You entered: %s\n", input);
    }

    return 0;
}