#ifndef SYSTEM_FUNCTIONS_H
#define SYSTEM_FUNCTIONS_H

void clear();
char *get_cwd();
void cd(char *path);
void ls();
void execute(char *file, char *args[]);

#endif // SYSTEM_FUNCTIONS_H