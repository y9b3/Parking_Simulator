#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "../include/parking.h"

// Définitions des couleurs
#define RESET "\033[0m"
#define CLEAR_SCREEN "\033[2J"
#define CURSOR_HOME "\033[H"
#define RED_TEXT "\033[91m"    // Voiture en mouvement
#define BLUE_TEXT "\033[34m"   // Voiture garée
#define BG_GREEN "\033[42m"    // Place libre
#define SPOT_CHAR '@' 

// Variables globales
ParkingSpot all_spots[TOTAL_SPOTS];
Vehicule *liste_vehicules = NULL;
ModeleVehicule modeles[3];
char map_logique[HAUTEUR_MAX][LARGEUR_MAX];
int compteur_id_vehicule = 0;

// --- INITIALISATION ---

void init_modeles(void) {
    // Type 0 : Voiture standard
    modeles[0].id = 0; modeles[0].largeur = 9;
    modeles[0].forme[0] = "┌═╦═════╗";
    modeles[0].forme[1] = "║ ║▆    ║";
    modeles[0].forme[2] = "└═╩═════╝";

    // Type 1 : Camionnette
    modeles[1].id = 1; modeles[1].largeur = 12;
    modeles[1].forme[0] = "╔════════╦═┐";
    modeles[1].forme[1] = "║       ▅║ │";
    modeles[1].forme[2] = "╚════════╩═┘";

    // Type 2 : Compacte
    modeles[2].id = 2; modeles[2].largeur = 10;
    modeles[2].forme[0] = "┌──┬───┬─╗";
    modeles[2].forme[1] = "│  ║ ║ ║ │";
    modeles[2].forme[2] = "└──┴───┴─╝";
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
    for(int y=0; y<HAUTEUR_MAX; y++) 
        for(int x=0; x<LARGEUR_MAX; x++) map_logique[y][x] = ' ';

    FILE *file = fopen(filename, "r");
    if (!file) return;

    int x = 0, y = 0, idx = 0;
    int byte;
    while ((byte = fgetc(file)) != EOF) {
        if (byte == '\r') continue;
        if (byte == '\n') { y++; x = 0; continue; }
        
        if (x < LARGEUR_MAX && y < HAUTEUR_MAX) {
            map_logique[y][x] = (char)byte;
        }

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

// --- MOTEUR PHYSIQUE ---

int est_obstacle(int x, int y) {
    if (x < 0 || x >= LARGEUR_MAX || y < 0 || y >= HAUTEUR_MAX) return 1;
    unsigned char c = (unsigned char)map_logique[y][x];
    if (c == ' ' || c == '>' || c == '<' || c == '^' || c == 'v' || c == SPOT_CHAR || c == '.') {
        return 0; 
    }
    return 1; 
}

int est_bloque_par_voiture(int x, int y, int mon_id) {
    Vehicule *v = liste_vehicules;
    while(v != NULL) {
        if (v->id != mon_id) {
            if (abs(v->x - x) < 11 && abs(v->y - y) < 2) return 1;
        }
        v = v->suivant;
    }
    return 0;
}

void effacer_vehicule(Vehicule *v) {
    int largeur = modeles[v->type].largeur;
    int draw_x = v->x - (largeur/2);
    int draw_y = v->y - 1; 
    if (draw_x < 0) draw_x = 0;

    for (int i=0; i<3; i++) {
        goto_xy(draw_x, draw_y + i);
        for (int j=0; j < largeur; j++) printf(" "); 
    }
}

// --- GESTION DYNAMIQUE ---

void spawner_vehicule(void) {
    int start_x = 180; 
    int start_y = 19; 

    if (est_bloque_par_voiture(start_x, start_y, -1)) return;

    Vehicule *nouveau = malloc(sizeof(Vehicule));
    if (!nouveau) return;

    nouveau->id = compteur_id_vehicule++;
    nouveau->x = start_x; nouveau->y = start_y;
    nouveau->type = rand() % 3;
    nouveau->etat = ETAT_CHERCHE_PLACE;
    
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
        nouveau->etat = ETAT_SORTIE;
        nouveau->cible_x = 180; nouveau->cible_y = 5;
    }

    nouveau->suivant = liste_vehicules;
    liste_vehicules = nouveau;
}

void mettre_a_jour_vehicules(void) {
    Vehicule *v = liste_vehicules;
    Vehicule *precedent = NULL;

    while (v != NULL) {
        effacer_vehicule(v);

        if (v->etat != ETAT_GARE) {
            int next_x = v->x;
            int next_y = v->y;
            int a_bouge = 0;

            if (v->x < v->cible_x) next_x++;
            else if (v->x > v->cible_x) next_x--;

            if (!est_obstacle(next_x, v->y) && !est_bloque_par_voiture(next_x, v->y, v->id)) {
                v->x = next_x; a_bouge = 1;
            } 
            
            if (!a_bouge || abs(v->x - v->cible_x) < 2) {
                next_y = v->y;
                if (v->y < v->cible_y) next_y++;
                else if (v->y > v->cible_y) next_y--;

                if (!est_obstacle(v->x, next_y) && !est_bloque_par_voiture(v->x, next_y, v->id)) {
                    v->y = next_y;
                }
            }
        }

        if (v->etat == ETAT_CHERCHE_PLACE && abs(v->x - v->cible_x) <= 1 && abs(v->y - v->cible_y) <= 1) {
            v->x = v->cible_x; v->y = v->cible_y;
            v->etat = ETAT_GARE;
            v->temps_gare = 100 + (rand() % 200);
        }
        else if (v->etat == ETAT_GARE) {
            v->temps_gare--;
            if (v->temps_gare <= 0) {
                v->etat = ETAT_SORTIE;
                v->cible_x = 180; v->cible_y = 2; 
                for (int i=0; i<TOTAL_SPOTS; i++) {
                    if (all_spots[i].id_voiture == v->id) {
                        all_spots[i].is_occupied = 0;
                        all_spots[i].id_voiture = -1;
                    }
                }
            }
        }

        if (v->etat == ETAT_SORTIE && abs(v->y - v->cible_y) <= 1) {
            Vehicule *tmp = v;
            if (precedent == NULL) { liste_vehicules = v->suivant; v = liste_vehicules; }
            else { precedent->suivant = v->suivant; v = v->suivant; }
            free(tmp);
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

        for (int i=0; i<3; i++) {
            goto_xy(draw_x, draw_y + i);
            if (v->etat == ETAT_GARE) printf(BLUE_TEXT); else printf(RED_TEXT);
            printf("%s", modeles[v->type].forme[i]);
            printf(RESET);
        }
        v = v->suivant;
    }
}

// --- AFFICHAGE DES PLACES (CORRECTED) ---

void draw_spot(ParkingSpot spot, int is_selected) {
    (void)is_selected; 
    // On n'affiche le carré vert QUE si la place est libre
    if (spot.is_occupied) return;

    int base_y = spot.screen_y - 1;
    if (base_y < 0) base_y = 0;
    
    for (int dy = 0; dy < 3; dy++) {
        goto_xy(spot.screen_x, base_y + dy);
        printf("%s %s", BG_GREEN, RESET);
    }
}

void draw_all_spots(int selected_index) {
    for (int i = 0; i < TOTAL_SPOTS; i++) {
        draw_spot(all_spots[i], (i == selected_index));
    }
    fflush(stdout);
}

void liberer_memoire_vehicules() {
    Vehicule *v = liste_vehicules;
    while (v != NULL) {
        Vehicule *temp = v;
        v = v->suivant;
        free(temp);
    }
}
