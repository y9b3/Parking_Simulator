#ifndef PARKING_H
#define PARKING_H

#define TOTAL_SPOTS 20  // Nombre de places (Ajuste si nécessaire)
#define HAUTEUR_MAX 40
#define LARGEUR_MAX 120

// --- STRUCTURES ---

// Une place de parking
typedef struct {
    int screen_x;
    int screen_y;
    int is_occupied;     // 0 = libre, 1 = occupé
    int id_voiture;      // ID de la voiture qui la réserve (-1 si vide)
} ParkingSpot;

// États de la voiture
typedef enum {
    ETAT_CHERCHE_PLACE,
    ETAT_GARE,
    ETAT_SORTIE
} EtatVehicule;

// La voiture (Liste Chaînée)
typedef struct voiture {
    int id;
    int x, y;            // Position actuelle
    int cible_x, cible_y;// Destination
    int type;            // 0, 1, 2 (Design)
    
    EtatVehicule etat;
    int temps_gare;      // Compteur de temps stationné
    
    struct voiture *suivant; // Pointeur vers la suivante
} Vehicule;

// Modèle graphique ASCII
typedef struct {
    int id;
    int largeur;
    const char *forme[3];
} ModeleVehicule;

// --- GLOBALES ---
extern ParkingSpot all_spots[TOTAL_SPOTS];
extern Vehicule *liste_vehicules;
extern ModeleVehicule modeles[3];

// --- PROTOTYPES ---
void init_modeles(void);
void display_static_map(const char *filename);
void init_spots_from_map(const char *filename);
void draw_all_spots(int selected_index);
void goto_xy(int x, int y);

// Fonctions Logiques
void spawner_vehicule(void);
void mettre_a_jour_vehicules(void); // C'est elle qui gère le mouvement et l'effacement
void afficher_vehicules_dynamiques(void);
void liberer_memoire_vehicules(void);

#endif