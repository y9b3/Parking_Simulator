#ifndef VEHICULE_H
#define VEHICULE_H

// Déclaration de la structure VEHICULE
typedef struct voiture VEHICULE;

struct voiture {
    char direction;               // Direction du véhicule (N, S, E, O)
    int posx;                     // Position courante sur l'axe X
    int posy;                     // Position courante sur l'axe Y
    int vitesse;                  // Vitesse du véhicule
    char alignement;              // 'g' pour gauche ou 'd' pour droite
    char type;                    // Type de véhicule ('v' pour voiture, 'c' pour camion, etc.)
    char carrosserie[4][30];      // Représentation ASCII du véhicule
    int code_couleur;             // Code couleur pour l'affichage
    char etat;                    // '1' pour actif, '0' pour inactif
    unsigned long int tps;        // Temps d’entrée dans le parking
    VEHICULE *next;               // Pointeur vers le prochain véhicule (liste chaînée)
};

// === Fonctions de gestion des véhicules ===

// Création
VEHICULE* creer_vehicule(char direction, int posx, int posy, int vitesse,
                         char alignement, char type, char *carrosserie,
                         int code_couleur);

// Liste chaînée
void ajouter_vehicule(VEHICULE *nouveau_vehicule);
void supprimer_vehicule(VEHICULE *vehicule_a_supprimer);
void afficher_vehicules();

// Recherche
VEHICULE* chercher_vehicule(char *nom);

// Déplacement
void deplacer_vehicule(VEHICULE *v);

// Calcul du tarif
float calculer_tarif(VEHICULE *v);

void deplacer_tous_vehicules();

#endif // VEHICULE_H
