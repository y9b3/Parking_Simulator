#ifndef PARKING_H
#define PARKING_H

#define TOTAL_SPOTS 20  // Ajuste selon le nombre de @ dans ta map
#define HAUTEUR_MAX 40
#define LARGEUR_MAX 120

// --- STRUCTURES ---

// Structure pour une place de parking (fixe)
typedef struct {
    int screen_x;
    int screen_y;
    int is_occupied;     // 0 = libre, 1 = occupé
    int id_voiture;      // ID de la voiture garée (-1 si vide)
} ParkingSpot;

// États possibles d'une voiture (Machine à états)
typedef enum {
    ETAT_CHERCHE_PLACE,
    ETAT_GARE,
    ETAT_SORTIE
} EtatVehicule;

// Structure Véhicule (LISTE CHAÎNÉE OBLIGATOIRE)
typedef struct voiture {
    int id;
    int x, y;            // Position actuelle
    int cible_x, cible_y;// Destination (Place ou Sortie)
    int type;            // 0, 1, 2 (Design)
    
    EtatVehicule etat;
    int temps_gare;      // Compteur de temps resté garé
    
    struct voiture *suivant; // Pointeur vers la suivante
} Vehicule;

// Structure pour les dessins ASCII
typedef struct {
    int id;
    int largeur;
    const char *forme[3];
} ModeleVehicule;

// --- VARIABLES GLOBALES ---
extern ParkingSpot all_spots[TOTAL_SPOTS];
extern Vehicule *liste_vehicules;
extern ModeleVehicule modeles[3];

// --- PROTOTYPES ---
void init_modeles(void);
void display_static_map(const char *filename);
void init_spots_from_map(const char *filename);
void draw_all_spots(int selected_index);
void goto_xy(int x, int y);

// Fonctions Logiques (Moteur du jeu)
void spawner_vehicule(void);
void mettre_a_jour_vehicules(void);
void afficher_vehicules_dynamiques(void);
void liberer_memoire_vehicules(void);

#endif