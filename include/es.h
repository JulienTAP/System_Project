#ifndef ES_H
#define ES_H

#include <unistd.h>
#include <fcntl.h>

int rediriger_entre(const char *input_source);
int rediriger_sortie(const char *output_target);

#endif // REDIRECT_H