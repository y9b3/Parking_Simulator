#include <stdio.h>
#include <stdlib.h>
#include "../include/parking.h"

// --- Séquences d'échappement ANSI ---
#define RESET "\033[0m"
#define CLEAR_SCREEN "\033[2J"
#define CURSOR_HOME "\033[H"
#define GREEN_TEXT "\033[92m"
#define RED_TEXT "\033[91m"
#define BLUE_BG "\033[44m"

// --- Définition de la variable globale ---
ParkingSpot all_spots[TOTAL_SPOTS];

// --- Implémentation des Fonctions ---

void display_static_map(const char *filename)
{
    FILE *file;
    char buffer[1024];

    file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("Erreur: Impossible d'ouvrir le fichier du plan.");
        printf("Vérifiez que '%s' existe bien.\n", filename);
        return;
    }

    printf(CLEAR_SCREEN);
    printf(CURSOR_HOME);

    while (fgets(buffer, sizeof(buffer), file) != NULL)
    {
        printf("%s", buffer);
    }

    fclose(file);
    printf(RESET);
    fflush(stdout);
}

void goto_xy(int x, int y)
{
    printf("\033[%d;%dH", y + 1, x + 1);
}

/**
 * CHANGEMENT MAJEUR:
 * Initialise les 32 places avec leurs nouvelles coordonnées.
 */
void init_spots()
{
    // Rangée 1 (y=3)
    all_spots[0] = (ParkingSpot){.screen_x = 2, .screen_y = 3, .is_occupied = 0};
    all_spots[1] = (ParkingSpot){.screen_x = 6, .screen_y = 3, .is_occupied = 1};
    all_spots[2] = (ParkingSpot){.screen_x = 12, .screen_y = 3, .is_occupied = 0};
    all_spots[3] = (ParkingSpot){.screen_x = 16, .screen_y = 3, .is_occupied = 0};
    all_spots[4] = (ParkingSpot){.screen_x = 22, .screen_y = 3, .is_occupied = 1};
    all_spots[5] = (ParkingSpot){.screen_x = 26, .screen_y = 3, .is_occupied = 0};
    all_spots[6] = (ParkingSpot){.screen_x = 32, .screen_y = 3, .is_occupied = 0};
    all_spots[7] = (ParkingSpot){.screen_x = 36, .screen_y = 3, .is_occupied = 0};

    // Rangée 2 (y=5)
    all_spots[8] = (ParkingSpot){.screen_x = 2, .screen_y = 5, .is_occupied = 0};
    all_spots[9] = (ParkingSpot){.screen_x = 6, .screen_y = 5, .is_occupied = 0};
    all_spots[10] = (ParkingSpot){.screen_x = 12, .screen_y = 5, .is_occupied = 0};
    all_spots[11] = (ParkingSpot){.screen_x = 16, .screen_y = 5, .is_occupied = 0};
    all_spots[12] = (ParkingSpot){.screen_x = 22, .screen_y = 5, .is_occupied = 0};
    all_spots[13] = (ParkingSpot){.screen_x = 26, .screen_y = 5, .is_occupied = 0};
    all_spots[14] = (ParkingSpot){.screen_x = 32, .screen_y = 5, .is_occupied = 0};
    all_spots[15] = (ParkingSpot){.screen_x = 36, .screen_y = 5, .is_occupied = 0};

    // Rangée 3 (y=7)
    all_spots[16] = (ParkingSpot){.screen_x = 2, .screen_y = 7, .is_occupied = 0};
    all_spots[17] = (ParkingSpot){.screen_x = 6, .screen_y = 7, .is_occupied = 0};
    all_spots[18] = (ParkingSpot){.screen_x = 12, .screen_y = 7, .is_occupied = 0};
    all_spots[19] = (ParkingSpot){.screen_x = 16, .screen_y = 7, .is_occupied = 0};
    all_spots[20] = (ParkingSpot){.screen_x = 22, .screen_y = 7, .is_occupied = 0};
    all_spots[21] = (ParkingSpot){.screen_x = 26, .screen_y = 7, .is_occupied = 0};
    all_spots[22] = (ParkingSpot){.screen_x = 32, .screen_y = 7, .is_occupied = 0};
    all_spots[23] = (ParkingSpot){.screen_x = 36, .screen_y = 7, .is_occupied = 0};

    // Rangée 4 (y=9)
    all_spots[24] = (ParkingSpot){.screen_x = 2, .screen_y = 9, .is_occupied = 0};
    all_spots[25] = (ParkingSpot){.screen_x = 6, .screen_y = 9, .is_occupied = 0};
    all_spots[26] = (ParkingSpot){.screen_x = 12, .screen_y = 9, .is_occupied = 0};
    all_spots[27] = (ParkingSpot){.screen_x = 16, .screen_y = 9, .is_occupied = 0};
    all_spots[28] = (ParkingSpot){.screen_x = 22, .screen_y = 9, .is_occupied = 0};
    all_spots[29] = (ParkingSpot){.screen_x = 26, .screen_y = 9, .is_occupied = 0};
    all_spots[30] = (ParkingSpot){.screen_x = 32, .screen_y = 9, .is_occupied = 0};
    all_spots[31] = (ParkingSpot){.screen_x = 36, .screen_y = 9, .is_occupied = 0};
}

void draw_spot(ParkingSpot spot, int is_selected)
{
    goto_xy(spot.screen_x, spot.screen_y);

    const char *bg_color = is_selected ? BLUE_BG : "";
    const char *text_color;
    char symbol;

    if (spot.is_occupied)
    {
        text_color = RED_TEXT;
        symbol = 'X';
    }
    else
    {
        text_color = GREEN_TEXT;
        symbol = 'P';
    }
    printf("%s%s%c%s", bg_color, text_color, symbol, RESET);
}

void draw_all_spots(int selected_index)
{
    for (int i = 0; i < TOTAL_SPOTS; i++)
    {
        draw_spot(all_spots[i], i == selected_index);
    }

    // CHANGEMENT: Déplacer le curseur plus bas (car la carte est plus grande)
    goto_xy(0, 14);
    fflush(stdout);
}