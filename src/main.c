#include "input.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>


int main() {
    char input[1024];
    char *tokens[100];

    printf("Simple Terminal. Type something and press Enter (type 'exit' or 'q' to quit) :\n");

    system("clear"); // Clear the terminal screen
    chdir(getenv("HOME")); // Change to home directory

    while (1) {
        char *cwd = getcwd(NULL, 0);
        if (cwd == NULL) {
            perror("getcwd failed");
            return 1;
        }

        printf("%s $> ", cwd);
        fflush(stdout); // Ensure prompt is displayed immediately

        read_input(input, sizeof(input));
        
        if (strcmp(input, "exit") == 0 || strcmp(input, "q") == 0) {
            printf("Exiting terminal.\n");
            break;
        }

        if(input[0] == '\0') {
            continue; // Skip empty input
        }

        parse_input(tokens, input, sizeof(input));

        if(strcmp(tokens[0], "cd") == 0) {
            if(tokens[1] == NULL) {
                chdir(getenv("HOME")); // Change to home directory
            } else if (chdir(tokens[1]) != 0) {
                perror("cd failed");
            }
        } else if (strcmp(tokens[0], "ls") == 0) {
            // Call the ls function here
            system("ls");
        } else {
            printf("Command not recognized: %s\n", tokens[0]);
        }
    }

    return 0;
}