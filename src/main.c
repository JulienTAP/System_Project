#include "systemFunctions.h"
#include "structures.h"
#include "input.h"
#include "dynamicArray.h"
#include "io.h"
#include "banner.h"

#include "pipe.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>

int main()
{
    clear(); // Clear the terminal screen

    chdir(getenv("HOME")); // Change to home directory

    int stdin_backup = dup(STDIN_FILENO);   // Save the input stream
    int stdout_backup = dup(STDOUT_FILENO); // Save the output stream

    //Using the banner function outside the while loop so that it only appears once 
    display_colored_banner();

    printf("\nSimple Terminal. Type something and press Enter (type 'exit' or 'q' to quit) :\n");

    while (1)
    {
        // After handling redirection, restore stdin and stdout
        dup2(stdin_backup, STDIN_FILENO);
        dup2(stdout_backup, STDOUT_FILENO);

        char *cwd = get_cwd(); // Get current working directory

        printf("%sPoly_Shell%s %s$> ", WHITE_ON_BLUE, RESET, cwd); // Display the current working directory
        fflush(stdout);        // Ensure prompt is displayed immediately

        char input[1024];
        struct dynamicArray tokens = init_array(32); // Initialize dynamic array for tokens

        parse_input(&tokens, input, sizeof(input));

        // Handle the pipe command, then free the used memory
        if (handle_pipes(&tokens, cwd))
        {
            free(tokens.data); // Free command tokens
            free(cwd);         // Free current working directory string
            continue;          // Jump to next shell prompt
        }

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

        handle_redirection(&tokens); // Handle input/output redirection

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
        else if (strcmp(tokens.data[0], "clear") == 0)
        {
            clear(); // Clear the terminal screen
            free(cwd);
            continue; // Skip to next iteration
        }
        else if (strcmp(tokens.data[0], "pwd") == 0)
        {
            printf("%s\n", cwd); // Print current working directory
            free(cwd);
            continue; // Skip to next iteration
        }
        else if (strcmp(tokens.data[0], "cp") == 0)
        {
            pop_element(&tokens, 0); // Remove the 'cp' command from tokens
            cp(&tokens); // Copy file or directory
            free(cwd);
            continue; // Skip to next iteration
        }
        else if (strcmp(tokens.data[0], "mkdir") == 0)
        {
            if (tokens.size > 1)
            {
                mkdir(tokens.data[1], 0777); // Create directory with specified permissions
            }
            else
            {
                printf("No directory name provided\n");
            }
            free(cwd);
            continue; // Skip to next iteration
        }
        else if (strcmp(tokens.data[0], "rm") == 0)
        {
            if (tokens.size > 1)
            {
                remove(tokens.data[1]); // Remove file
            }
            else
            {
                printf("No file name provided\n");
            }
            free(cwd);
            continue; // Skip to next iteration
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