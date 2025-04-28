#include "input.h"
#include "dynamicArray.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main()
{
    system("clear");

    printf("Simple Terminal. Type something and press Enter (type 'exit' or 'q' to quit) :\n");

    chdir(getenv("HOME")); // Change to home directory

    while (1)
    {
        char *cwd = getcwd(NULL, 0);
        if (cwd == NULL)
        {
            perror("getcwd failed");
            return 1;
        }

        printf("%s $> ", cwd); // Display the current working directory
        fflush(stdout);        // Ensure prompt is displayed immediately

        char input[1024];
        struct dynamicArray tokens = init_array(32); // Initialize dynamic array for tokens

        parse_input(&tokens, input, sizeof(input));

        for (size_t i = 0; i < tokens.size; i++)
        {
            printf("tokens[%zu]: %s\n", i, tokens.data[i]);
        }
        printf("Size: %zu\n", tokens.size);

        if (tokens.size == 0)
        {
            free(cwd);
            continue; // Skip empty input
        }

        if (strcmp(tokens.data[0], "cd") == 0)
        {
            if (tokens.data[1] == NULL)
            {
                chdir(getenv("HOME")); // Change to home directory
            }
            else if (chdir(tokens.data[1]) != 0)
            {
                perror("cd failed");
            }
        }
        else if (strcmp(tokens.data[0], "exit") == 0 || strcmp(tokens.data[0], "q") == 0)
        {
            printf("Exiting...\n");
            free(cwd);
            break; // Exit the loop if user types 'exit' or 'q'
        }
        else if (strcmp(tokens.data[0], "ls") == 0 || strcmp(tokens.data[0], "l") == 0)
        {
            // Call the ls function here
            system("ls");
        }
        else if (strcmp(tokens.data[0], "./") == 0)
        {
            // Execute the command in tokens[1]
            if (tokens.data[1] != NULL)
            {
                pid_t pid = fork();
                if (pid == 0)
                {
                    // Child process
                    execvp(tokens.data[1], &tokens.data[1]);
                    perror("exec failed");
                    exit(1);
                }
                else if (pid < 0)
                {
                    perror("fork failed");
                }
                else
                {
                    // Parent process
                    wait(NULL); // Wait for child process to finish
                }
            }
            else
            {
                printf("No command provided after './'\n");
            }
        }

        else
        {
            printf("Command not recognized: %s\n", tokens.data[0]);
        }

        // Free the dynamic array
        free(tokens.data);
    }

    return 0;
}