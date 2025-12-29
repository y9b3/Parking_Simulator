#ifndef PARKING_H
#define PARKING_H

#define TOTAL_SPOTS 42 // ici tu peux changer si tu change le nombre de P

typedef struct
{
    int screen_x;
    int screen_y;
    int is_occupied;
} ParkingSpot;

extern ParkingSpot all_spots[TOTAL_SPOTS];

void display_static_map(const char *filename);
void goto_xy(int x, int y);

void init_spots_from_map(const char *filename);

void draw_spot(ParkingSpot spot, int is_selected);
void draw_all_spots(int selected_index);

#endif
