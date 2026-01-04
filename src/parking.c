#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <math.h> // Pour abs()
#include "../include/parking.h"

// Couleurs ANSI
#define RESET "\033[0m"
#define CLEAR_SCREEN "\033[2J"
#define CURSOR_HOME "\033[H"
#define RED_TEXT "\033[91m"     // Voiture qui roule
#define BLUE_TEXT "\033[34m"    // Voiture garée
#define BG_GREEN "\033[42m"     // Place libre
#define SPOT_CHAR '@'           // Symbole dans le .txt

// --- VARIABLES GLOBALES ---
ParkingSpot all_spots[TOTAL_SPOTS];
Vehicule *liste_vehicules = NULL;
ModeleVehicule modeles[3];
char map_logique[HAUTEUR_MAX][LARGEUR_MAX]; // Grille de collision
int compteur_id_vehicule = 0;

// --- INITIALISATION ---

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

// Charge les places ET remplit la grille de collision (Murs)
void init_spots_from_map(const char *filename) {
    // 1. Vider la map logique
    for(int y=0; y<HAUTEUR_MAX; y++) 
        for(int x=0; x<LARGEUR_MAX; x++) map_logique[y][x] = ' ';

    FILE *file = fopen(filename, "r");
    if (!file) return;

    int x = 0, y = 0, idx = 0;
    int byte;
    while ((byte = fgetc(file)) != EOF) {
        if (byte == '\r') continue; // Ignorer Windows CR
        if (byte == '\n') { y++; x = 0; continue; }
        
        // 2. Remplir la grille logique
        if (x < LARGEUR_MAX && y < HAUTEUR_MAX) {
            map_logique[y][x] = (char)byte;
        }

        // 3. Détecter les places (@)
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

// Vérifie si une case est un obstacle (Mur)
int est_obstacle(int x, int y) {
    // 1. Toujours vérifier les limites
    if (x < 0 || x >= LARGEUR_MAX || y < 0 || y >= HAUTEUR_MAX) return 1;
    
    char c = map_logique[y][x];
    
    // 2. LOGIQUE INVERSÉE : On liste UNIQUEMENT les murs connus.
    // Si le caractère est un mur vertical, horizontal, un coin ou un underscore...
    if (c == '|' || c == '-' || c == '_' || c == '+') {
        return 1; // C'est un obstacle, on ne passe pas
    }
    
    // Pour TOUT le reste (espace, flèches, points, lettres...), on considère que c'est de la route.
    return 0; 
}

// Vérifie si une autre voiture bloque le passage
int est_bloque_par_voiture(int x, int y, int mon_id) {
    Vehicule *v = liste_vehicules;
    while(v != NULL) {
        if (v->id != mon_id) {
            // hitbox simple
            if (abs(v->x - x) < 10 && abs(v->y - y) < 2) return 1;
        }
        v = v->suivant;
    }
    return 0;
}

// Efface la voiture avant déplacement (Anti-Ghosting)
void effacer_vehicule(Vehicule *v) {
    int largeur = modeles[v->type].largeur;
    int draw_x = v->x - (largeur/2);
    int draw_y = v->y - 1; 
    if (draw_x < 0) draw_x = 0; if (draw_y < 0) draw_y = 0;

    for (int i=0; i<3; i++) {
        goto_xy(draw_x, draw_y + i);
        for (int j=0; j < largeur; j++) printf(" "); 
    }
}

void spawner_vehicule(void) {
    // --- POSITION CALIBREE POUR TA MAP ---
    // Ton curseur était à 188 (sur le mur). On décale à gauche pour centrer.
    int start_x = 180; 
    int start_y = 19;  // Bas de la map

    // Si bouchon à l'entrée, on attend
    if (est_bloque_par_voiture(start_x, start_y, -1)) return;

    Vehicule *nouveau = malloc(sizeof(Vehicule));
    if (!nouveau) return;

    nouveau->id = compteur_id_vehicule++;
    nouveau->x = start_x; 
    nouveau->y = start_y; 
    
    nouveau->type = rand() % 3;
    nouveau->etat = ETAT_CHERCHE_PLACE;
    
    // Trouver une place
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
        // Parking plein -> Sortie
        nouveau->etat = ETAT_SORTIE;
        nouveau->cible_x = 180; // Retour vers l'entrée/sortie
        nouveau->cible_y = 5;  
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

            // X
            if (v->x < v->cible_x) next_x++;
            else if (v->x > v->cible_x) next_x--;

            if (!est_obstacle(next_x, v->y) && !est_bloque_par_voiture(next_x, v->y, v->id)) {
                v->x = next_x; a_bouge = 1;
            } 
            else {
                // Contournement Y
                if (v->y < v->cible_y) next_y++; else next_y--;
                if (!est_obstacle(v->x, next_y) && !est_bloque_par_voiture(v->x, next_y, v->id)) {
                    v->y = next_y; a_bouge = 1;
                }
            }

            // Y
            if (!a_bouge || abs(v->x - v->cible_x) < 5) {
                next_y = v->y;
                if (v->y < v->cible_y) next_y++; else next_y--;
                if (next_y != v->y && !est_obstacle(v->x, next_y) && !est_bloque_par_voiture(v->x, next_y, v->id)) {
                    v->y = next_y;
                }
            }
        }

        // Snap
        if (v->etat == ETAT_CHERCHE_PLACE && 
            abs(v->x - v->cible_x) <= 2 && abs(v->y - v->cible_y) <= 2) {
            v->x = v->cible_x; v->y = v->cible_y;
            v->etat = ETAT_GARE;
            v->temps_gare = 50 + (rand() % 100);
        }

        // Départ
        else if (v->etat == ETAT_GARE) {
            v->temps_gare--;
            if (v->temps_gare <= 0) {
                v->etat = ETAT_SORTIE;
                v->cible_x = 180; // Retour sortie
                v->cible_y = 5;  
                for (int i=0; i<TOTAL_SPOTS; i++) {
                    if (all_spots[i].id_voiture == v->id) {
                        all_spots[i].is_occupied = 0;
                        all_spots[i].id_voiture = -1;
                    }
                }
            }
        }

        // Suppression
        if (v->etat == ETAT_SORTIE && abs(v->x - v->cible_x) <= 3 && abs(v->y - v->cible_y) <= 3) {
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
        if (draw_x < 0) draw_x = 0; if (draw_y < 0) draw_y = 0;

        for (int i=0; i<3; i++) {
            goto_xy(draw_x, draw_y + i);
            if (v->etat == ETAT_GARE) printf(BLUE_TEXT); else printf(RED_TEXT);
            printf("%s", modeles[v->type].forme[i]);
            printf(RESET);
        }
        v = v->suivant;
    }
}

void draw_all_spots(int selected_index) {
    (void)selected_index;
    for (int i = 0; i < TOTAL_SPOTS; i++) {
        if (!all_spots[i].is_occupied) {
            int base_y = all_spots[i].screen_y - 1;
            if (base_y < 0) base_y = 0;
            for (int dy = 0; dy < 3; dy++) {
                goto_xy(all_spots[i].screen_x, base_y + dy);
                printf("%s ", BG_GREEN); printf("%s", RESET);
            }
        }
    }
}

void liberer_memoire_vehicules() {
    Vehicule *v = liste_vehicules;
    while (v != NULL) {
        Vehicule *temp = v;
        v = v->suivant;
        free(temp);
    }
}