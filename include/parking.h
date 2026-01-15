#ifndef PARKING_H
#define PARKING_H

#include <time.h>

#define TOTAL_SPOTS 42
#define HAUTEUR_MAX 100
#define LARGEUR_MAX 400

/* Definition des emplacements de parking */
typedef struct
{
    int screen_x;
    int screen_y;
    int is_occupied;
    int id_voiture;
} ParkingSpot;

/* États possibles d'un vehicule */
typedef enum
{
    ETAT_CHERCHE_PLACE,
    ETAT_GARE,
    ETAT_SORTIE
} EtatVehicule;

/* Structure principale d'un vehicule (Liste chainee) */
typedef struct voiture
{
    int id;
    int x, y;
    int type;
    EtatVehicule etat;
    unsigned long int tps;
    char direction; /* N, S, E, O */
    int clignotement;

    int id_place_visee;
    long heure_arrivee; /* Stockage du temps pour la facturation */
    int a_paye;

    struct voiture *suivant; /* Pointeur liste chainee */
} Vehicule;

/* Modeles graphiques des voitures */
typedef struct
{
    int id;
    int largeur;
    char *forme[3];
    char *forme_v[5];
} ModeleVehicule;

/* Variables globales partagees */
extern ParkingSpot all_spots[TOTAL_SPOTS];
extern Vehicule *liste_vehicules;
extern Vehicule *voiture_joueur;
extern ModeleVehicule modeles[3];
extern char map_logique[HAUTEUR_MAX][LARGEUR_MAX];
extern int spawn_x, spawn_y, sortie_x, sortie_y;

/* Prototypes des fonctions */
void init_modeles(void);
void display_static_map(const char *f);
void init_spots_from_map(const char *f);
void draw_all_spots(int idx);
void goto_xy(int x, int y);
void spawner_vehicule(void);
void mettre_a_jour_vehicules(void);
void afficher_vehicule(Vehicule *v);
void effacer_vehicule(Vehicule *v);
void liberer_memoire_vehicules(void);
int est_obstacle(int x, int y);
void deplacer_joueur(int dx, int dy, char dir);
void sauvegarder_background(void);
int verifier_place_proche(Vehicule *v);
void jouer_mode_solo(void);
void jouer_mode_multi(void);

#endif