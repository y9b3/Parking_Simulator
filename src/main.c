#define _XOPEN_SOURCE 700
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
    int c; 
    char result = 0;

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
    // --- 1. IMPORTANT : On charge les dessins des véhicules ici ---
    init_modeles(); 

    const char *map_path = "assets/parking_map.txt";

    display_static_map(map_path);
    init_spots_from_map(map_path);

    int selected_spot_index = 0;
    draw_all_spots(selected_spot_index);

    goto_xy(0, 14); 
    printf("ZQSD: Deplacer | ESPACE: Changer vehicule | E: Quitter\n");

    char key = 0;
    while (key != 'e')
    {
        key = key_pressed();

        if (key != 0)
        {
            int old_selection = selected_spot_index;
            int selection_changed = 0;

            switch (key)
            {
            case 'q': // gauche
                if (selected_spot_index > 0)
                {
                    selected_spot_index--;
                    selection_changed = 1;
                }
                break;

            case 'd': // droite
                if (selected_spot_index < TOTAL_SPOTS - 1)
                {
                    selected_spot_index++;
                    selection_changed = 1;
                }
                break;

            case 'z': // haut
                if (selected_spot_index > 0)
                {
                    selected_spot_index--;
                    selection_changed = 1;
                }
                break;

            case 's': // bas
                if (selected_spot_index < TOTAL_SPOTS - 1)
                {
                    selected_spot_index++;
                    selection_changed = 1;
                }
                break;

            // --- 2. LOGIQUE ESPACE MODIFIÉE ---
            case ' ':
                // Si la place est vide, on met le premier véhicule (Type 0)
                if (!all_spots[selected_spot_index].is_occupied) {
                    all_spots[selected_spot_index].is_occupied = 1;
                    all_spots[selected_spot_index].type_vehicule = 0;
                }
                // Si elle est occupée, on passe au véhicule suivant
                else {
                    all_spots[selected_spot_index].type_vehicule++;
                    
                    // Si on a dépassé le dernier type (2), on vide la place
                    if (all_spots[selected_spot_index].type_vehicule > 2) {
                        all_spots[selected_spot_index].is_occupied = 0;
                        all_spots[selected_spot_index].type_vehicule = 0; // Reset
                    }
                }
                // On redessine immédiatement la place avec le nouveau véhicule
                draw_spot(all_spots[selected_spot_index], 1);
                break;
            }

            if (selection_changed)
            {
                draw_spot(all_spots[old_selection], 0);
                draw_spot(all_spots[selected_spot_index], 1);
            }

            goto_xy(0, 15); 
            // Affichage de debug utile
            printf("Place: %d | Type: %d   ", selected_spot_index, all_spots[selected_spot_index].type_vehicule);
            fflush(stdout);
        }

        usleep(50000);
    }

    goto_xy(0, 16); 
    printf("Simulation terminée.\n");

    return 0;
}
