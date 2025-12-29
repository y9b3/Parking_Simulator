#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include "../include/parking.h"

// (Copie de la fonction key_pressed() du PDF)
char key_pressed()
{
    struct termios oldterm, newterm;
    int oldfd;
    char c, result = 0;
    tcgetattr(STDIN_FILENO, &oldterm);
    newterm = oldterm;
    newterm.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newterm);
    oldfd = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldfd | O_NONBLOCK);
    c = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldterm);
    fcntl(STDIN_FILENO, F_SETFL, oldfd);
    if (c != EOF)
    {
        ungetc(c, stdin);
        result = getchar();
    }
    return result;
}

int main()
{
    const char *map_path = "assets/parking_map.txt";

    display_static_map(map_path);
    init_spots();

    int selected_spot_index = 0;
    draw_all_spots(selected_spot_index);

    // CHANGEMENT: Afficher les messages plus bas
    goto_xy(0, 14); // Anciennement 10
    printf("Utilisez ZQSD pour déplacer, ESPACE pour changer l'état, 'e' pour quitter.\n");

    char key = 0;
    while (key != 'e')
    {
        key = key_pressed();

        if (key != 0)
        {
            int old_selection = selected_spot_index;
            int selection_changed = 0;

            // --- CHANGEMENT DE LA LOGIQUE ZQSD ---
            // La grille est maintenant de 8 colonnes x 4 rangées
            switch (key)
            {
            case 'q': // Gauche
                // On ne peut pas aller à gauche si on est sur la colonne 0 (index % 8 == 0)
                if (selected_spot_index % 8 != 0)
                {
                    selected_spot_index--;
                    selection_changed = 1;
                }
                break;
            case 'd': // Droite
                // On ne peut pas aller à droite si on est sur la colonne 7 (index % 8 == 7)
                if (selected_spot_index % 8 != 7)
                {
                    selected_spot_index++;
                    selection_changed = 1;
                }
                break;
            case 'z': // Haut
                // On ne peut pas monter si on est sur la première rangée (index < 8)
                if (selected_spot_index >= 8)
                {
                    selected_spot_index -= 8; // On saute 8 places en arrière
                    selection_changed = 1;
                }
                break;
            case 's': // Bas
                // On ne peut pas descendre si on est sur la dernière rangée (index >= 24)
                if (selected_spot_index < 24)
                {                             // (32 places - 8 = 24)
                    selected_spot_index += 8; // On saute 8 places en avant
                    selection_changed = 1;
                }
                break;

            case ' ': // Espace pour basculer
                all_spots[selected_spot_index].is_occupied = !all_spots[selected_spot_index].is_occupied;
                draw_spot(all_spots[selected_spot_index], 1);
                break;
            }

            if (selection_changed)
            {
                draw_spot(all_spots[old_selection], 0);
                draw_spot(all_spots[selected_spot_index], 1);
            }

            // CHANGEMENT: Afficher les messages plus bas
            goto_xy(0, 15); // Anciennement 11
            printf("Place sélectionnée: %d   ", selected_spot_index);
            fflush(stdout);
        }

        usleep(50000);
    }

    // CHANGEMENT: Nettoyer plus bas
    goto_xy(0, 16); // Anciennement 12
    printf("Simulation terminée.\n");

    return 0;
}
