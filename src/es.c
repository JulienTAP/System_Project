#include "es.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>    
#include <fcntl.h>

int rediriger_entre(const char *source){
    //Ouvrir le fichier source pour lecture unniquement
    int nouv_entre = open(source, O_RDONLY);
    if (nouv_entre < 0) {
        perror("Failed to open source file : ");
        return -1;
    }
    
    // Remplacer stdin par le fichier source
    if (dup2(nouv_entre, STDIN_FILENO) < 0) {
        perror("Failed to change entry : ");
        close(nouv_entre);
        return -1;
    }

    //Fermer le descripteur inutile
    close(nouv_entre);
    return 0;
}

int rediriger_sortie(const char *sortie){
    // Ouvrir ou créer le fichier de sortie en écriture
    int nouv_sortie = open(sortie, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (nouv_sortie < 0) {
        perror("Failed to open output file : ");
        return -1;
    }
    
    //Remplacer stdout par le fichier de sortie
    if (dup2(nouv_sortie, STDOUT_FILENO) < 0) {
        perror("Failed to change output : ");
        close(nouv_sortie);
        return -1;
    }

    close(nouv_sortie);
    return 0;
}
