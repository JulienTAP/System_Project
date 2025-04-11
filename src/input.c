#include "input.h"
#include "es.h"
#include "es.c"
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

void handle_cd(char *tokens[100]){
    if(tokens[1] == NULL) {
        chdir(getenv("HOME")); // Change to home directory
    } else if (chdir(tokens[1]) != 0) {
        perror("cd failed");
    }
}

void handle_ls(char *tokens[100]) {
    char *sortie = NULL;
    
    //  On cherche une redirection de sortie
    for (int i = 0; tokens[i] != NULL; i++) {
        if (strcmp(tokens[i], ">") == 0 && tokens[i+1] != NULL) {
            sortie = tokens[i+1];  // On stocke le nom du fichier de sortie
            tokens[i] = NULL;  // On tronque la commande pour system()
            break;  // On s'arrête après avoir trouvé >
        }
    }

    // Cas sans redirection
    if (sortie == NULL) {
        system("ls");  // Exécution simple de ls
        return;
    }

    // Cas avec redirection
    pid_t pid = fork();
    if (pid == 0) {  // Processus enfant
        // On redirige la sortie vers le fichier
        if (rediriger_sortie(sortie) == -1) {
            exit(EXIT_FAILURE);  // Échec de la redirection
        }
        
        system("ls");  // On exécute ls (la sortie ira dans le fichier)
        exit(0);  // Fin du processus enfant
    } 
    else if (pid > 0) {  // Processus parent
        wait(NULL);  // On attend que le ls se termine
    } 
    else {
        perror("Erreur de fork");  // Échec du fork
    }
}

void handle_run(char *tokens[100]) {
    // On prépare deux variables pour stocker les noms des fichiers
    // d'entrée et de sortie s'il y a des redirections
    char *source = NULL;
    char *sortie = NULL;
    // Le premier élément du tableau est toujours la commande à exécuter
    char *command = tokens[0];
    
    // On parcourt tous les éléments de la commande
    for (int i = 0; tokens[i] != NULL; i++) {
        // Si on trouve le symbole <, ça veut dire qu'on veut rediriger l'entrée
        if (tokens[i+1] == NULL) continue;
        if (strcmp(tokens[i], "<") == 0 ) {
            // On garde le nom du fichier qui suit le <
            source = tokens[i+1];
            // On met NULL à la place du < pour que execvp fonctionne 
            tokens[i] = "NULL";
        } 
        // Si on trouve le symbole >, ça veut dire qu'on veut rediriger la sortie
        else if (strcmp(tokens[i], ">") == 0 && tokens[i+1] != NULL) {
            sortie = tokens[i+1];
            tokens[i] = NULL;
        }
    }

    // On crée un nouveau processus pour exécuter la commande
    pid_t pid = fork();
    
    // Si on est dans le processus enfant
    if (pid == 0) {

        // Si on a un fichier d'entrée, on fait la redirection
        if (source && rediriger_entre(source) == -1) {
            exit(EXIT_FAILURE); // Si ça échoue, on quitte
        }
        
        // Si on a un fichier de sortie, on fait la redirection
        if (sortie && rediriger_sortie(sortie) == -1) {
            fprintf(stderr, "Échec redirection entrée: %s\n", source);
            exit(EXIT_FAILURE); // Si ça échoue, on quitte
        }

        // On exécute la commande avec les arguments
        execvp(tokens[1], &tokens[1]);

        // Si execvp échoue, on affiche l'erreur et on quitte
        perror("Échec execvp");
        exit(1);

    } 

    // Si fork a échoué
    else if (pid < 0) {
        perror("fork failed");
    } 
    
    // Si on est dans le processus parent
    else {
        // On attend que le processus enfant finisse
        wait(NULL);
    }
}





