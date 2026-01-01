#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include "parking.h"

#define RESET "\033[0m"
#define CLEAR_SCREEN "\033[2J"
#define CURSOR_HOME "\033[H"

// Couleurs (texte)
#define GREEN_TEXT "\033[32m"
#define RED_TEXT "\033[31m"

// Caractère qui marque une place dans la map ASCII
#define SPOT_CHAR 'D'

ParkingSpot all_spots[TOTAL_SPOTS];

void display_static_map(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("Erreur: Impossible d'ouvrir le fichier du plan");
        exit(EXIT_FAILURE);
    }

    printf(CLEAR_SCREEN);
    printf(CURSOR_HOME);

    int c;
    while ((c = fgetc(file)) != EOF)
        putchar(c);

    fclose(file);
    printf(RESET);
    fflush(stdout);
}

void goto_xy(int x, int y)
{
    // +1 car les coordonnées ANSI commencent à 1,1
    printf("\033[%d;%dH", y + 1, x + 1);
}

void init_spots_from_map(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("Erreur ouverture parking_map");
        exit(EXIT_FAILURE);
    }

    int x = 0, y = 0, idx = 0;
    int c;

    while ((c = fgetc(file)) != EOF && idx < TOTAL_SPOTS)
    {
        if (c == '\n')
        {
            y++;
            x = 0;
            continue;
        }

        if (c == SPOT_CHAR) // 'P'
        {
            all_spots[idx].screen_x = x;
            all_spots[idx].screen_y = y - 7;
            all_spots[idx].is_occupied = 0;
            idx++;
        }

        x++; // ASCII => 1 char = 1 colonne
    }

    fclose(file);

    if (idx != TOTAL_SPOTS)
    {
        fprintf(stderr, "Erreur: spots trouvés=%d, attendu=%d\n", idx, TOTAL_SPOTS);
        exit(EXIT_FAILURE);
    }
}

void draw_spot(ParkingSpot spot, int is_selected)
{
    goto_xy(spot.screen_x, spot.screen_y);

    const char *color = spot.is_occupied ? "\033[31m" : "\033[32m"; // rouge/vert
    const char *sel = is_selected ? "\033[7m" : "";                 // sélection

    // On remplace le P par un symbole plein
    printf("%s%s█%s", sel, color, RESET);
}

void draw_all_spots(int selected_index)
{
    for (int i = 0; i < TOTAL_SPOTS; i++)
        draw_spot(all_spots[i], i == selected_index);

    fflush(stdout);
}