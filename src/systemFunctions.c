#define _GNU_SOURCE

#include "systemFunctions.h"
#include "input.h"
#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#include "dynamicArray.h"

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

/**
 * @brief Prints the permissions of a file.
 *
 * This function retrieves and prints the permissions of the specified file.
 *
 * @param file The path to the file.
 */
void printFilePermissions(char *file)
{
    struct stat file_stat;
    // Get the file permissions
    if (stat(file, &file_stat) < 0)
    {
        perror("Error getting file permissions");
        return;
    }

    // Print the file permissions
    printf("File %s permissions: ", file);
    printf((S_ISDIR(file_stat.st_mode)) ? "d" : "-");
    printf((file_stat.st_mode & S_IRUSR) ? "r" : "-");
    printf((file_stat.st_mode & S_IWUSR) ? "w" : "-");
    printf((file_stat.st_mode & S_IXUSR) ? "x" : "-");
    printf((file_stat.st_mode & S_IRGRP) ? "r" : "-");
    printf((file_stat.st_mode & S_IWGRP) ? "w" : "-");
    printf((file_stat.st_mode & S_IXGRP) ? "x" : "-");
    printf((file_stat.st_mode & S_IROTH) ? "r" : "-");
    printf((file_stat.st_mode & S_IWOTH) ? "w" : "-");
    printf((file_stat.st_mode & S_IXOTH) ? "x" : "-");
    printf("\n");
}

/**
 * @brief Copies the permissions from a source file to a destination file.
 *
 * This function retrieves the permissions of the source file and applies them to the destination file.
 * If any error occurs during the process, it prints an error message and returns -1.
 *
 * @param src The path to the source file.
 * @param dest The path to the destination file.
 * @return 0 on success, -1 on failure.
 */
int copyFilePermissions(char *src, char *dest)
{
    struct stat src_stat;
    // Get the source file permissions
    if (stat(src, &src_stat) < 0)
    {
        perror("Error getting source file permissions");
        return -1;
    }

    // Set the destination file permissions
    if (chmod(dest, src_stat.st_mode) < 0)
    {
        perror("Error setting destination file permissions");
        return -1;
    }

    printf("File permissions copied successfully\n");
    return 0;
}

/**
 * @brief Copies the contents of a source file to a destination file.
 *
 * This function opens the source file for reading and the destination file for writing.
 * It reads the contents of the source file in chunks and writes them to the destination file.
 * It also copies the file permissions from the source file to the destination file.
 * If any error occurs during the process, it prints an error message and returns -1.
 *
 * @param src The path to the source file.
 * @param dest The path to the destination file.
 * @return 0 on success, -1 on failure.
 */
int copyFile(char *src, char *dest)
{
    int src_desc = open(src, O_RDONLY);
    // Check if the source file was opened successfully
    if (src_desc < 0)
    {
        perror("Error opening source file");
        return -1;
    }

    int dest_desc = open(dest, O_WRONLY | O_CREAT, 0666);
    // Check if the destination file was opened/created successfully
    if (dest_desc < 0)
    {
        perror("Error opening/creating destination file");
        close(src_desc);
        return -1;
    }

    if (copyFilePermissions(src, dest) < 0)
    {
        perror("Error copying file permissions");
        close(src_desc);
        close(dest_desc);
        return -1;
    }

    struct stat src_stat;
    // Check if the source file size can be retrieved
    if (stat(src, &src_stat) < 0)
    {
        perror("Error getting source file size");
        return -1;
    }
    off_t src_size = src_stat.st_size;

    // Copy the contents of the source file to the destination file
    copy_file_range(src_desc, NULL, dest_desc, NULL, src_size, 0);

    close(src_desc);
    close(dest_desc);

    printf("File copied successfully\n");

    return 0;
}

/**
 * @brief Copies the contents of a source directory to a destination directory.
 *
 * This function opens the source directory and iterates through its entries.
 * For each entry, it recursively copies directories and files.
 * If any error occurs during the process, it prints an error message and returns -1.
 *
 * @param src The path to the source directory.
 * @param dest The path to the destination directory.
 * @return 0 on success, -1 on failure.
 */
