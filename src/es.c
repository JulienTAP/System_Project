#include "es.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>    
#include <fcntl.h>


int redirect_input(const char *source) {
    // Open the source file for reading only
    int new_input = open(source, O_RDONLY);
    if (new_input < 0) {
        perror("Failed to open source file: ");
        return -1;
    }
    
    // Replace stdin with the source file
    if (dup2(new_input, STDIN_FILENO) < 0) {
        perror("Failed to redirect input: ");
        close(new_input);
        return -1;
    }

    // Close the now-unnecessary file descriptor
    close(new_input);
    return 0;
}


int redirect_output(const char *output) {
    // Open or create the output file for writing
    int new_output = open(output, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (new_output < 0) {
        perror("Failed to open output file: ");
        return -1;
    }
    
    // Replace stdout with the output file
    if (dup2(new_output, STDOUT_FILENO) < 0) {
        perror("Failed to redirect output: ");
        close(new_output);
        return -1;
    }

    // Close the now-unnecessary file descriptor
    close(new_output);
    return 0;
}
