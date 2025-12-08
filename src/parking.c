#include <stdio.h>
#include <stdlib.h>
#include "../include/parking.h"
#define SPOT_Y_OFFSET 1
#define RESET "\033[0m"
#define CLEAR_SCREEN "\033[2J"
#define CURSOR_HOME "\033[H"
#define GREEN_TEXT "\033[92m"
#define RED_TEXT "\033[91m"
#define BLUE_BG "\033[44m"
#define BG_GREEN "\033[42m" // fond vert
#define BG_RED "\033[41m"   // fond rouge

// caractère qui marque une place dans parking_map.txt
#define SPOT_CHAR '@' // ou 'P' si tu es repassé aux P

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

    int x = 0;
    int y = 0;
    int byte;
    int idx = 0;

    while ((byte = fgetc(file)) != EOF && idx < TOTAL_SPOTS)
    {
        unsigned char c = (unsigned char)byte;

        if (c == '\n')
        {
            y++;
            x = 0;
            continue;
        }

        // si ce n'est pas un octet de continuation UTF-8
        if ((c & 0xC0) != 0x80)
        {
            if (c == SPOT_CHAR)
            {
                all_spots[idx].screen_x = x;

                // on remonte toutes les places d'une ligne
                all_spots[idx].screen_y = (y >= SPOT_Y_OFFSET) ? y - SPOT_Y_OFFSET : 0;

                all_spots[idx].is_occupied = 0;
                idx++;
            }

            // on avance d'une colonne d'affichage
            x++;
        }
    }

    fclose(file);
}

void draw_spot(ParkingSpot spot, int is_selected)
{
    (void)is_selected;

    const char *bg = spot.is_occupied ? BG_RED : BG_GREEN;

    // paramètres du bloc
    int width = 1;  // largeur horizontale
    int height = 2; // HAUTEUR verticale

    int base_x = spot.screen_x;
    int base_y = spot.screen_y; // ⚠️ on ANCRE en haut, on ne centre plus

    for (int dy = 0; dy < height; dy++)
    {
        goto_xy(base_x, base_y + dy); // on descend de dy lignes

        printf("%s", bg);
        for (int dx = 0; dx < width; dx++)
            printf(" ");
        printf("%s", RESET);
    }
}

void draw_all_spots(int selected_index)
{
    for (int i = 0; i < TOTAL_SPOTS; i++)
        draw_spot(all_spots[i], i == selected_index);

    fflush(stdout);
}