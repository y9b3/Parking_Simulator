#ifndef PARKING_H
#define PARKING_H

// --- CONFIGURATION ---
#define TOTAL_SPOTS 20   // Nombre max de places
#define HAUTEUR_MAX 50   // Hauteur max de ta grille logique
#define LARGEUR_MAX 400  // Largeur max de ta grille logique

// --- STRUCTURES ---

// 1. Une place de parking
typedef struct {
    int screen_x;
    int screen_y;
    int is_occupied; 
    int id_voiture;      // ID de la voiture garée (-1 si vide)
    int type_vehicule;   // Design du véhicule
} ParkingSpot;

// 2. États de la voiture (Machine à états)
typedef enum {
    ETAT_CHERCHE_PLACE,
    ETAT_GARE,
    ETAT_SORTIE
} EtatVehicule;

// 3. La voiture (Structure pour liste chaînée)
typedef struct voiture {
    int id;
    int x, y;             // Position actuelle
    int cible_x, cible_y; // Destination (la place ou la sortie)
    int type;             // 0, 1 ou 2
    
    EtatVehicule etat;
    int temps_gare;       // Durée avant de repartir
    
    struct voiture *suivant; // Pointeur pour la liste chaînée
} Vehicule;

// 4. Modèle graphique (ASCII Art)
typedef struct {
    int id;
    int largeur;
    const char *forme[3];
} ModeleVehicule;

// --- VARIABLES GLOBALES (Accessibles partout) ---
extern ParkingSpot all_spots[TOTAL_SPOTS];
extern Vehicule *liste_vehicules;
extern ModeleVehicule modeles[3];
extern char map_logique[HAUTEUR_MAX][LARGEUR_MAX];

// --- PROTOTYPES DES FONCTIONS ---

// Initialisations
void init_modeles(void);
void display_static_map(const char *filename);
void init_spots_from_map(const char *filename);
void goto_xy(int x, int y);

// Moteur de jeu et Affichage
void spawner_vehicule(void);
void mettre_a_jour_vehicules(void);
void afficher_vehicules_dynamiques(void);
void draw_all_spots(int selected_index);
void draw_spot(ParkingSpot spot, int is_selected);
void liberer_memoire_vehicules(void);

#endif
