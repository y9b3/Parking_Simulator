#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <math.h> // Pour abs()
#include "../include/parking.h"

// Couleurs ANSI
#define RESET "\033[0m"
#define CLEAR_SCREEN "\033[2J"
#define CURSOR_HOME "\033[H"
#define RED_TEXT "\033[91m"
#define BLUE_TEXT "\033[34m"
#define BG_GREEN "\033[42m"
#define SPOT_CHAR '@' 

// Variables Globales
ParkingSpot all_spots[TOTAL_SPOTS];
Vehicule *liste_vehicules = NULL;
ModeleVehicule modeles[3];
int compteur_id_vehicule = 0;

// --- INITIALISATIONS ---

void init_modeles(void) {
    modeles[0].id = 0; modeles[0].largeur = 9;
    modeles[0].forme[0] = "┌═╦═════╗"; modeles[0].forme[1] = "║ ║▆   ║"; modeles[0].forme[2] = "└═╩═════╝";

    modeles[1].id = 1; modeles[1].largeur = 12;
    modeles[1].forme[0] = "╔════════╦═┐"; modeles[1].forme[1] = "║      ▅║ │"; modeles[1].forme[2] = "╚════════╩═┘";

    modeles[2].id = 2; modeles[2].largeur = 10;
    modeles[2].forme[0] = "┌──┬───┬─╗"; modeles[2].forme[1] = "│  ║ ║ ║ │"; modeles[2].forme[2] = "└──┴───┴─╝";
}

void goto_xy(int x, int y) {
    printf("\033[%d;%dH", y + 1, x + 1);
}

void display_static_map(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return;
    printf(CLEAR_SCREEN); printf(CURSOR_HOME);
    int c;
    while ((c = fgetc(file)) != EOF) putchar(c);
    fclose(file);
    printf(RESET); fflush(stdout);
}

void init_spots_from_map(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return;
    int x = 0, y = 0, idx = 0;
    int byte;
    while ((byte = fgetc(file)) != EOF) {
        if (byte == '\r') continue;
        if (byte == '\n') { y++; x = 0; continue; }
        
        if (byte == SPOT_CHAR) {
            if (idx < TOTAL_SPOTS) {
                all_spots[idx].screen_x = x;
                all_spots[idx].screen_y = y;
                all_spots[idx].is_occupied = 0;
                all_spots[idx].id_voiture = -1;
                idx++;
            }
        }
        x++;
    }
    fclose(file);
}

// --- LOGIQUE (MOUVEMENT ET GHOSTING) ---

// Fonction CRITIQUE : Efface la voiture à son ancienne position
void effacer_vehicule(Vehicule *v) {
    int largeur = modeles[v->type].largeur;
    int draw_x = v->x - (largeur/2);
    int draw_y = v->y - 1; 

    if (draw_x < 0) draw_x = 0;
    if (draw_y < 0) draw_y = 0;

    for (int i=0; i<3; i++) {
        goto_xy(draw_x, draw_y + i);
        for (int j=0; j < largeur; j++) printf(" "); // On remplit de vide
    }
}

void spawner_vehicule(void) {
    Vehicule *nouveau = malloc(sizeof(Vehicule));
    if (nouveau == NULL) return;

    nouveau->id = compteur_id_vehicule++;
    // !!! IMPORTANT : METTRE ICI LES COORDONNÉES DE TON ENTREE !!!
    nouveau->x = 90; 
    nouveau->y = 25; 
    
    nouveau->type = rand() % 3;
    nouveau->etat = ETAT_CHERCHE_PLACE;
    nouveau->temps_gare = 0;
    
    // Recherche place libre
    int place_trouvee = -1;
    for (int i = 0; i < TOTAL_SPOTS; i++) {
        if (!all_spots[i].is_occupied) {
            place_trouvee = i;
            break; 
        }
    }

    if (place_trouvee != -1) {
        nouveau->cible_x = all_spots[place_trouvee].screen_x;
        nouveau->cible_y = all_spots[place_trouvee].screen_y;
        all_spots[place_trouvee].is_occupied = 1;
        all_spots[place_trouvee].id_voiture = nouveau->id;
    } else {
        // Parking plein -> Sortie directe
        nouveau->etat = ETAT_SORTIE;
        nouveau->cible_x = 90; 
        nouveau->cible_y = 5;  
    }

    nouveau->suivant = liste_vehicules;
    liste_vehicules = nouveau;
}

