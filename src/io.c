#include "io.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>


int *handle_redirection(struct dynamicArray *tokens)
{
    // Allocate an array to store infile and outfile descriptors
    int *fds = (int *)malloc(2 * sizeof(int));
    if (!fds)
    {
        perror("malloc failed for file descriptors");
        return NULL;
    }

    // Initialize infile and outfile to default (stdin and stdout)
    fds[0] = STDIN_FILENO;  // infile
    fds[1] = STDOUT_FILENO; // outfile

    for (size_t i = 0; i < tokens->size; i++)
    {
        if (strcmp(tokens->data[i], "<") == 0 && i + 1 < tokens->size)
        {
            // Open the input file
            fds[0] = open(tokens->data[i + 1], O_RDONLY);
            if (fds[0] < 0)
            {
                perror("Failed to open input file");
                free(fds);
                return NULL;
            }

            // Remove the redirection token and its argument from the array
            for (size_t j = i; j < tokens->size - 2; j++)
            {
                tokens->data[j] = tokens->data[j + 2];
            }
            tokens->size -= 2;
            i--; // Adjust index to account for removed elements
        }
        else if (strcmp(tokens->data[i], ">") == 0 && i + 1 < tokens->size)
        {
            // Open the output file
            fds[1] = open(tokens->data[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fds[1] < 0)
            {
                perror("Failed to open output file");
                free(fds);
                return NULL;
            }

            // Remove the redirection token and its argument from the array
            for (size_t j = i; j < tokens->size - 2; j++)
            {
                tokens->data[j] = tokens->data[j + 2];
            }
            tokens->size -= 2;
            i--; // Adjust index to account for removed elements
        }
    }

    return fds;
}
