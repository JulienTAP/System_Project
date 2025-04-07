#include "input.h"

#include <stdio.h>
#include <string.h>


int main() {
    char input[1024];
    char *tokens[100];

    printf("Simple Terminal. Type something and press Enter (type 'exit' or 'q' to quit) :\n");

    while (1) {
        printf("$> ");
        fflush(stdout); // Ensure prompt is displayed immediately

        read_input(input, sizeof(input));
        if (strcmp(input, "exit") == 0 || strcmp(input, "q") == 0) {
            printf("Exiting terminal.\n");
            break;
        }

        parse_input(tokens, input, sizeof(input));

    }

    return 0;
}