int copyDirectory(char *src, char *dest)
{
    DIR *src_dir = opendir(src);
    if (src_dir == NULL)
    {
        perror("Error opening source directory");
        return -1;
    }

    struct stat dest_stat;
    // Check if the destination directory exists
    if (stat(dest, &dest_stat) < 0)
    {
        // Destination directory does not exist, create it
        // Check if the destination directory was created successfully
        if (mkdir(dest, 0777) < 0)
        {
            perror("Error creating destination directory");
            closedir(src_dir);
            return -1;
        }
    }
    else if (!S_ISDIR(dest_stat.st_mode))
    {
        // Destination exists but is not a directory
        perror("Destination exists and is not a directory");
        closedir(src_dir);
        return -1;
    }

    struct dirent *dir_entry;
    // Copy the contents of the source directory to the destination directory
    while ((dir_entry = readdir(src_dir)) != NULL)
    {
        // Skip the current and parent directories
        if (strcmp(dir_entry->d_name, ".") == 0 || strcmp(dir_entry->d_name, "..") == 0)
        {
            continue;
        }

        int buffer_size = 4096;

        char src_entry[buffer_size];
        snprintf(src_entry, buffer_size, "%s/%s", src, dir_entry->d_name);

        char dest_entry[buffer_size];
        snprintf(dest_entry, buffer_size, "%s/%s", dest, dir_entry->d_name);

        struct stat entry_stat;
        // Check if the source entry permissions can be retrieved
        if (stat(src_entry, &entry_stat) < 0)
        {
            perror("Error getting source entry permissions");
            closedir(src_dir);
            return -1;
        }

        // Recursively copy directories and files
        if (S_ISDIR(entry_stat.st_mode))
        {
            // The entry is a directory
            // Check if the copy operation was successful
            if (copyDirectory(src_entry, dest_entry) < 0)
            {
                perror("Error copying directory");
                closedir(src_dir);
                return -1;
            }
        }
        else
        {
            // The entry is a file
            // Check if the copy operation was successful
            if (copyFile(src_entry, dest_entry) < 0)
            {
                perror("Error copying file");
                closedir(src_dir);
                return -1;
            }
        }
    }

    closedir(src_dir);

    printf("Directory copied successfully\n");

    return 0;
}

/**
 * @brief Copies one or multiple files or directories from source to destination.
 *
 * This function checks for each source if it is a file or directory and calls the appropriate copy function.
 * If the source is neither, it prints an error message.
 *
 * @param src The paths to the source files or directories.
 * @param dest The path to the destination file or directory.
 */
void cp(char *src, char *dest)
{
    struct stat dest_stat;
    // Check if the destination exists and is a directory
    if (stat(dest, &dest_stat) < 0)
    {
        // Destination does not exist, create it
        // Check if the destination directory was created successfully
        if (mkdir(dest, 0777) < 0)
        {
            perror("Error creating destination directory");
            return;
        }
        printf("Destination directory created successfully\n");
    }
    else if (!S_ISDIR(dest_stat.st_mode))
    {
        // Destination exists but is not a directory
        perror("Destination exists but is not a directory");
        return;
    }

    struct dynamicArray src_tokens = init_array(8); // Initialize dynamic array for tokens
    char *token = strtok(src, " ");
    while (token != NULL)
    {
        add_element(&src_tokens, token); // Add token to dynamic array
        token = strtok(NULL, " ");
    }

    // Debuging: Print the source tokens
    printf("Source tokens:\n");
    for (size_t i = 0; i < src_tokens.size; i++)
    {
        printf("%s\n", src_tokens.data[i]);
    }

    // Iterate through each source token
    for (size_t i = 0; i < src_tokens.size; i++)
    {
        char *src_path = src_tokens.data[i];
        struct stat src_stat;
        // Check if the source entry permissions can be retrieved
        if (stat(src_path, &src_stat) < 0)
        {
            perror("Error getting source entry permissions");
            continue;
        }

        // Check if the source is a file or directory
        if (S_ISDIR(src_stat.st_mode))
        {
            // The source is a directory
            // Check if the copy operation was successful
            if (copyDirectory(src_path, dest) < 0)
            {
                perror("Error copying directory");
                continue;
            }
        }
        else
        {
            // The source is a file
            // Check if the copy operation was successful
            if (copyFile(src_path, dest) < 0)
            {
                perror("Error copying file");
                continue;
            }
        }
    }
    printf("Copy operation completed successfully\n");
    free(src_tokens.data); // Free command tokens
}