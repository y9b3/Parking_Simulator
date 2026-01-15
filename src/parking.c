#define _XOPEN_SOURCE_EXTENDED 1
#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <ncurses.h> // INDISPENSABLE
#include <locale.h>  // Pour l'Unicode
#include "../include/parking.h"

#define OFFSET_MAP 7 // Le décalage dû à l'en-tête du fichier texte

// --- VARIABLES GLOBALES ---
ParkingSpot all_spots[TOTAL_SPOTS];
Vehicule *liste_vehicules = NULL;
Vehicule *voiture_joueur = NULL;
ModeleVehicule modeles[3];
char map_logique[HAUTEUR_MAX][LARGEUR_MAX];
int spawn_x, spawn_y;

cchar_t map_snapshot[HAUTEUR_MAX][LARGEUR_MAX];

void init_modeles(void)
{
    // TYPE 0 : Voiture standard
    modeles[0].id = 0;
    modeles[0].largeur = 9;
    modeles[0].forme[0] = "┌═╦═════╗";
    modeles[0].forme[1] = "║ ║▆    ║";
    modeles[0].forme[2] = "└═╩═════╝";
    modeles[0].forme_v[0] = "┌═══┐";
    modeles[0].forme_v[1] = "║▆ ║";
    modeles[0].forme_v[2] = "╠═══╣";
    modeles[0].forme_v[3] = "║   ║";
    modeles[0].forme_v[4] = "╚═══╝";

    // TYPE 1 : Camionnette
    modeles[1].id = 1;
    modeles[1].largeur = 12;
    modeles[1].forme[0] = "┌─╦════════╗";
    modeles[1].forme[1] = "│ ║▅       ║";
    modeles[1].forme[2] = "└─╩════════╝";
    modeles[1].forme_v[0] = "┌────┐";
    modeles[1].forme_v[1] = "│▅   │";
    modeles[1].forme_v[2] = "╠════╣";
    modeles[1].forme_v[3] = "║    ║";
    modeles[1].forme_v[4] = "╚════╝";

    // TYPE 2 : Compacte
    modeles[2].id = 2;
    modeles[2].largeur = 10;
    modeles[2].forme[0] = "╔─┬───┬──┐";
    modeles[2].forme[1] = "│ ║ ║ ║  │";
    modeles[2].forme[2] = "╚─┴───┴──┘";
    modeles[2].forme_v[0] = "┌───┐";
    modeles[2].forme_v[1] = "│║ ║│";
    modeles[2].forme_v[2] = "│║ ║│";
    modeles[2].forme_v[3] = "│║ ║│";
    modeles[2].forme_v[4] = "└───┘";
}

void spawner_vehicule(void)
{
    voiture_joueur = malloc(sizeof(Vehicule));
    if (!voiture_joueur)
        return;

    // La voiture utilise les coordonnées LOGIQUES (relatives à la map sans header)
    voiture_joueur->x = spawn_x;
    voiture_joueur->y = spawn_y;

    voiture_joueur->direction = 'O';
    voiture_joueur->tps = (unsigned long int)time(NULL);
    voiture_joueur->type = 0;
    voiture_joueur->etat = ETAT_CHERCHE_PLACE;
    voiture_joueur->clignotement = 0;
    voiture_joueur->id = 101;
    voiture_joueur->suivant = liste_vehicules;
    liste_vehicules = voiture_joueur;
}

void deplacer_joueur(int dx, int dy, char dir)
{
    if (!voiture_joueur)
        return;
    int nx = voiture_joueur->x + dx;
    int ny = voiture_joueur->y + dy;

    if (!est_obstacle(nx, ny))
    {
        effacer_vehicule(voiture_joueur);
        voiture_joueur->x = nx;
        voiture_joueur->y = ny;
        voiture_joueur->direction = dir;
    }
    else
    {
        voiture_joueur->clignotement = 10;
    }
}

