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

void execute_pipe(struct dynamicArray *commands) {
    int num_cmds = commands->size;
    if (num_cmds < 2) return;

    int prev_pipe[2];
    int next_pipe[2];
    pid_t pid;
    
    // Create first pipe
    if (pipe(prev_pipe) == -1) {
        perror("pipe failed");
        return;
    }

    // First command (writes to pipe)
    pid = fork();
    if (pid == 0) {
        close(prev_pipe[0]);
        dup2(prev_pipe[1], STDOUT_FILENO);
        close(prev_pipe[1]);
        
        char **args = parse_section(commands->data[0]);
        execvp(args[0], args);
        perror("execvp failed");
        free(args);
        exit(EXIT_FAILURE);
    }
    close(prev_pipe[1]); // Parent closes write end

    // Middle commands (both read and write)
    for (int i = 1; i < num_cmds-1; i++) {
        if (pipe(next_pipe) == -1) {
            perror("pipe failed");
            return;
        }

        pid = fork();
        if (pid == 0) {
            // Connect to previous pipe for input
            dup2(prev_pipe[0], STDIN_FILENO);
            close(prev_pipe[0]);
            
            // Connect to next pipe for output
            dup2(next_pipe[1], STDOUT_FILENO);
            close(next_pipe[1]);
            
            char **args = parse_section(commands->data[i]);
            execvp(args[0], args);
            perror("execvp failed");
            free(args);
            exit(EXIT_FAILURE);
        }
        
        // Clean up previous pipe and move to next
        close(prev_pipe[0]);
        close(next_pipe[1]);
        //update pipes
        prev_pipe[0] = next_pipe[0];
        prev_pipe[1] = next_pipe[1];
    }

    // Last command (reads from pipe)
    pid = fork();
    if (pid == 0) {
        dup2(prev_pipe[0], STDIN_FILENO);
        close(prev_pipe[0]);
        
        char **args = parse_section(commands->data[num_cmds-1]);
        execvp(args[0], args);
        perror("execvp failed");
        free(args);
        exit(EXIT_FAILURE);
    }
    close(prev_pipe[0]);

    // Wait for all children
    for (int i = 0; i < num_cmds; i++) {
        wait(NULL);
    }
}

// Validates pipe position in command
int check_pipe_position(struct dynamicArray *tokens, int pipe_index)
{
    if (pipe_index == 0)
    {
        fprintf(stderr, "Error : Pipe at start of command\n");
        return 0;
    }
    else if (pipe_index == tokens->size - 1)
    {
        fprintf(stderr, "Error : Pipe at end of command\n");
        return 0;
    }
    return 1;
}

// Combines tokens into a command string
void build_section(char *section, size_t section_size, struct dynamicArray *tokens, size_t start, size_t end)
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

// Handles pipe parsing and execution
int handle_pipes(struct dynamicArray *tokens, char *cwd) {
    // Find all pipe positions
    struct dynamicArray pipe_indices = init_array(4);
    for (size_t i = 0; i < tokens->size; i++) {
        if (strcmp(tokens->data[i], "|") == 0) {
            add_element(&pipe_indices, (char*)(long)i);
        }
    }
    if(pipe_indices.size == 0) {
        printf("No pipes found\n");
        free(pipe_indices.data);
        return 0; // No pipes found
    }
    // Validate all pipe positions
    for (size_t i = 0; i < pipe_indices.size; i++) {
        int idx = (int)(long)pipe_indices.data[i];
        if (!check_pipe_position(tokens, idx)) {
            free(pipe_indices.data);
            return -1;
        }
    }

    // Split the command into parts between pipes //
    // Create array to store command sections (need +1 because N pipes means N+1 commands)
    struct dynamicArray command_list = init_array(pipe_indices.size + 1);

    size_t current_position = 0; // Start at beginning of tokens

    // For each pipe position (plus one extra for the last command)
    for (size_t i = 0; i <= pipe_indices.size; i++) {
        // Find where this command ends
        size_t end_position;
        if (i < pipe_indices.size) {
            // End at the next pipe
            end_position = (size_t)(long)pipe_indices.data[i];
        } else {
            // For last command, end at the end of tokens
            end_position = tokens->size;
        }

        // Make a buffer to hold this command section
        char command_buffer[1024] = {0}; // Initialize to empty string
    
        // Combine all tokens between current_position and end_position
        build_section(command_buffer, sizeof(command_buffer), 
                    tokens, current_position, end_position);
    
        // Add this command to our array (strdup makes a copy)
        add_element(&command_list, strdup(command_buffer));
    
        // Move past this command and the pipe
        current_position = end_position + 1;
    }

    // Execute the pipeline
    execute_pipe(&command_list);

    // Cleanup
    for (size_t i = 0; i < command_list.size; i++) {
        free(command_list.data[i]);
    }
    free(command_list.data);
    free(pipe_indices.data);

    return 1;
}