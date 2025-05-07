#ifndef SYSTEM_FUNCTIONS_H
#define SYSTEM_FUNCTIONS_H

#include "dynamicArray.h"

void clear();

char *get_cwd();

void cd(char *path);

void ls();

void execute(char *file, char *args[]);

void printFilePermissions(char *file);

int copyFilePermissions(char *src, char *dest);

int copyFile(char *src, char *dest);

int copyDirectory(char *src, char *dest);

void cp(struct dynamicArray *Array);

#endif // SYSTEM_FUNCTIONS_H