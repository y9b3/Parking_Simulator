#ifndef PARKING_H
#define PARKING_H

#define TOTAL_SPOTS 42  // Nombre max de places (ajuste selon ta map)
#define HAUTEUR_MAX 50  // Hauteur max de la carte
#define LARGEUR_MAX 400 // Largeur max de la carte
extern int spawn_x, spawn_y;
// --- STRUCTURES ---

// Une place de parking (Zone verte)
typedef struct
{
    int screen_x;
    int screen_y;
    int is_occupied; // 0 = libre, 1 = occupé
    int id_voiture;  // ID de la voiture garée dessus (-1 si vide)
} ParkingSpot;

// États de la voiture (Machine à états)
typedef enum
{
    ETAT_CHERCHE_PLACE,
    ETAT_GARE,
    ETAT_SORTIE
} EtatVehicule;

// La voiture (Liste Chaînée - OBLIGATOIRE)
typedef struct voiture
{
    int id;
    int x, y;             // Position actuelle
    int cible_x, cible_y; // Destination
    int type;             // 0, 1, 2 (Design)

    EtatVehicule etat;
    int temps_gare; // Compteur (combien de temps elle reste)

    struct voiture *suivant; // Pointeur vers la suivante
} Vehicule;

// Modèle graphique (Dessin ASCII)
typedef struct
{
    int id;
    int largeur;
    const char *forme[3];
} ModeleVehicule;

// --- VARIABLES GLOBALES ---
extern ParkingSpot all_spots[TOTAL_SPOTS];
extern Vehicule *liste_vehicules;
extern ModeleVehicule modeles[3];
// Grille logique pour les collisions (Murs)
extern char map_logique[HAUTEUR_MAX][LARGEUR_MAX];

// --- FONCTIONS ---
void init_modeles(void);
void display_static_map(const char *filename);
void init_spots_from_map(const char *filename); // Charge aussi les collisions
void draw_all_spots(int selected_index);
void goto_xy(int x, int y);

// Moteur du jeu
void spawner_vehicule(void);
void mettre_a_jour_vehicules(void); // Gère Mouvement + Collisions + Ghosting
void afficher_vehicules_dynamiques(void);
void liberer_memoire_vehicules(void);

#endif