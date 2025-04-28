#include "systemFunctions.h"
#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

void clear()
{
    // Clear the terminal screen
    if (system("clear") == -1)
    {
        perror("clear failed");
    }
}

char *get_cwd()
{
    char *cwd = getcwd(NULL, 0);
    if (cwd == NULL)
    {
        perror("getcwd failed");
        return NULL;
    }
    return cwd;
}

void cd(char *path)
{
    if (path == NULL)
    {
        chdir(getenv("HOME")); // Change to home directory
    }
    else if (chdir(path) != 0)
    {
        perror("cd failed");
    }
}

void ls()
{
    system("ls");
}

void execute(char *file, char *args[])
{
    pid_t pid = fork();
    if (pid == 0)
    {
        // Child process
        execvp(file, args);
        perror("exec failed");
        exit(EXIT_FAILURE);
    }
    else if (pid < 0)
    {
        perror("fork failed");
    }
    else
    {
        // Parent process
        wait(NULL); // Wait for child process to finish
    }
}