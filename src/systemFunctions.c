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
 * @brief Copies the permissions from a source to a destination.
 *
 * This function retrieves the permissions of the source and applies them to the destination.
 * If any error occurs during the process, it prints an error message and returns -1.
 *
 * @param src The path to the source.
 * @param dest The path to the destination.
 * @return 0 on success, -1 on failure.
 */
int copyPermissions(char *src, char *dest)
{
    struct stat src_stat;
    // Get the source file permissions
    if (stat(src, &src_stat) < 0)
    {
        perror("Error getting source permissions");
        return -1;
    }

    // Set the destination file permissions
    if (chmod(dest, src_stat.st_mode) < 0)
    {
        perror("Error setting destination permissions");
        return -1;
    }

    printf("Permissions copied successfully\n");
    return 0;
}

/**
 * @brief Copies a file from the source path to the destination directory.
 *
 * This function reads the contents of the source file and writes them to a new file
 * in the destination directory. It also preserves the file permissions.
 *
 * @param src_file The path to the source file.
 * @param dest_directory The path to the destination directory.
 * @return 0 on success, -1 on failure.
 */
int copyFile(char *src_file, char *dest_directory)
{
    size_t buffer_size = 65536; // Define buffer size
    char buffer[buffer_size];

    // Open the source file for reading
    int src_fd = open(src_file, O_RDONLY);
    if (src_fd < 0)
    {
        perror("Error opening source file");
        return -1;
    }

    // Construct the destination file path
    char dest_file[buffer_size];
    snprintf(dest_file, buffer_size, "%s/%s", dest_directory, strrchr(src_file, '/') ? strrchr(src_file, '/') + 1 : src_file);

    // Open the destination file for writing (create if it doesn't exist)
    int dest_fd = open(dest_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (dest_fd < 0)
    {
        perror("Error opening destination file");
        close(src_fd);
        return -1;
    }

    // Copy the contents from the source file to the destination file
    ssize_t bytes_read, bytes_written;
    while ((bytes_read = read(src_fd, buffer, buffer_size)) > 0)
    {
        bytes_written = write(dest_fd, buffer, bytes_read);
        if (bytes_written < 0)
        {
            perror("Error writing to destination file");
            close(src_fd);
            close(dest_fd);
            return -1;
        }
    }

    if (bytes_read < 0)
    {
        perror("Error reading source file");
        close(src_fd);
        close(dest_fd);
        return -1;
    }

    // Close the file descriptors
    close(src_fd);
    close(dest_fd);

    // Copy the file permissions from the source file to the destination file
    if (copyPermissions(src_file, dest_file) < 0)
    {
        perror("Error copying file permissions");
        return -1;
    }

    printf("File copied successfully: %s -> %s\n", src_file, dest_file);
    return 0;
}

/**
 * @brief Copies a directory from the source path to the destination path.
 *
 * This function recursively copies the contents of the source directory to a new directory
 * at the destination path. It preserves the permissions of the source directory and its contents.
 *
 * @param srcDirectory The path to the source directory.
 * @param destDirectory The path to the destination directory.
 * @return 0 on success, -1 on failure.
 */
int copyDirectory(char *srcDirectory, char *destDirectory)
{
    DIR *srcDir = opendir(srcDirectory);
    if (srcDir == NULL)
    {
        perror("Error opening source directory");
        return -1;
    }

    // Construct the destination directory path
    char destDirPath[PATH_MAX];
    snprintf(destDirPath, PATH_MAX, "%s/%s", destDirectory, strrchr(srcDirectory, '/') ? strrchr(srcDirectory, '/') + 1 : srcDirectory);

    // Create the destination directory
    if (mkdir(destDirPath, 0777) < 0)
    {
        perror("Error creating destination directory");
        closedir(srcDir);
        return -1;
    }

    struct dirent *entry;
    while ((entry = readdir(srcDir)) != NULL)
    {
        // Skip "." and ".." entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        // Construct the source and destination paths for the current entry
        char srcPath[PATH_MAX];
        char destPath[PATH_MAX];
        snprintf(srcPath, PATH_MAX, "%s/%s", srcDirectory, entry->d_name);
        strncpy(destPath, destDirPath, PATH_MAX - 1);
        destPath[PATH_MAX - 1] = '\0'; // Ensure null termination
        strncat(destPath, "/", PATH_MAX - strlen(destPath) - 1);
        strncat(destPath, entry->d_name, PATH_MAX - strlen(destPath) - 1);

        struct stat entryStat;
        if (stat(srcPath, &entryStat) < 0)
        {
            perror("Error getting entry permissions");
            closedir(srcDir);
            return -1;
        }

        if (S_ISDIR(entryStat.st_mode))
        {
            // Recursively copy subdirectory
            if (copyDirectory(srcPath, destDirPath) < 0)
            {
                closedir(srcDir);
                return -1;
            }
        }
        else
        {
            // Copy file
            if (copyFile(srcPath, destDirPath) < 0)
            {
                closedir(srcDir);
                return -1;
            }
        }
    }

    closedir(srcDir);

    // Copy the permissions of the source directory to the destination directory
    if (copyPermissions(srcDirectory, destDirPath) < 0)
    {
        perror("Error copying directory permissions");
        return -1;
    }

    printf("Directory copied successfully: %s -> %s\n", srcDirectory, destDirPath);
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
void cp(struct dynamicArray *Array)
{
    if (Array->size < 2)
    {
        fprintf(stderr, "Usage: cp <source>... <destination>\n");
        return;
    }

    char *dest = Array->data[Array->size - 1]; // Get the destination path
    pop_element(Array, Array->size - 1);       // Remove the destination from the array

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

    // Iterate through each source token
    for (size_t i = 0; i < Array->size; i++)
    {
        char *src_path = Array->data[i];
        struct stat src_stat;
        // Check if the source entry permissions can be retrieved
        if (stat(src_path, &src_stat) < 0)
        {
            fprintf(stderr, "Error getting source entry permissions for file: %s\n", src_path);
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
    free(Array->data); // Free command tokens
}