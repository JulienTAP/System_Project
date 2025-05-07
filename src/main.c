#include "systemFunctions.h"
#include "structures.h"
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
#include <sys/stat.h>

int main()
{
    clear(); // Clear the terminal screen

    chdir(getenv("HOME")); // Change to home directory

    while (1)
    {

        init_shell(); // Reinitialize shell settings

        do_job_notification(); // Check for job notifications

        char *cwd = get_cwd(); // Get current working directory
        printf("%s $> ", cwd); // Display the current working directory

        fflush(stdout); // Ensure prompt is displayed immediately

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

        // Check exit
        if (strcmp(tokens.data[0], "exit") == 0 || strcmp(tokens.data[0], "q") == 0)
        {
            delete_all_jobs(); // Delete all jobs before exiting
            printf("Exiting...\n");
            free(cwd);
            break; // Exit the loop if user types 'exit' or 'q'
        }

        bool background = is_background_job(&tokens); // Check if job is in background

        // Handle input/output redirection
        int *fds = handle_redirection(&tokens);
        if (fds == NULL)
        {
            free(cwd);
            continue; // Skip to next iteration if redirection failed
        }

        if (strcmp(tokens.data[0], "cd") == 0)
        {
            if (tokens.size > 1)
            {
                cd(tokens.data[1]); // Change directory
            }
            else
            {
                cd(NULL); // Change to home directory
            }
        }
        else if (strcmp(tokens.data[0], "jobs") == 0)
        {
            if (tokens.size > 1 && strcmp(tokens.data[1], "-l") == 0)
            {
                print_jobs_info(); // Print jobs info
            }
            else
            {
                list_jobs(); // List all jobs
            }
        }
        else if (strcmp(tokens.data[0], "fg") == 0)
        {
            if (tokens.size > 1)
            {
                pid_t pgid = atoi(tokens.data[1]); // Get the job's process group ID
                job *j = find_job(pgid);           // Find the job by PGID
                if (j != NULL)
                {
                    put_job_in_foreground(j, 1); // Bring the job to the foreground
                }
                else
                {
                    printf("No such job with PGID: %d\n", pgid);
                }
            }
            else
            {
                printf("Usage: fg <job-pgid>\n");
            }
        }
        else if (strcmp(tokens.data[0], "cp") == 0)
        {
            pop_element(&tokens, 0);            // Remove the 'cp' command from tokens
            cp(tokens.data[0], tokens.data[1]); // Copy file or directory
            free(cwd);
            continue; // Skip to next iteration
        }
        else
        {
            // Create job from tokens and execute
            job *new_job = create_job_from_tokens(&tokens, fds);
            if (new_job == NULL)
            {
                fprintf(stderr, "Failed to create job.\n");
                free(cwd);
                free(fds);
                free(tokens.data);
                continue;
            }

            add_job(new_job); // Add the job to the job list

            if (background)
            {
                launch_job(new_job, 0); // Launch the job in the background
                printf("Job [%d] running in background: %s\n", new_job->pgid, new_job->command);
            }
            else
            {
                launch_job(new_job, 1); // Launch the job in the foreground
            }
        }

        free(cwd); // Free current working directory string
        free(fds);
        free(tokens.data); // Free command tokens
    }

    return 0;
}