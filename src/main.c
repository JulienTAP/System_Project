#include "systemFunctions.h"
#include "input.h"
#include "dynamicArray.h"
#include "io.h"
#include "structures.h"
#include "shellHandler.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main()
{
    clear(); // Clear the terminal screen

    printf("Simple Terminal. Type something and press Enter (type 'exit' or 'q' to quit) :\n");

    chdir(getenv("HOME")); // Change to home directory

    init_shell(); // Initialize shell settings

    //print current pid, ppid and pgid
    printf("PID: %d\n", getpid());
    printf("PPID: %d\n", getppid());
    printf("PGID: %d\n", getpgid(getpid()));

    while (1)
    {
        do_job_notification(); // Check for job notifications

        char *cwd = get_cwd(); // Get current working directory
        printf("%s $> ", cwd); // Display the current working directory
        
        fflush(stdout);        // Ensure prompt is displayed immediately

        char input[1024];
        struct dynamicArray tokens = init_array(32); // Initialize dynamic array for tokens
        parse_input(&tokens, input, sizeof(input));
        // print_elements(&tokens); // Print parsed tokens

        // Check if the command is empty
        if (tokens.size == 0)
        {
            free(cwd);
            continue; // Skip empty input
        }

        if (strcmp(tokens.data[0], "exit") == 0 || strcmp(tokens.data[0], "q") == 0)
        {
            printf("Exiting...\n");
            free(cwd);
            break; // Exit the loop if user types 'exit' or 'q'
        }

        // Handle input/output redirection
        int *fds = handle_redirection(&tokens);
        if (fds == NULL)
        {
            free(cwd);
            continue; // Skip to next iteration if redirection failed
        }

        if (strcmp(tokens.data[0], "cd") == 0)
        {
            cd(tokens.data[1]); // Change directory
        }
        else
        {
            job *new_job = create_job_from_tokens(&tokens, fds); // Create a job from tokens
                              // Print job info
            if (new_job == NULL)
            {
                free(cwd);
                continue; // Skip to next iteration if job creation failed
            }
            else
            {
                // Launch the job
                launch_job(new_job, 1); // 1 for foreground
                free(new_job->command);
                free(new_job);
            }

            print_job_info(new_job);
        }

        

        free(cwd); // Free current working directory string
        free(fds);
        free(tokens.data); // Free command tokens

        // // Handle the pipe command, then free the used memory
        // if (handle_pipes(&tokens, cwd))
        // {
        //     free(tokens.data); // Free command tokens
        //     free(cwd);         // Free current working directory string
        //     continue;          // Jump to next shell prompt
        // }

        // // DEBUG
        // for (size_t i = 0; i < tokens.size; i++)
        // {
        //     printf("tokens[%zu]: %s\n", i, tokens.data[i]);
        // }
        // printf("Size : %zu\n", tokens.size);

        // handle_redirection(&tokens); // Handle input/output redirection

        // if (strcmp(tokens.data[0], "cd") == 0)
        // {
        //     cd(tokens.data[1]); // Change directory
        //     free(cwd);
        //     continue; // Skip to next iteration
        // }
        // else if (strcmp(tokens.data[0], "ls") == 0 || strcmp(tokens.data[0], "l") == 0)
        // {
        //     ls(); // List files in current directory
        //     free(cwd);
        //     continue; // Skip to next iteration
        // }
        // else if (strcmp(tokens.data[0], "./") == 0)
        // {
        //     // Execute the command in tokens[1]
        //     if (tokens.data[1] != NULL)
        //     {
        //         execute(tokens.data[1], &tokens.data[1]); // Execute the command
        //         free(cwd);
        //         continue; // Skip to next iteration
        //     }
        //     else
        //     {
        //         printf("No command provided after './'\n");
        //     }
        // }
        // else if (strcmp(tokens.data[0], "clear") == 0)
        // {
        //     clear(); // Clear the terminal screen
        //     free(cwd);
        //     continue; // Skip to next iteration
        // }
        // else if (strcmp(tokens.data[0], "pwd") == 0)
        // {
        //     printf("%s\n", cwd); // Print current working directory
        //     free(cwd);
        //     continue; // Skip to next iteration
        // }
        // else if (strcmp(tokens.data[0], "cp") == 0)
        // {
        //     cp(tokens.data[1], tokens.data[2]); // Copy file or directory
        //     free(cwd);
        //     continue; // Skip to next iteration
        // }
        // else if (strcmp(tokens.data[0], "exit") == 0 || strcmp(tokens.data[0], "q") == 0)
        // {
        //     printf("Exiting...\n");
        //     free(cwd);
        //     break; // Exit the loop if user types 'exit' or 'q'
        // }
        // else
        // {
        //     printf("Command not recognized : %s\n", tokens.data[0]);
        // }

        // // Free the dynamic array
        // free(tokens.data);
    }

    return 0;
}