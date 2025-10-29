#ifndef PARKING_H
#define PARKING_H

// --- Constantes ---
#define TOTAL_SPOTS 32 // CHANGEMENT: On passe de 12 à 32 places

// --- Structures ---
typedef struct
{
    int screen_x;
    int screen_y;
    int is_occupied;
} ParkingSpot;

// --- Variables Globales ---
extern ParkingSpot all_spots[TOTAL_SPOTS];

// --- Prototypes de Fonctions ---

void display_static_map(const char *filename);

void goto_xy(int x, int y);

void init_spots();

/**
 * Dessine une seule place de parking.
 * @param spot La place à dessiner.
 * @param is_selected 1 si la place doit être "surlignée", 0 sinon.
 */
void draw_spot(ParkingSpot spot, int is_selected);

/**
 * Appelle draw_spot() pour toutes les places.
 * @param selected_index L'index (dans all_spots) de la place à surligner.
 */
void draw_all_spots(int selected_index);

#endif /* PARKING_H */