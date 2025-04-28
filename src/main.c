#include "systemFunctions.h"
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
        char *cwd = get_cwd(); // Get current working directory

        printf("%s $> ", cwd); // Display the current working directory
        fflush(stdout);        // Ensure prompt is displayed immediately

        char input[1024];
        struct dynamicArray tokens = init_array(32); // Initialize dynamic array for tokens

        parse_input(&tokens, input, sizeof(input));

        for (size_t i = 0; i < tokens.size; i++)
        {
            printf("tokens[%zu]: %s\n", i, tokens.data[i]);
        }
        printf("Size : %zu\n", tokens.size);

        if (tokens.size == 0)
        {
            free(cwd);
            continue; // Skip empty input
        }

        if (strcmp(tokens.data[0], "cd") == 0)
        {
            cd(tokens.data[1]); // Change directory
            free(cwd);
            continue; // Skip to next iteration
        }
        else if (strcmp(tokens.data[0], "ls") == 0 || strcmp(tokens.data[0], "l") == 0)
        {
            ls(); // List files in current directory
            free(cwd);
            continue; // Skip to next iteration
        }
        else if (strcmp(tokens.data[0], "./") == 0)
        {
            // Execute the command in tokens[1]
            if (tokens.data[1] != NULL)
            {
                execute(tokens.data[1], &tokens.data[1]); // Execute the command
                free(cwd);
                continue; // Skip to next iteration
            }
            else
            {
                printf("No command provided after './'\n");
            }
        }
        else if (strcmp(tokens.data[0], "exit") == 0 || strcmp(tokens.data[0], "q") == 0)
        {
            printf("Exiting...\n");
            free(cwd);
            break; // Exit the loop if user types 'exit' or 'q'
        }
        else
        {
            printf("Command not recognized : %s\n", tokens.data[0]);
        }

        // Free the dynamic array
        free(tokens.data);
    }

    return 0;
}