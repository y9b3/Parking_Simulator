#include <stdio.h>
#include "../include/parking.h"

int main()
{

    // Le chemin d'accès au fichier map (depuis la racine où l'exécutable est lancé)
    const char *map_path = "assets/parking_map.txt";

    // Affiche le plan statique
    display_static_map(map_path);

    // Garde le terminal ouvert pour voir la carte
    printf("\nPlan affiché. Appuyez sur Entrée pour quitter...\n");
    getchar();

    return 0;
}