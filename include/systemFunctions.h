#ifndef SYSTEM_FUNCTIONS_H
#define SYSTEM_FUNCTIONS_H

char *get_cwd();
void cd(char *path);
void ls();
void clear();
void execute(char *file, char *args[]);


#endif  // SYSTEM_FUNCTIONS_H