#define _XOPEN_SOURCE_EXTENDED 1
#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <ncurses.h>
#include <locale.h>
#include "../include/parking.h"

#define OFFSET_MAP 7 /* Decalage vertical du au titre dans le fichier texte */

/* --- VARIABLES GLOBALES --- */
ParkingSpot all_spots[TOTAL_SPOTS];
Vehicule *liste_vehicules = NULL;
Vehicule *voiture_joueur = NULL;
ModeleVehicule modeles[3];
char map_logique[HAUTEUR_MAX][LARGEUR_MAX];
int spawn_x, spawn_y;

/* Snapshot du decor pour la gestion de l'effacement des vehicules */
cchar_t map_snapshot[HAUTEUR_MAX][LARGEUR_MAX];

/* Initialisation des modeles graphiques des vehicules */
void init_modeles(void)
{
    /* TYPE 0 : Voiture standard */
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

    /* TYPE 1 : Camionnette */
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

    /* TYPE 2 : Compacte */
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

/* Creation du vehicule controle par l'utilisateur */
void spawner_vehicule(void)
{
    voiture_joueur = malloc(sizeof(Vehicule));
    if (!voiture_joueur)
        return;

    voiture_joueur->x = spawn_x;
    voiture_joueur->y = spawn_y;
    voiture_joueur->direction = 'O';
    voiture_joueur->tps = (unsigned long int)time(NULL);
    voiture_joueur->type = 0;
    voiture_joueur->etat = ETAT_CHERCHE_PLACE;
    voiture_joueur->clignotement = 0;
    voiture_joueur->id = 101;

    /* Insertion dans la liste chainee */
    voiture_joueur->suivant = liste_vehicules;
    liste_vehicules = voiture_joueur;
}

/* Gestion des deplacements avec verification de collision */
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
        voiture_joueur->clignotement = 10; /* Effet visuel lors d'un choc */
    }
}

/* Detection d'obstacles sur la map logique */
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
            if (c > 127 || c == '|' || c == '-' || c == '+')
                return 1;
        }
    }
    return 0;
}

/* Sauvegarde de l'etat graphique du decor avant mouvement */
void sauvegarder_background(void)
{
    cchar_t cell;
    for (int y = 0; y < HAUTEUR_MAX; y++)
    {
        for (int x = 0; x < LARGEUR_MAX; x++)
        {
            if (mvin_wch(y, x, &cell) != ERR)
                map_snapshot[y][x] = cell;
            else
                setcchar(&map_snapshot[y][x], L" ", 0, 0, NULL);
        }
    }
}

/* Restauration du decor a l'ancienne position du vehicule */
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

            if (cx >= 0 && screen_y >= 0 && screen_y < HAUTEUR_MAX && cx < LARGEUR_MAX)
            {
                mvadd_wch(screen_y, cx, &map_snapshot[screen_y][cx]);
            }
        }
    }
}

/* Mise a jour logique et affichage du HUD */
void mettre_a_jour_vehicules(void)
{
    if (voiture_joueur == NULL)
        return;

    if (voiture_joueur->clignotement > 0)
        voiture_joueur->clignotement--;

    unsigned long duree = (unsigned long)time(NULL) - voiture_joueur->tps;
    float tarif = duree * 0.10;

    attron(COLOR_PAIR(2));
    mvprintw(0, 2, "[PILOTAGE] | TEMPS: %lus | TARIF: %.2f EUR", duree, tarif);
    attroff(COLOR_PAIR(2));

    afficher_vehicule(voiture_joueur);
    refresh();
}

/* Affichage graphique du vehicule selon sa direction */
void afficher_vehicule(Vehicule *v)
{
    if (v->clignotement > 0 && v->clignotement % 2 == 0)
        return;

    if (v->etat == ETAT_GARE)
        attron(COLOR_PAIR(1));
    else if (v->clignotement > 0)
        attron(A_BOLD);
    else
        attron(COLOR_PAIR(3));

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

    attroff(COLOR_PAIR(1) | A_BOLD | COLOR_PAIR(3));
}

/* Affichage des indicateurs d'etat des places (Vert/Rouge) */
void draw_all_spots(int idx)
{
    (void)idx;
    for (int i = 0; i < TOTAL_SPOTS; i++)
    {
        for (int dy = -1; dy <= 1; dy++)
        {
            int y = all_spots[i].screen_y + dy;
            int x = all_spots[i].screen_x;

            move(y, x);
            if (all_spots[i].is_occupied)
            {
                attron(COLOR_PAIR(3) | A_REVERSE);
                addstr(" ");
                attroff(COLOR_PAIR(3) | A_REVERSE);
            }
            else
            {
                attron(COLOR_PAIR(2) | A_REVERSE);
                addstr(" ");
                attroff(COLOR_PAIR(2) | A_REVERSE);
            }
        }
    }
    refresh();
}

/* Recherche de la place de parking la plus proche du vehicule */
int verifier_place_proche(Vehicule *v)
{
    if (v == NULL)
        return -1;

    int meilleur_spot = -1;
    double distance_min = 100000.0;

    for (int i = 0; i < TOTAL_SPOTS; i++)
    {
        int dx = v->x - all_spots[i].screen_x;
        int dy = (v->y + OFFSET_MAP) - all_spots[i].screen_y;

        if (abs(dx) <= 10 && abs(dy) <= 6)
        {
            double d = sqrt((dx * dx) + (dy * 2.0 * dy * 2.0));
            if (d < distance_min)
            {
                distance_min = d;
                meilleur_spot = i;
            }
        }
    }
    return meilleur_spot;
}

void goto_xy(int x, int y) { move(y, x); }

/* Chargement et affichage de la map depuis un fichier texte */
void display_static_map(const char *f)
{
    FILE *file = fopen(f, "r");
    if (!file)
        return;

    char line[1024];
    int row = 0;
    while (fgets(line, sizeof(line), file))
    {
        mvprintw(row++, 0, "%s", line);
    }
    fclose(file);
    refresh();
}

/* Analyse du fichier map pour initialiser les places et le spawn */
void init_spots_from_map(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
        return;

    char line[1024];
    int y = 0, spot_idx = 0;

    while (fgets(line, sizeof(line), file) && y < HAUTEUR_MAX)
    {
        int vx = 0;
        for (int i = 0; line[i] != '\0' && line[i] != '\n';)
        {
            unsigned char c = (unsigned char)line[i];
            int char_len = (c >= 0xe0) ? 3 : 1;
            int real_y = y - OFFSET_MAP;

            if (real_y >= 0)
            {
                map_logique[real_y][vx] = line[i];
                if (line[i] == 'D')
                {
                    spawn_x = vx;
                    spawn_y = real_y;
                }
                if (line[i] == '@' && spot_idx < TOTAL_SPOTS)
                {
                    all_spots[spot_idx].screen_x = vx;
                    all_spots[spot_idx].screen_y = y;
                    all_spots[spot_idx].is_occupied = 0;
                    all_spots[spot_idx].id_voiture = -1;
                    spot_idx++;
                    map_logique[real_y][vx] = ' ';
                }
            }
            i += char_len;
            vx++;
        }
        y++;
    }
    fclose(file);
}

/* Nettoyage de la memoire dynamique des vehicules */
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