#ifndef PARKING_H
#define PARKING_H

#define TOTAL_SPOTS 42
#define HAUTEUR_MAX 60
#define LARGEUR_MAX 400

extern int spawn_x, spawn_y;

typedef struct {
    int screen_x;
    int screen_y;
    int is_occupied;
    int id_voiture;
} ParkingSpot;

typedef enum {
    ETAT_CHERCHE_PLACE,
    ETAT_GARE,
    ETAT_SORTIE
} EtatVehicule;

typedef struct voiture {
    int id;
    int x, y;
    int cible_x, cible_y;
    int etape_trajet; // 0: Entrée vers Allée, 1: Allée vers Place
    int type;
    EtatVehicule etat;
    int temps_gare;
    struct voiture *suivant;
} Vehicule;

typedef struct {
    int id;
    int largeur;
    const char *forme[3];
} ModeleVehicule;

extern ParkingSpot all_spots[TOTAL_SPOTS];
extern Vehicule *liste_vehicules;
extern ModeleVehicule modeles[3];
extern char map_logique[HAUTEUR_MAX][LARGEUR_MAX];

void init_modeles(void);
void display_static_map(const char *filename);
void init_spots_from_map(const char *filename);
void draw_all_spots(int selected_index);
void goto_xy(int x, int y);
void spawner_vehicule(void);
void mettre_a_jour_vehicules(void);
void afficher_vehicules_dynamiques(void);
void liberer_memoire_vehicules(void);

#endif
