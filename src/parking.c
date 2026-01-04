#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <math.h> // Pour abs()
#include "../include/parking.h"

// Couleurs ANSI
#define RESET "\033[0m"
#define CLEAR_SCREEN "\033[2J"
#define CURSOR_HOME "\033[H"
#define RED_TEXT "\033[91m"  // Voiture qui roule
#define BLUE_TEXT "\033[34m" // Voiture garée
#define BG_GREEN "\033[42m"  // Place libre
#define SPOT_CHAR '@'        

// --- REGLAGES DU CHEMIN (A ajuster selon votre carte) ---
// Coordonnée X de l'allée centrale verticale où les voitures circulent
#define WP_ALLEE_CENTRALE_X 130 
// Coordonnées de la sortie finale (en haut à gauche)
#define WP_SORTIE_X 10
#define WP_SORTIE_Y 5

// --- VARIABLES GLOBALES ---
ParkingSpot all_spots[TOTAL_SPOTS];
Vehicule *liste_vehicules = NULL;
ModeleVehicule modeles[3];
char map_logique[HAUTEUR_MAX][LARGEUR_MAX]; 
int compteur_id_vehicule = 0;
int spawn_x = 0;
int spawn_y = 0;

void init_modeles(void) {
    modeles[0].id = 0; modeles[0].largeur = 9;
    modeles[0].forme[0] = "┌═╦═════╗"; modeles[0].forme[1] = "║ ║▆   ║"; modeles[0].forme[2] = "└═╩═════╝";
    modeles[1].id = 1; modeles[1].largeur = 12;
    modeles[1].forme[0] = "╔════════╦═┐"; modeles[1].forme[1] = "║      ▅║ │"; modeles[1].forme[2] = "╚════════╩═┘";
    modeles[2].id = 2; modeles[2].largeur = 10;
    modeles[2].forme[0] = "┌──┬───┬─╗"; modeles[2].forme[1] = "│  ║ ║ ║ │"; modeles[2].forme[2] = "└──┴───┴─╝";
}

void goto_xy(int x, int y) { printf("\033[%d;%dH", y + 1, x + 1); }

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
    for (int y = 0; y < HAUTEUR_MAX; y++)
        for (int x = 0; x < LARGEUR_MAX; x++) map_logique[y][x] = ' ';

    FILE *file = fopen(filename, "r");
    if (!file) return;

    char line[1024];
    int y = 0, idx_spot = 0;
    while (fgets(line, sizeof(line), file) && y < HAUTEUR_MAX) {
        int visual_x = 0;
        for (int i = 0; line[i] != '\0' && line[i] != '\n';) {
            unsigned char c = (unsigned char)line[i];
            int char_len = (c >= 0xf0) ? 4 : (c >= 0xe0) ? 3 : (c >= 0xc0) ? 2 : 1;

            if (line[i] == 'D') {
                spawn_x = visual_x; spawn_y = y;
                map_logique[y][visual_x] = ' ';
            } else if (line[i] == SPOT_CHAR) {
                if (idx_spot < TOTAL_SPOTS) {
                    all_spots[idx_spot].screen_x = visual_x;
                    all_spots[idx_spot].screen_y = y;
                    all_spots[idx_spot].is_occupied = 0;
                    all_spots[idx_spot].id_voiture = -1;
                    idx_spot++;
                }
                map_logique[y][visual_x] = ' ';
            } else {
                if (visual_x < LARGEUR_MAX) map_logique[y][visual_x] = line[i];
            }
            i += char_len; visual_x++;
        }
        y++;
    }
    fclose(file);
}

int est_obstacle(int x, int y) {
    if (x < 0 || x >= LARGEUR_MAX || y < 0 || y >= HAUTEUR_MAX) return 1;
    char c = map_logique[y][x];
    // Route autorisée : espace, flèches, point, ou symbole de place
    if (c == ' ' || c == '.' || c == '>' || c == '<' || c == '^' || c == 'v' || c == SPOT_CHAR) return 0;
    return 1; // Tout le reste (murs |, -, +) est un obstacle
}

int est_bloque_par_voiture(int x, int y, int mon_id) {
    Vehicule *v = liste_vehicules;
    while (v != NULL) {
        if (v->id != mon_id && abs(v->x - x) < 12 && abs(v->y - y) < 2) return 1;
        v = v->suivant;
    }
    return 0;
}

void effacer_vehicule(Vehicule *v) {
    int largeur = modeles[v->type].largeur;
    int draw_x = v->x - (largeur / 2);
    if (draw_x < 0) draw_x = 0;
    for (int i = 0; i < 3; i++) {
        goto_xy(draw_x, v->y - 1 + i);
        for (int j = 0; j < largeur; j++) printf(" ");
    }
}

