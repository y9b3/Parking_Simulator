#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../include/parking.h"

#define RESET "\033[0m"
#define RED_TEXT "\033[91m"
#define BLUE_TEXT "\033[34m"
#define BG_GREEN "\033[42m"
#define SPOT_CHAR '@'

// --- RÉGLAGES DU CHEMIN ---
// Remplacez 135 par la colonne X de votre route verticale (utilisez ZQSD pour trouver)
#define WP_ALLEE_CENTRALE_X 135 
#define WP_SORTIE_X 10
#define WP_SORTIE_Y 5

ParkingSpot all_spots[TOTAL_SPOTS];
Vehicule *liste_vehicules = NULL;
ModeleVehicule modeles[3];
char map_logique[HAUTEUR_MAX][LARGEUR_MAX];
int compteur_id_vehicule = 0;
int spawn_x = 0, spawn_y = 0;

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
    printf("\033[2J\033[H");
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
        for (int i = 0; line[i] != '\0' && line[i] != '\n' && line[i] != '\r';) {
            unsigned char c = (unsigned char)line[i];
            int len = (c >= 0xf0) ? 4 : (c >= 0xe0) ? 3 : (c >= 0xc0) ? 2 : 1;
            if (line[i] == 'D') { spawn_x = visual_x; spawn_y = y; map_logique[y][visual_x] = ' '; }
            else if (line[i] == SPOT_CHAR) {
                if (idx_spot < TOTAL_SPOTS) {
                    all_spots[idx_spot].screen_x = visual_x;
                    all_spots[idx_spot].screen_y = y; // Calibrage hauteur @
                    all_spots[idx_spot].is_occupied = 0;
                    all_spots[idx_spot].id_voiture = -1;
                    idx_spot++;
                }
                map_logique[y][visual_x] = ' ';
            } else { if (visual_x < LARGEUR_MAX) map_logique[y][visual_x] = line[i]; }
            i += len; visual_x++;
        }
        y++;
    }
    fclose(file);
}

int est_obstacle(int x, int y) {
    if (x < 0 || x >= LARGEUR_MAX || y < 0 || y >= HAUTEUR_MAX) return 1;
    char c = map_logique[y][x];
    // Seuls ces caractères autorisent le passage
    if (c == ' ' || c == '.' || c == '>' || c == '<' || c == '^' || c == 'v' || c == SPOT_CHAR) return 0;
    return 1;
}

int est_bloque_par_voiture(int x, int y, int mon_id) {
    Vehicule *v = liste_vehicules;
    while (v) {
        if (v->id != mon_id && abs(v->x - x) < 14 && abs(v->y - y) < 3) return 1;
        v = v->suivant;
    }
    return 0;
}

void effacer_vehicule(Vehicule *v) {
    int largeur = modeles[v->type].largeur;
    for (int i = 0; i < 3; i++) {
        goto_xy(v->x - (largeur / 2), v->y - 1 + i);
        for (int j = 0; j < largeur; j++) printf(" ");
    }
}

void spawner_vehicule(void) {
    if (est_bloque_par_voiture(spawn_x, spawn_y, -1)) return;
    Vehicule *n = malloc(sizeof(Vehicule));
    if (!n) return;
    n->id = compteur_id_vehicule++; n->x = spawn_x; n->y = spawn_y;
    n->type = rand() % 3; n->etat = ETAT_CHERCHE_PLACE; n->etape_trajet = 0;
    int p = -1;
    for (int i = 0; i < TOTAL_SPOTS; i++)
        if (!all_spots[i].is_occupied) { p = i; break; }
    if (p != -1) {
        n->cible_x = all_spots[p].screen_x; n->cible_y = all_spots[p].screen_y;
        all_spots[p].is_occupied = 1; all_spots[p].id_voiture = n->id;
    } else { n->etat = ETAT_SORTIE; n->cible_x = WP_SORTIE_X; n->cible_y = WP_SORTIE_Y; }
    n->suivant = liste_vehicules; liste_vehicules = n;
}

void mettre_a_jour_vehicules(void) {
    Vehicule *v = liste_vehicules; Vehicule *prec = NULL;
    while (v) {
        effacer_vehicule(v);
        if (v->etat != ETAT_GARE) {
            int tx = v->cible_x, ty = v->cible_y;
            if (v->etat == ETAT_CHERCHE_PLACE && v->etape_trajet == 0) {
                tx = WP_ALLEE_CENTRALE_X; ty = v->y;
                if (abs(v->x - tx) < 2) v->etape_trajet = 1;
            }
            int nx = v->x + ((tx > v->x) ? 1 : (tx < v->x ? -1 : 0));
            int ny = v->y + ((ty > v->y) ? 1 : (ty < v->y ? -1 : 0));
            // Priorité X (rejoindre l'allée) puis Y
            if (nx != v->x && !est_obstacle(nx, v->y) && !est_bloque_par_voiture(nx, v->y, v->id)) v->x = nx;
            else if (ny != v->y && !est_obstacle(v->x, ny) && !est_bloque_par_voiture(v->x, ny, v->id)) v->y = ny;
        }
        if (v->etat == ETAT_CHERCHE_PLACE && v->etape_trajet == 1 && abs(v->x - v->cible_x) < 2 && abs(v->y - v->cible_y) < 2) {
            v->x = v->cible_x; v->y = v->cible_y; v->etat = ETAT_GARE; v->temps_gare = 100 + rand()%150;
        } else if (v->etat == ETAT_GARE && --v->temps_gare <= 0) {
            v->etat = ETAT_SORTIE; v->cible_x = WP_SORTIE_X; v->cible_y = WP_SORTIE_Y;
            for(int i=0; i<TOTAL_SPOTS; i++) if(all_spots[i].id_voiture == v->id) { all_spots[i].is_occupied = 0; all_spots[i].id_voiture = -1; }
        }
        if (v->etat == ETAT_SORTIE && abs(v->x - v->cible_x) < 3 && abs(v->y - v->cible_y) < 3) {
            Vehicule *tmp = v; if (!prec) liste_vehicules = v->suivant; else prec->suivant = v->suivant;
            v = v->suivant; free(tmp); continue;
        }
        prec = v; v = v->suivant;
    }
}

void afficher_vehicules_dynamiques(void) {
    Vehicule *v = liste_vehicules;
    while (v) {
        int lx = modeles[v->type].largeur;
        for (int i = 0; i < 3; i++) {
            goto_xy(v->x - (lx / 2), v->y - 1 + i);
            printf("%s%s%s", (v->etat == ETAT_GARE ? BLUE_TEXT : RED_TEXT), modeles[v->type].forme[i], RESET);
        }
        v = v->suivant;
    }
}

void draw_all_spots(int sel) {
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
    while (v) { Vehicule *t = v; v = v->suivant; free(t); }
    liste_vehicules = NULL;
}
