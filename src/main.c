#include "input.h"

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
        } else if (strcmp(tokens[0], "ls") == 0 || strcmp(tokens[0], "l") == 0) {
            // Call the ls function here
            system("ls");
        } else if (strcmp(tokens[0], "./") == 0) {
            // Execute the command in tokens[1]
            if (tokens[1] != NULL) {
                pid_t pid = fork();
                if (pid == 0) {
                    // Child process
                    execvp(tokens[1], &tokens[1]);
                    perror("exec failed");
                    exit(1);
                } else if (pid < 0) {
                    perror("fork failed");
                } else {
                    // Parent process
                    wait(NULL); // Wait for child process to finish
                }
            } else {
                printf("No command provided after './'\n");
            }
        }
        
        else {
            printf("Command not recognized: %s\n", tokens[0]);
        }
    }

    return 0;
}