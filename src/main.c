#include "read_line.h"

#include <stdio.h>
#include <string.h>


int main() {
    char input[256];

    printf("Simple Terminal. Type something and press Enter (type 'exit' or 'q' to quit) :\n");

    while (1) {
        read_line(input, sizeof(input));

        if (strcmp(input, "exit") == 0 || strcmp(input, "q") == 0) {
            printf("Exiting terminal.\n");
            break;
        }

        printf("You entered: %s\n", input);
    }

    return 0;
}