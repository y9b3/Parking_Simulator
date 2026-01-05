#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "../include/parking.h"

// --- VARIABLES GLOBALES ---
ParkingSpot all_spots[TOTAL_SPOTS];
Vehicule *liste_vehicules = NULL;
Vehicule *voiture_joueur = NULL; // Pointeur vers le véhicule piloté
ModeleVehicule modeles[3];
char map_logique[HAUTEUR_MAX][LARGEUR_MAX];
int spawn_x, spawn_y;
int compteur_id_vehicule = 0;

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

    // On utilise les coordonnées exactes stockées
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
        // COLLISION : On active le clignotement pour 10 frames
        voiture_joueur->clignotement = 10;
    }
}

int est_obstacle(int x, int y)
{
    // On vérifie un carré de 5x3 autour du futur centre
    for (int dy = -1; dy <= 1; dy++)
    {
        for (int dx = -2; dx <= 2; dx++)
        {
            int cx = x + dx;
            int cy = y + dy;
            if (cx < 0 || cx >= LARGEUR_MAX || cy < 0 || cy >= HAUTEUR_MAX)
                return 1;

            unsigned char c = (unsigned char)map_logique[cy][cx];
            // Si on touche un mur (Unicode > 127 ou bordure ASCII)
            if (c > 127 || c == '|' || c == '-' || c == '+')
                return 1;
        }
    }
    return 0;
}

void effacer_vehicule(Vehicule *v)
{
    // Taille exacte de la voiture pour ne pas déborder sur les murs à côté
    int l = (v->direction == 'N' || v->direction == 'S') ? 5 : 9;
    int h = (v->direction == 'N' || v->direction == 'S') ? 5 : 3;

    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < l; j++)
        {
            int cx = v->x - (l / 2) + j;
            int cy = v->y - (h / 2) + i;
            if (cx >= 0 && cy >= 0 && cy < HAUTEUR_MAX && cx < LARGEUR_MAX)
            {
                goto_xy(cx, cy);
                // On restaure le décor de fond
                putchar(map_logique[cy][cx]);
            }
        }
    }
}

void mettre_a_jour_vehicules(void)
{
    if (voiture_joueur == NULL)
        return;
    if (voiture_joueur->clignotement > 0)
    {
        voiture_joueur->clignotement--;
    }

    // Calcul du tarif en temps réel (0.10€ / seconde)
    unsigned long duree = (unsigned long)time(NULL) - voiture_joueur->tps;
    float tarif = duree * 0.10;

    // HUD en haut de l'écran
    goto_xy(2, 0);
    printf("\033[1;32m[PILOTAGE] | TEMPS: %lus | TARIF: %.2f EUR \033[0m", duree, tarif);

    // Affichage dans le cadre map dédié
    goto_xy(20, 8);
    printf("%.2f € ", tarif);

    afficher_vehicule(voiture_joueur);
}

void afficher_vehicule(Vehicule *v)
{
    // Si la voiture clignote, on ne l'affiche qu'une frame sur deux
    if (v->clignotement > 0 && v->clignotement % 2 == 0)
    {
        return; // Saute l'affichage pour cette frame
    }

    if (v->etat == ETAT_GARE)
        printf("\033[1;34m"); // Bleu
    else if (v->clignotement > 0)
        printf("\033[1;37m"); // Blanc pendant le choc
    else
        printf("\033[1;31m"); // Rouge normal

    if (v->direction == 'E' || v->direction == 'O')
    {
        for (int i = 0; i < 3; i++)
        {
            goto_xy(v->x - 4, v->y - 1 + i);
            printf("%s", modeles[v->type].forme[i]);
        }
    }
    else
    {
        for (int i = 0; i < 5; i++)
        {
            goto_xy(v->x - 2, v->y - 2 + i);
            printf("%s", modeles[v->type].forme_v[i]);
        }
    }
    printf("\033[0m");
}

void draw_all_spots(int idx)
{
    (void)idx;
    for (int i = 0; i < TOTAL_SPOTS; i++)
    {
        // On dessine la place sur 3 lignes de haut pour qu'elle soit bien visible
        for (int dy = -1; dy <= 1; dy++)
        {
            goto_xy(all_spots[i].screen_x, all_spots[i].screen_y + dy);

            if (all_spots[i].is_occupied)
            {
                // ROUGE : Fond rouge (\033[41m)
                printf("\033[41m \033[0m");
            }
            else
            {
                // VERT : Fond vert (\033[42m)
                printf("\033[42m \033[0m");
            }
        }
    }
    fflush(stdout);
}
int verifier_place_proche(Vehicule *v)
{
    if (v == NULL)
        return -1;
    for (int i = 0; i < TOTAL_SPOTS; i++)
    {
        // Distance entre le centre de la voiture et le centre enregistré de la place
        int dx = abs(v->x - all_spots[i].screen_x);
        int dy = abs(v->y - all_spots[i].screen_y);

        // On accepte une large zone (un carré de 8x6)
        if (dx <= 5 && dy <= 3)
        {
            return i;
        }
    }
    return -1;
}

void goto_xy(int x, int y) { printf("\033[%d;%dH", y + 1, x + 1); }

void display_static_map(const char *f)
{
    FILE *file = fopen(f, "r");
    if (!file)
        return;
    printf("\033[2J\033[H");
    int c;
    while ((c = fgetc(file)) != EOF)
        putchar(c);
    fclose(file);
}

void init_spots_from_map(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
        return;

    char line[1024];
    int y = 0;
    int spot_idx = 0;

    while (fgets(line, sizeof(line), file) && y < HAUTEUR_MAX)
    {
        int vx = 0;
        for (int i = 0; line[i] != '\0' && line[i] != '\n';)
        {
            unsigned char c = (unsigned char)line[i];
            int char_len = (c >= 0xe0) ? 3 : 1;

            // --- ON REVIENT AU DÉCALAGE DE RÉFÉRENCE ---
            int real_y = y - 7;

            if (real_y >= 0)
            {
                map_logique[real_y][vx] = line[i];

                if (line[i] == 'D')
                {
                    spawn_x = vx;
                    spawn_y = real_y;
                    // Tunnel pour sortir du box D
                    for (int dy = -2; dy <= 2; dy++)
                    {
                        for (int dx = -10; dx <= 0; dx++)
                        {
                            if (real_y + dy >= 0 && vx + dx >= 0)
                                map_logique[real_y + dy][vx + dx] = ' ';
                        }
                    }
                }

                if (line[i] == '@' && spot_idx < TOTAL_SPOTS)
                {
                    // On enregistre la position précise pour all_spots
                    all_spots[spot_idx].screen_x = vx;
                    all_spots[spot_idx].screen_y = real_y;
                    all_spots[spot_idx].is_occupied = 0;
                    all_spots[spot_idx].id_voiture = -1;
                    spot_idx++;

                    // CRUCIAL : On remplace le '@' par un vide pour ne pas l'afficher en texte
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