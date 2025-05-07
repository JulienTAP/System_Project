/**
 * @file input.c
 * @brief Input parsing and job creation for shell commands.
 * 
 * Provides functionality to parse user input into tokens and create job structures
 * from these tokens, handling both foreground and background processes as well as
 * command pipelines.
 */

#include "input.h"
#include "dynamicArray.h"
#include "structures.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

/**
 * @brief Parses user input into tokens.
 * 
 * Reads input from stdin and splits it into tokens using spaces as delimiters.
 * The resulting tokens are stored in a dynamic array for further processing.
 * 
 * @param tokens Pointer to a dynamic array where tokens will be stored
 * @param input Buffer to store the raw input string
 * @param size Size of the input buffer
 */
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


/**
 * @brief Creates a job structure from parsed tokens.
 * 
 * Processes the token array to create a job structure containing one or more
 * processes (for pipelined commands). Handles file descriptor redirection
 * and command argument processing.
 * 
 * @param tokens Pointer to dynamic array containing command tokens
 * @param fds Array containing input/output file descriptors [stdin, stdout]
 * @return Pointer to the created job structure, or NULL on failure
 * 
 * @note The caller is responsible for freeing the returned job structure
 *       and all its associated memory.
 */
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
    new_job->pgid = 0; 
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

/**
 * @brief Determines if a command should run in background.
 * 
 * Checks the token array for a trailing '&' character indicating
 * the command should run as a background job.
 * 
 * @param tokens Pointer to dynamic array containing command tokens
 * @return true if the command should run in background, false otherwise
 */
bool is_background_job(struct dynamicArray *tokens)
{
    if (tokens->size > 0 && strcmp(tokens->data[tokens->size - 1], "&") == 0)
    {
        // Remove the '&' token from the tokens array
        pop_element(tokens, tokens->size-1);
        return true; // Job is a background job
    }
    return false; // Job is a foreground job
}