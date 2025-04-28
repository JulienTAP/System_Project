#include "input.h"
#include "structures.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>


int main() {
    char input[1024];
    char *tokens[100];


    printf("Simple Terminal. Type something and press Enter (type 'exit' or 'q' to quit) :\n");

    system("clear"); // Clear the terminal screen
    chdir(getenv("HOME")); // Change to home directory

    while (1) {

        char curr_dir[1024];
        getcwd(curr_dir, sizeof(curr_dir));
        if (curr_dir == NULL) {
            perror(RED "getcwd failed" RST);
            return 1;
        }

        printf("%s $> ", curr_dir);


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
            handle_cd(tokens);
        } else if (strcmp(tokens[0], "ls") == 0 || strcmp(tokens[0], "l") == 0) {
            handle_ls(tokens);
        } else if (strcmp(tokens[0], "./") == 0) {
            handle_run(tokens);
        }
        
        else {
            printf(RED"Command not recognized: %s\n" RST, tokens[0]);
        }
    }

    return 0;
}