void spawner_vehicule(void) {
    if (est_bloque_par_voiture(spawn_x, spawn_y, -1)) return;
    Vehicule *nouveau = malloc(sizeof(Vehicule));
    if (!nouveau) return;
    nouveau->id = compteur_id_vehicule++;
    nouveau->x = spawn_x; nouveau->y = spawn_y;
    nouveau->type = rand() % 3;
    nouveau->etat = ETAT_CHERCHE_PLACE;
    
    int place_trouvee = -1;
    for (int i = 0; i < TOTAL_SPOTS; i++) {
        if (!all_spots[i].is_occupied) { place_trouvee = i; break; }
    }
    if (place_trouvee != -1) {
        nouveau->cible_x = all_spots[place_trouvee].screen_x;
        nouveau->cible_y = all_spots[place_trouvee].screen_y;
        all_spots[place_trouvee].is_occupied = 1;
        all_spots[place_trouvee].id_voiture = nouveau->id;
    } else {
        nouveau->etat = ETAT_SORTIE;
        nouveau->cible_x = WP_SORTIE_X; nouveau->cible_y = WP_SORTIE_Y;
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
            // Cible temporaire pour le mouvement en équerre
            int tx = v->cible_x;
            int ty = v->cible_y;

            // Si on cherche une place et qu'on n'est pas encore dans l'allée centrale
            if (v->etat == ETAT_CHERCHE_PLACE && abs(v->x - WP_ALLEE_CENTRALE_X) > 2) {
                tx = WP_ALLEE_CENTRALE_X;
                ty = v->y; // On reste sur la ligne horizontale de l'entrée
            }

            int next_x = v->x;
            int next_y = v->y;

            // Mouvement horizontal prioritaire
            if (v->x < tx) next_x++;
            else if (v->x > tx) next_x--;
            
            // Si aligné en X, on bouge en Y
            if (next_x == v->x) {
                if (v->y < ty) next_y++;
                else if (v->y > ty) next_y--;
            }

            if (!est_obstacle(next_x, next_y) && !est_bloque_par_voiture(next_x, next_y, v->id)) {
                v->x = next_x; v->y = next_y;
            }
        }

        // Arrivée à destination
        if (v->etat == ETAT_CHERCHE_PLACE && abs(v->x - v->cible_x) <= 2 && abs(v->y - v->cible_y) <= 2) {
            v->x = v->cible_x; v->y = v->cible_y;
            v->etat = ETAT_GARE;
            v->temps_gare = 50 + (rand() % 100);
        } else if (v->etat == ETAT_GARE && --v->temps_gare <= 0) {
            v->etat = ETAT_SORTIE;
            v->cible_x = WP_SORTIE_X; v->cible_y = WP_SORTIE_Y;
            for (int i = 0; i < TOTAL_SPOTS; i++) {
                if (all_spots[i].id_voiture == v->id) {
                    all_spots[i].is_occupied = 0; all_spots[i].id_voiture = -1;
                }
            }
        }

        // Suppression en sortie
        if (v->etat == ETAT_SORTIE && abs(v->x - v->cible_x) <= 3 && abs(v->y - v->cible_y) <= 3) {
            Vehicule *tmp = v;
            if (precedent == NULL) { liste_vehicules = v->suivant; v = liste_vehicules; }
            else { precedent->suivant = v->suivant; v = v->suivant; }
            free(tmp); continue;
        }
        precedent = v; v = v->suivant;
    }
}

void afficher_vehicules_dynamiques(void) {
    Vehicule *v = liste_vehicules;
    while (v != NULL) {
        int largeur = modeles[v->type].largeur;
        for (int i = 0; i < 3; i++) {
            goto_xy(v->x - (largeur / 2), v->y - 1 + i);
            printf("%s%s%s", (v->etat == ETAT_GARE ? BLUE_TEXT : RED_TEXT), modeles[v->type].forme[i], RESET);
        }
        v = v->suivant;
    }
}

void draw_all_spots(int selected_index) {
    (void)selected_index;
    for (int i = 0; i < TOTAL_SPOTS; i++) {
        if (!all_spots[i].is_occupied) {
            for (int dy = -1; dy <= 1; dy++) {
                goto_xy(all_spots[i].screen_x, all_spots[i].screen_y + dy);
                printf("%s %s", BG_GREEN, RESET);
            }
        }
    }
}

void liberer_memoire_vehicules() {
    Vehicule *v = liste_vehicules;
    while (v != NULL) {
        Vehicule *temp = v; v = v->suivant; free(temp);
    }
    liste_vehicules = NULL;
}
