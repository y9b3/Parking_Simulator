#ifndef PARKING_H
#define PARKING_H

#define TOTAL_SPOTS 42 

typedef struct
{
    int screen_x;
    int screen_y;
    int is_occupied;
    int type_vehicule;
} ParkingSpot;

// Variable globale partagée
extern ParkingSpot all_spots[TOTAL_SPOTS];

// Prototypes des fonctions (Juste les noms, pas le code)
void display_static_map(const char *filename);
void goto_xy(int x, int y);
void init_spots_from_map(const char *filename);
void draw_spot(ParkingSpot spot, int is_selected);
void draw_all_spots(int selected_index);

// On ajoute celle-ci pour que le main la connaisse
void init_modeles(); 

#endif
