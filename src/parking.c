#define _XOPEN_SOURCE 700
#define SPOT_VISUAL_Y_OFFSET (-5)

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <wchar.h>
#include "parking.h"

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

// =====================
//      VÉHICULES
// =====================

typedef struct {
    int id;
    int largeur;
    const char *forme[3]; // hauteur 3 lignes (comme draw_spot)
} ModeleVehicule;

ModeleVehicule modeles[3];

void init_modeles(void)
{
    // 1. Voiture standard
    modeles[0].id = 0;
    modeles[0].largeur = 9;
    modeles[0].forme[0] = "┌═╦═════╗";
    modeles[0].forme[1] = "║ ║▆    ║";
    modeles[0].forme[2] = "└═╩═════╝";

    // 2. Camionnette
    modeles[1].id = 1;
    modeles[1].largeur = 12;
    modeles[1].forme[0] = "╔════════╦═┐";
    modeles[1].forme[1] = "║       ▅║ │";
    modeles[1].forme[2] = "╚════════╩═┘";

    // 3. Compacte
    modeles[2].id = 2;
    modeles[2].largeur = 10;
    modeles[2].forme[0] = "┌──┬───┬─╗";
    modeles[2].forme[1] = "│  ║ ║ ║ │";
    modeles[2].forme[2] = "└──┴───┴─╝";
}

// =====================
//      PARKING
// =====================

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
    // +1 car ANSI commence à 1,1
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
    int byte;

    while ((byte = fgetc(file)) != EOF)
    {
        unsigned char c = (unsigned char)byte;

        if (c == '\n')
        {
            y++;
            x = 0;
            continue;
        }

        // UTF-8 : on ignore les octets de continuation
        if ((c & 0xC0) != 0x80)
        {
            if (c == (unsigned char)SPOT_CHAR)
            {
                if (idx >= TOTAL_SPOTS)
                    break;

                all_spots[idx].screen_x = x;
                all_spots[idx].screen_y = y + SPOT_VISUAL_Y_OFFSET;
                if (all_spots[idx].screen_y < 0)
                    all_spots[idx].screen_y = 0;

                all_spots[idx].is_occupied = 0;
                idx++;
            }
            x++;
        }
    }

    fclose(file);

    if (idx != TOTAL_SPOTS)
    {
        fprintf(stderr,
                "Erreur: spots trouvés=%d, attendu=%d\n",
                idx, TOTAL_SPOTS);
        exit(EXIT_FAILURE);
    }
}

void draw_spot(ParkingSpot spot, int is_selected)
{
    (void)is_selected;

    const char *bg = spot.is_occupied ? BG_RED : BG_GREEN;

    int width = 1;
    int height = 3;

    int base_x = spot.screen_x;
    int base_y = spot.screen_y - (height / 2);
    if (base_y < 0)
        base_y = 0;

    for (int dy = 0; dy < height; dy++)
    {
        goto_xy(base_x, base_y + dy);
        printf("%s ", bg);
        printf("%s", RESET);
    }
}

void draw_all_spots(int selected_index)
{
    for (int i = 0; i < TOTAL_SPOTS; i++)
        draw_spot(all_spots[i], i == selected_index);

    fflush(stdout);
}
