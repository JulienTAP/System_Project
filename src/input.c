#include "input.h"
#include "dynamicArray.h"
#include "structures.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// parse inputs on space or "./"
void parse_input(struct dynamicArray *tokens, char *input, size_t size)
{

    // read input from stdin
    if (fgets(input, size, stdin) == NULL)
    {
        perror("fgets failed");
        return;
    }
    // remove newline character
    size_t len = strlen(input);
    if (len > 0 && input[len - 1] == '\n')
    {
        input[len - 1] = '\0';
    }

    char *token = strtok(input, " ");
    while (token != NULL)
    {
        add_element(tokens, token); // Add token to dynamic array
        token = strtok(NULL, " ");
    }
}


// Create a job from the parsed tokens
job *create_job_from_tokens(struct dynamicArray *tokens, int *fds)
{
    if (tokens == NULL || tokens->size == 0)
    {
        return NULL; // No tokens, return NULL
    }

    // Allocate memory for the job
    job *new_job = (job *)malloc(sizeof(job));
    if (!new_job)
    {
        perror("malloc failed for job");
        return NULL;
    }

    // Initialize the job
    new_job->next = NULL;
    new_job->command = strdup(tokens->data[0]); // Use the first token as the command
    new_job->first_process = NULL;
    new_job->pgid = getpid(); // Set the process group ID to the current process ID
    new_job->notified = 0;
    memset(&new_job->tmodes, 0, sizeof(struct termios));
    new_job->stdin = fds[0];  // Set stdin to the input file descriptor
    new_job->stdout = fds[1]; // Set stdout to the output file descriptor
    new_job->stderr = STDERR_FILENO;

    process *current_process = NULL;
    process *last_process = NULL;

    size_t start = 0;
    for (size_t i = 0; i <= tokens->size; i++)
    {
        if (i == tokens->size || strcmp(tokens->data[i], "|") == 0)
        {
            // Allocate memory for a new process
            process *new_process = (process *)malloc(sizeof(process));
            if (!new_process)
            {
                perror("malloc failed for process");
                free(new_job);
                return NULL;
            }

            // Initialize the process
            new_process->next = NULL;
            new_process->argv = (char **)malloc((i - start + 1) * sizeof(char *));
            if (!new_process->argv)
            {
                perror("malloc failed for argv");
                free(new_process);
                free(new_job);
                return NULL;
            }

            for (size_t j = start; j < i; j++)
            {
                new_process->argv[j - start] = strdup(tokens->data[j]); // Copy each token
            }
            new_process->argv[i - start] = NULL; // Null-terminate the argv array
            new_process->pid = 0;
            new_process->completed = 0;
            new_process->stopped = 0;
            new_process->status = 0;

            // Link the process to the job
            if (last_process == NULL)
            {
                new_job->first_process = new_process;
            }
            else
            {
                last_process->next = new_process;
            }
            last_process = new_process;

            // Update start index for the next process
            start = i + 1;
        }
    }

    return new_job;
}