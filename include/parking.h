#ifndef PARKING_H
#define PARKING_H

#define TOTAL_SPOTS 42 

typedef struct {
    int screen_x;
    int screen_y;
    int is_occupied;
    int type_vehicule; // <--- AJOUTE CETTE LIGNE OBLIGATOIREMENT
} ParkingSpot;

extern ParkingSpot all_spots[TOTAL_SPOTS];

// Ajoute aussi le prototype de l'initialisation pour que le main le voie
void init_modeles(void); 

void display_static_map(const char *filename);
void goto_xy(int x, int y);
void init_spots_from_map(const char *filename);
void draw_spot(ParkingSpot spot, int is_selected);
void draw_all_spots(int selected_index);

#endif