int est_obstacle(int x, int y)
{
    for (int dy = -1; dy <= 1; dy++)
    {
        for (int dx = -2; dx <= 2; dx++)
        {
            int cx = x + dx;
            int cy = y + dy;
            if (cx < 0 || cx >= LARGEUR_MAX || cy < 0 || cy >= HAUTEUR_MAX)
                return 1;

            unsigned char c = (unsigned char)map_logique[cy][cx];
            // Murs standards et Unicode étendu
            if (c > 127 || c == '|' || c == '-' || c == '+')
                return 1;
        }
    }
    return 0;
}
void sauvegarder_background(void)
{
    cchar_t cell;
    int res;

    // On parcourt toute la taille théorique de la map
    for (int y = 0; y < HAUTEUR_MAX; y++)
    {
        for (int x = 0; x < LARGEUR_MAX; x++)
        {

            // On essaie de lire
            res = mvin_wch(y, x, &cell);

            if (res != ERR)
            {
                map_snapshot[y][x] = cell;
            }
            else
            {
                // Si Ncurses refuse de lire (hors zone), on force un caractère de debug
                // Si tu vois des points rouges, c'est que la zone est hors limite
                setcchar(&map_snapshot[y][x], L".", 0, 0, NULL);
            }
        }
    }
}
void effacer_vehicule(Vehicule *v)
{
    int l = (v->direction == 'N' || v->direction == 'S') ? 5 : 9;
    int h = (v->direction == 'N' || v->direction == 'S') ? 5 : 3;

    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < l; j++)
        {
            int cx = v->x - (l / 2) + j;
            int cy = v->y - (h / 2) + i;
            int screen_y = cy + OFFSET_MAP;

            // On vérifie juste qu'on est dans le tableau
            if (cx >= 0 && screen_y >= 0 && screen_y < HAUTEUR_MAX && cx < LARGEUR_MAX)
            {
                mvadd_wch(screen_y, cx, &map_snapshot[screen_y][cx]);
            }
        }
    }
}

void mettre_a_jour_vehicules(void)
{
    if (voiture_joueur == NULL)
        return;

    if (voiture_joueur->clignotement > 0)
        voiture_joueur->clignotement--;

    unsigned long duree = (unsigned long)time(NULL) - voiture_joueur->tps;
    float tarif = duree * 0.10;

    // HUD - Utilisation de ncurses mvprintw
    attron(COLOR_PAIR(2)); // Vert
    mvprintw(0, 2, "[PILOTAGE] | TEMPS: %lus | TARIF: %.2f EUR", duree, tarif);
    attroff(COLOR_PAIR(2));

    // Affichage dans le cadre map
    mvprintw(8, 20, "%.2f E", tarif); // Attention au symbole Euro parfois capricieux

    afficher_vehicule(voiture_joueur);
    refresh(); // Rafraîchissement global
}

void afficher_vehicule(Vehicule *v)
{
    if (v->clignotement > 0 && v->clignotement % 2 == 0)
        return;

    // Gestion des couleurs ncurses
    if (v->etat == ETAT_GARE)
        attron(COLOR_PAIR(1)); // Cyan/Bleu si garé
    else if (v->clignotement > 0)
        attron(A_BOLD); // Blanc brillant si choc
    else
        attron(COLOR_PAIR(3)); // Rouge normal

    // On dessine avec l'OFFSET_MAP pour s'aligner sur la map
    if (v->direction == 'E' || v->direction == 'O')
    {
        for (int i = 0; i < 3; i++)
            mvprintw((v->y - 1 + i) + OFFSET_MAP, v->x - 4, "%s", modeles[v->type].forme[i]);
    }
    else
    {
        for (int i = 0; i < 5; i++)
            mvprintw((v->y - 2 + i) + OFFSET_MAP, v->x - 2, "%s", modeles[v->type].forme_v[i]);
    }

    // Reset des couleurs
    if (v->etat == ETAT_GARE)
        attroff(COLOR_PAIR(1));
    else if (v->clignotement > 0)
        attroff(A_BOLD);
    else
        attroff(COLOR_PAIR(3));
}

