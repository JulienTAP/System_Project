#include <stdio.h>
#include <unistd.h>

int main() {
    // Écriture sur la sortie standard (stdout) pour le mettre dans un fichier txt
    write(STDOUT_FILENO, "123456789\n", 9);  
    printf("123456789");
    return 0;
}