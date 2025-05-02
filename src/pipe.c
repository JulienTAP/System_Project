#include "pipe.h"
#include "dynamicArray.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

// Splits section string into executable arguments
char **parse_section(char *cmd)
{
    // Allocate space for max 31 arguments + NULL terminator
    char **args = malloc(32 * sizeof(char *));
    int arg_count = 0;

    // Tokenize command on spaces
    char *token = strtok(cmd, " ");

    // Build arguments array
    while (token != NULL && arg_count < 31)
    {
        args[arg_count++] = token;
        token = strtok(NULL, " ");
    }

    // NULL-terminate for execvp
    args[arg_count] = NULL;
    return args;
}

// Executes two commands connected by a pipe
void execute_pipe(struct dynamicArray *commands)
{
    int fd[2]; // pipe descriptors

    // Create pipe for inter-process communication
    if (pipe(fd) == -1)
    {
        perror("pipe creation failed");
        return;
    }

    // Fork left process (writer)
    pid_t left_pid = fork();
    if (left_pid == 0)
    {
        // Left process setup:
        close(fd[0]); // Close unused read end

        // Redirect stdout to pipe write end
        if (dup2(fd[1], STDOUT_FILENO) == -1)
        {
            perror("dup2 left failed");
            exit(EXIT_FAILURE);
        }
        close(fd[1]); // Clean up original fd

        // Execute left command
        char **left_args = parse_section(commands->data[0]);
        execvp(left_args[0], left_args);
        perror("execvp left failed");
        free(left_args);
        exit(EXIT_FAILURE);
    }

    // Fork right process (reader)
    pid_t right_pid = fork();
    if (right_pid == 0)
    {
        // Right process setup:
        close(fd[1]); // Close unused write end

        // Redirect stdin from pipe read end
        if (dup2(fd[0], STDIN_FILENO) == -1)
        {
            perror("dup2 right failed");
            exit(EXIT_FAILURE);
        }
        close(fd[0]); // Clean up original fd

        // Execute right command
        char **right_args = parse_section(commands->data[1]);
        execvp(right_args[0], right_args);
        perror("execvp right failed");
        free(right_args);
        exit(EXIT_FAILURE);
    }

    // Parent process cleanup
    close(fd[0]); // Close pipe ends
    close(fd[1]);

    // Wait for child processes
    waitpid(left_pid, NULL, 0);
    waitpid(right_pid, NULL, 0);
}

// Validates pipe position in command
int check_pipe_position(struct dynamicArray *tokens, int pipe_index)
{
    if (pipe_index == 0 || pipe_index == tokens->size - 1)
    {
        fprintf(stderr, "Error: Pipe at start or end of command\n");
        return 0;
    }
    return 1;
}

// Combines tokens into a command string
void build_section(char *section, size_t section_size,
                   struct dynamicArray *tokens,
                   size_t start, size_t end)
{
    section[0] = '\0'; // Initialize buffer

    // Concatenate tokens with spaces
    for (size_t i = start; i < end; i++)
    {
        if (i > start)
        {
            strncat(section, " ", section_size - strlen(section) - 1);
        }
        strncat(section, tokens->data[i], section_size - strlen(section) - 1);
    }
}

// Main pipe handling function
int handle_pipes(struct dynamicArray *tokens, char *cwd)
{
    // Locate pipe symbol in tokens
    int pipe_index = -1;
    for (size_t i = 0; i < tokens->size; i++)
    {
        if (strcmp(tokens->data[i], "|") == 0)
        {
            pipe_index = i;
            break;
        }
    }

    if (pipe_index == -1)
        return 0; // Early exit if no pipe

    // Validate pipe position
    if (!check_pipe_position(tokens, pipe_index))
    {
        return -1;
    }

    // Build left and right sections strings
    char left_section[1024], right_section[1024];
    build_section(left_section, sizeof(left_section), tokens, 0, pipe_index);
    build_section(right_section, sizeof(right_section), tokens, pipe_index + 1, tokens->size);

    // Prepare and execute piped commands
    struct dynamicArray pipe_commands = init_array(2);
    add_element(&pipe_commands, left_section);
    add_element(&pipe_commands, right_section);
    execute_pipe(&pipe_commands);
    free(pipe_commands.data);

    return 1; // Success
}