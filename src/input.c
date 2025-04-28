#include "input.h"
#include "io.h"
#include "io.c"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>    
#include <fcntl.h>

void read_input(char *input, size_t size) {
    if (fgets(input, size, stdin) != NULL) {
        // Remove newline character from input
        input[strcspn(input, "\n")] = 0;
    } else {
        printf("Error reading input.\n");
        input[0] = '\0'; // Set input to an empty string in case of error
    }
}

void parse_input(char **tokens, char *input, size_t size) {
    char *token;
    size_t index = 0;

    token = strtok(input, " ");
    while (token != NULL && index < size) {
        tokens[index++] = token;
        token = strtok(NULL, " ");
    }
}

void handle_cd(char *tokens[100]) {
    if (tokens[1] == NULL) {
        chdir(getenv("HOME")); // Change to home directory
    } else if (chdir(tokens[1]) != 0) {
        perror("cd failed");
    }
}

void handle_ls_parse(char *tokens[100]) {
    char *output = NULL;
    
    // Search for output redirection
    for (int i = 0; tokens[i] != NULL; i++) {
        if (strcmp(tokens[i], ">") == 0 && tokens[i+1] != NULL) {
            output = tokens[i+1];  // Store the name of the output file
            tokens[i] = NULL;      // Truncate the command for system()
            break;                 // Stop after finding '>'
        }
    }

    // Case without redirection
    if (output == NULL) {
        system("ls");  // Simple execution of ls
        return;
    }

    // Case with redirection
    pid_t pid = fork();
    if (pid == 0) {  // Child process
        // Redirect output to the file
        if (redirect_output(output) == -1) {
            exit(EXIT_FAILURE);  // Redirection failed
        }
        
        system("ls");  // Execute ls (output goes to the file)
        exit(0);       // End of child process
    } 
    else if (pid > 0) {  // Parent process
        wait(NULL);  // Wait for ls to finish
    } 
    else {
        perror("fork error");  // Fork failed
    }
}

void handle_ls(char *tokens[100]) {
    system("ls");
}

void handle_run_parse(char *tokens[100]) {
    // Prepare two variables to store input and output file names if redirections exist
    char *input_file = NULL;
    char *output_file = NULL;
    // The first element of the array is always the command to execute
    char *command = tokens[0];
    
    // Loop through all elements of the command
    for (int i = 0; tokens[i] != NULL; i++) {
        if (tokens[i+1] == NULL) continue;
        // If we find the '<' symbol, it means we want to redirect input
        if (strcmp(tokens[i], "<") == 0) {
            input_file = tokens[i+1];  // Save the name of the file after '<'
            tokens[i] = "NULL";        // Replace '<' with NULL so execvp works correctly
        } 
        // If we find the '>' symbol, it means we want to redirect output
        else if (strcmp(tokens[i], ">") == 0 && tokens[i+1] != NULL) {
            output_file = tokens[i+1];
            tokens[i] = NULL;
        }
    }

    // Create a new process to execute the command
    pid_t pid = fork();
    
    if (pid == 0) {  // Child process
        // If there is an input file, perform input redirection
        if (input_file && redirect_input(input_file) == -1) {
            exit(EXIT_FAILURE); // Exit if redirection fails
        }
        
        // If there is an output file, perform output redirection
        if (output_file && redirect_output(output_file) == -1) {
            fprintf(stderr, "Input redirection failed: %s\n", input_file);
            exit(EXIT_FAILURE); // Exit if redirection fails
        }

        // Execute the command with its arguments
        execvp(tokens[1], &tokens[1]);

        // If execvp fails, print the error and exit
        perror("execvp failed");
        exit(1);

    } else if (pid < 0) {  // Fork failed
        perror("fork failed");
    } else {  // Parent process
        wait(NULL); // Wait for the child process to finish
    }
}

void handle_run(char *tokens[100]) {
    // Execute the command stored in tokens[1]
    if (tokens[1] != NULL) {
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            execvp(tokens[1], &tokens[1]);
            perror("exec failed");
            exit(1);
        } else if (pid < 0) {
            perror("fork failed");
        } else {
            // Parent process
            wait(NULL); // Wait for the child process to finish
        }
    } else {
        printf("No command provided after './'\n");
    }
}