void mettre_a_jour_vehicules(void) {
    Vehicule *v = liste_vehicules;
    Vehicule *precedent = NULL;

    while (v != NULL) {
        
        // 1. ON EFFACE AVANT DE BOUGER (Anti-Ghosting)
        effacer_vehicule(v);

        // 2. MOUVEMENT (Si pas garé)
        if (v->etat != ETAT_GARE) {
            if (v->x < v->cible_x) v->x++;
            else if (v->x > v->cible_x) v->x--;
            
            // Priorité X puis Y pour faire des "carrés"
            if (abs(v->x - v->cible_x) < 5) {
                if (v->y < v->cible_y) v->y++;
                else if (v->y > v->cible_y) v->y--;
            }
        }

        // 3. ARRIVÉE SUR PLACE (Snap)
        if (v->etat == ETAT_CHERCHE_PLACE && 
            abs(v->x - v->cible_x) <= 2 && abs(v->y - v->cible_y) <= 2) {
            
            v->x = v->cible_x; // On force la position exacte
            v->y = v->cible_y;
            v->etat = ETAT_GARE;
            v->temps_gare = 50 + (rand() % 100);
        }

        // 4. TEMPS GARÉ
        else if (v->etat == ETAT_GARE) {
            v->temps_gare--;
            if (v->temps_gare <= 0) {
                v->etat = ETAT_SORTIE;
                v->cible_x = 90; // Sortie X
                v->cible_y = 5;  // Sortie Y
                
                // Libération place
                for (int i=0; i<TOTAL_SPOTS; i++) {
                    if (all_spots[i].id_voiture == v->id) {
                        all_spots[i].is_occupied = 0;
                        all_spots[i].id_voiture = -1;
                    }
                }
            }
        }

        // 5. SORTIE (Suppression)
        if (v->etat == ETAT_SORTIE && 
            abs(v->x - v->cible_x) <= 3 && abs(v->y - v->cible_y) <= 3) {
            
            Vehicule *a_supprimer = v;
            if (precedent == NULL) {
                liste_vehicules = v->suivant;
                v = liste_vehicules;
            } else {
                precedent->suivant = v->suivant;
                v = v->suivant;
            }
            free(a_supprimer);
            continue;
        }

        precedent = v;
        v = v->suivant;
    }
}

void afficher_vehicules_dynamiques(void) {
    Vehicule *v = liste_vehicules;
    while (v != NULL) {
        int largeur = modeles[v->type].largeur;
        int draw_x = v->x - (largeur/2);
        int draw_y = v->y - 1; 

        if (draw_x < 0) draw_x = 0;
        if (draw_y < 0) draw_y = 0;

        for (int i=0; i<3; i++) {
            goto_xy(draw_x, draw_y + i);
            if (v->etat == ETAT_GARE) printf(BLUE_TEXT);
            else printf(RED_TEXT);
            
            printf("%s", modeles[v->type].forme[i]);
            printf(RESET);
        }
        v = v->suivant;
    }
}

void draw_spot(ParkingSpot spot, int is_selected) {
    (void)is_selected;
    // On dessine le spot seulement s'il est LIBRE.
    // Si occupé, la voiture sera dessinée par dessus, donc on laisse vide ou on met une couleur de fond.
    if (!spot.is_occupied) {
        int base_y = spot.screen_y - 1;
        if (base_y < 0) base_y = 0;
        for (int dy = 0; dy < 3; dy++) {
            goto_xy(spot.screen_x, base_y + dy);
            printf("%s ", BG_GREEN); // Carré vert
            printf("%s", RESET);
        }
    }
}

void draw_all_spots(int selected_index) {
    for (int i = 0; i < TOTAL_SPOTS; i++)
        draw_spot(all_spots[i], i == selected_index);
}

void liberer_memoire_vehicules() {
    Vehicule *v = liste_vehicules;
    while (v != NULL) {
        Vehicule *temp = v;
        v = v->suivant;
        free(temp);
    }
}