void draw_all_spots(int idx)
{
    (void)idx;
    for (int i = 0; i < TOTAL_SPOTS; i++)
    {
        for (int dy = -1; dy <= 1; dy++)
        {
            // Position absolue à l'écran (calculée dans init_spots)
            int y = all_spots[i].screen_y + dy;
            int x = all_spots[i].screen_x;

            move(y, x); // Déplace le curseur ncurses

            if (all_spots[i].is_occupied)
            {
                // Simulation fond Rouge avec texte inversé
                attron(COLOR_PAIR(3) | A_REVERSE);
                addstr(" ");
                attroff(COLOR_PAIR(3) | A_REVERSE);
            }
            else
            {
                // Simulation fond Vert avec texte inversé
                attron(COLOR_PAIR(2) | A_REVERSE);
                addstr(" ");
                attroff(COLOR_PAIR(2) | A_REVERSE);
            }
        }
    }
    refresh();
}

int verifier_place_proche(Vehicule *v)
{
    if (v == NULL)
        return -1;

    int meilleur_spot = -1;
    double distance_min = 100000.0;

    // IMPORTANT : On ajoute 7 car les places sont enregistrées avec le décalage du titre
    int offset_map = 7;

    for (int i = 0; i < TOTAL_SPOTS; i++)
    {
        // Axe X : Pas de décalage, on compare direct
        int dx = v->x - all_spots[i].screen_x;

        // Axe Y : ON AJOUTE LE DÉCALAGE à la voiture pour qu'elle "parle la même langue" que la place
        int dy = (v->y + offset_map) - all_spots[i].screen_y;

        // Zone de tolérance
        if (abs(dx) <= 10 && abs(dy) <= 6)
        {
            // Formule de distance précise
            double distance_reelle = sqrt((dx * dx) + (dy * 2.0 * dy * 2.0));

            if (distance_reelle < distance_min)
            {
                distance_min = distance_reelle;
                meilleur_spot = i;
            }
        }
    }
    return meilleur_spot;
}
// Plus besoin de goto_xy, ncurses a "move" ou "mvprintw"
void goto_xy(int x, int y) { move(y, x); }

void display_static_map(const char *f)
{
    FILE *file = fopen(f, "r");
    if (!file)
        return;

    char line[1024];
    int row = 0;

    // On imprime le fichier ligne par ligne
    while (fgets(line, sizeof(line), file))
    {
        mvprintw(row, 0, "%s", line);
        row++;
    }
    fclose(file);
    refresh();
}

void init_spots_from_map(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
        return;

    char line[1024];
    int y = 0; // Ligne absolue du fichier
    int spot_idx = 0;

    while (fgets(line, sizeof(line), file) && y < HAUTEUR_MAX)
    {
        int vx = 0;
        for (int i = 0; line[i] != '\0' && line[i] != '\n';)
        {
            unsigned char c = (unsigned char)line[i];
            int char_len = (c >= 0xe0) ? 3 : 1;

            // Coordonnée logique pour la map (on ignore le header de 7 lignes)
            int real_y = y - OFFSET_MAP;

            if (real_y >= 0)
            {
                map_logique[real_y][vx] = line[i];

                if (line[i] == 'D')
                {
                    spawn_x = vx;
                    spawn_y = real_y; // Spawn stocké en coordonnées logiques

                    // Tunnel de sortie
                    for (int dy = -2; dy <= 2; dy++)
                        for (int dx = -10; dx <= 0; dx++)
                            if (real_y + dy >= 0 && vx + dx >= 0)
                                map_logique[real_y + dy][vx + dx] = ' ';
                }

                if (line[i] == '@' && spot_idx < TOTAL_SPOTS)
                {
                    all_spots[spot_idx].screen_x = vx;
                    // CORRECTION MAJEURE : On stocke la coordonnée ÉCRAN (y) et non logique
                    all_spots[spot_idx].screen_y = y;

                    all_spots[spot_idx].is_occupied = 0;
                    all_spots[spot_idx].id_voiture = -1;
                    spot_idx++;

                    map_logique[real_y][vx] = ' '; // On efface le @ de la logique
                }
            }
            i += char_len;
            vx++;
        }
        y++;
    }
    fclose(file);
}

void liberer_memoire_vehicules()
{
    Vehicule *v = liste_vehicules;
    while (v)
    {
        Vehicule *t = v;
        v = v->suivant;
        free(t);
    }
    liste_vehicules = NULL;
    voiture_joueur = NULL;
}