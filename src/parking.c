#include <stdio.h>
#include <stdlib.h>
#include "../include/parking.h"

// --- Séquences d'échappement ANSI ---
// Ces séquences permettent de gérer l'affichage figé et les couleurs.
#define RESET "\033[0m"        // Réinitialisation de toutes les couleurs/formats
#define CLEAR_SCREEN "\033[2J" // Efface l'intégralité de l'écran
#define CURSOR_HOME "\033[H"   // Déplace le curseur en haut à gauche

void display_static_map(const char *filename)
{
    FILE *file;
    // Taille du buffer suffisante pour les lignes, y compris les codes ANSI
    char buffer[1024];

    // Ouvre le fichier en mode lecture ("r")
    file = fopen(filename, "r");

    if (file == NULL)
    {
        // En cas d'erreur (chemin incorrect), affiche un message d'erreur
        perror("Erreur: Impossible d'ouvrir le fichier du plan.");
        printf("Vérifiez que 'assets/parking_map.txt' existe bien.\n");
        return;
    }

    // 1. Initialisation de l'affichage (Figé sur l'écran)
    printf(CLEAR_SCREEN);
    printf(CURSOR_HOME);

    // 2. Lit et affiche le contenu du fichier ligne par ligne
    while (fgets(buffer, sizeof(buffer), file) != NULL)
    {
        printf("%s", buffer);
    }

    // 3. Ferme le fichier et réinitialise la couleur du terminal
    fclose(file);
    printf(RESET);
    fflush(stdout); // S'assure que tout est affiché immédiatement
}