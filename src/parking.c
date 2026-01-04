#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h> // Pour abs()
#include "../include/parking.h"

// Couleurs ANSI
#define RESET "\033[0m"
#define CLEAR_SCREEN "\033[2J"
#define CURSOR_HOME "\033[H"
#define RED_TEXT "\033[91m"  // Voiture qui roule
#define BLUE_TEXT "\033[34m" // Voiture garée
#define BG_GREEN "\033[42m"  // Place libre
#define SPOT_CHAR '@'        // Symbole dans le .txt

// --- VARIABLES GLOBALES ---
ParkingSpot all_spots[TOTAL_SPOTS];
Vehicule *liste_vehicules = NULL;
ModeleVehicule modeles[3];
char map_logique[HAUTEUR_MAX][LARGEUR_MAX]; // Grille de collision
int compteur_id_vehicule = 0;

// --- INITIALISATION ---
// Définition des variables globales pour le point d'entrée 'D'
int spawn_x = 0;
int spawn_y = 0;
int sortie_x = 0;
int sortie_y = 0;

void init_modeles(void)
{
    // --- TYPE 0 : TA VOITURE STANDARD ---
    modeles[0].id = 0;
    modeles[0].largeur = 9;
    // Horizontal (E/O) - Ta version
    modeles[0].forme[0] = "┌═╦═════╗";
    modeles[0].forme[1] = "║ ║▆    ║";
    modeles[0].forme[2] = "└═╩═════╝";
    // Vertical (N/S) - La même, pivotée (Largeur 5, Hauteur 5)
    modeles[0].forme_v[0] = "┌═══┐";
    modeles[0].forme_v[1] = "║▆ ║";  // Le ▆ (vitre) est maintenant en haut
    modeles[0].forme_v[2] = "╠═══╣"; // Le double trait ╦ devient un ╠
    modeles[0].forme_v[3] = "║   ║";
    modeles[0].forme_v[4] = "╚═══╝";

    // --- TYPE 1 : TA CAMIONNETTE ---
    modeles[1].id = 1;
    modeles[1].largeur = 12;
    // Horizontal (E/O) - Ta version
    modeles[1].forme[0] = "┌─╦════════╗";
    modeles[1].forme[1] = "│ ║▅       ║";
    modeles[1].forme[2] = "└─╩════════╝";
    // Vertical (N/S) - Pivotée
    modeles[1].forme_v[0] = "┌────┐";
    modeles[1].forme_v[1] = "│▅   │";
    modeles[1].forme_v[2] = "╠════╣";
    modeles[1].forme_v[3] = "║    ║";
    modeles[1].forme_v[4] = "╚════╝";

    // --- TYPE 2 : TA COMPACTE ---
    modeles[2].id = 2;
    modeles[2].largeur = 10;
    // Horizontal (E/O) - Ta version
    modeles[2].forme[0] = "╔─┬───┬──┐";
    modeles[2].forme[1] = "│ ║ ║ ║  │";
    modeles[2].forme[2] = "╚─┴───┴──┘";
    // Vertical (N/S) - Pivotée
    modeles[2].forme_v[0] = "┌───┐";
    modeles[2].forme_v[1] = "│║ ║│";
    modeles[2].forme_v[2] = "│║ ║│";
    modeles[2].forme_v[3] = "│║ ║│";
    modeles[2].forme_v[4] = "└───┘";
}

void goto_xy(int x, int y)
{
    printf("\033[%d;%dH", y + 1, x + 1);
}

void display_static_map(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
        return;
    printf(CLEAR_SCREEN);
    printf(CURSOR_HOME);
    int c;
    while ((c = fgetc(file)) != EOF)
        putchar(c);
    fclose(file);
    printf(RESET);
    fflush(stdout);
}

// Charge les places ET remplit la grille de collision (Murs)
void init_spots_from_map(const char *filename)
{
    // 1. Initialisation de la map logique (on la vide avec des espaces)
    for (int y = 0; y < HAUTEUR_MAX; y++)
    {
        for (int x = 0; x < LARGEUR_MAX; x++)
        {
            map_logique[y][x] = ' ';
        }
    }

    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("Erreur ouverture parking_map");
        return;
    }

    char line[1024];
    int y = 0;
    int idx_spot = 0;

    // 2. Lecture ligne par ligne pour gérer l'alignement visuel et l'UTF-8
    while (fgets(line, sizeof(line), file) && y < HAUTEUR_MAX)
    {
        int visual_x = 0; // Colonne telle qu'elle apparaît réellement à l'écran

        for (int i = 0; line[i] != '\0' && line[i] != '\n' && line[i] != '\r';)
        {
            unsigned char c = (unsigned char)line[i];
            int char_len = 1;

            // Détection de la taille du caractère UTF-8 pour l'alignement visuel
            if (c >= 0xf0)
                char_len = 4;
            else if (c >= 0xe0)
                char_len = 3;
            else if (c >= 0xc0)
                char_len = 2;

            // --- DÉTECTION DU POINT D'ENTRÉE 'D' ---
            if (line[i] == 'D')
            {
                spawn_x = visual_x;
                spawn_y = y;
                map_logique[y][visual_x] = ' '; // Zone circulable
            }
            // --- DÉTECTION DU POINT DE SORTIE 'F' ---
            else if (line[i] == 'F')
            {
                sortie_x = visual_x;
                sortie_y = y;
                map_logique[y][visual_x] = ' '; // Zone circulable
            }
            // --- DÉTECTION DES PLACES DE PARKING '@' ---
            else if (line[i] == SPOT_CHAR)
            {
                if (idx_spot < TOTAL_SPOTS)
                {
                    all_spots[idx_spot].screen_x = visual_x;
                    // On garde l'offset de -5 pour l'affichage des carrés verts au-dessus
                    all_spots[idx_spot].screen_y = y - 7;
                    all_spots[idx_spot].is_occupied = 0;
                    all_spots[idx_spot].id_voiture = -1;
                    idx_spot++;
                }
                map_logique[y][visual_x] = ' '; // Zone circulable sur la place
            }
            // --- REMPLISSAGE DE LA MAP DE COLLISION (MURS, ETC.) ---
            else
            {
                if (visual_x < LARGEUR_MAX)
                {
                    map_logique[y][visual_x] = line[i];
                }
            }

            // On avance dans les octets du fichier
            i += char_len;
            // On avance d'une seule position visuelle sur l'écran
            visual_x++;
        }
        y++;
    }

    fclose(file);
}
// --- MOTEUR PHYSIQUE ---

// Vérifie si une case est un obstacle (Mur)
int est_obstacle(int x, int y)
{
    if (x < 0 || x >= LARGEUR_MAX || y < 0 || y >= HAUTEUR_MAX)
        return 1;

    // On récupère le caractère à la position (x, y)
    unsigned char c = (unsigned char)map_logique[y][x];

    // --- LISTE BLANCHE : Là où la voiture PEUT rouler ---
    // On autorise : Espace (32), D, F, @, et les flèches ASCII classiques
    if (c == ' ' || c == 'D' || c == 'F' || c == '@' ||
        c == '^' || c == 'v' || c == '<' || c == '>')
    {
        return 0; // Chemin libre
    }

    // --- DÉTECTION DES MURS PAR EXCLUSION ---
    // Si le caractère est un symbole spécial (code ASCII > 127),
    // c'est forcément un de tes murs (║, ═, etc.). On bloque.
    if (c > 127 || c == '|' || c == '-' || c == '+')
    {
        return 1;
    }

    // Par défaut, si on ne connaît pas le caractère, on considère que c'est un mur
    return 1;
}

// Vérifie si une autre voiture bloque le passage
int est_bloque_par_voiture(int x, int y, int mon_id)
{
    Vehicule *v = liste_vehicules;
    while (v != NULL)
    {
        if (v->id != mon_id)
        {
            // hitbox simple
            if (abs(v->x - x) < 10 && abs(v->y - y) < 2)
                return 1;
        }
        v = v->suivant;
    }
    return 0;
}

void effacer_vehicule(Vehicule *v)
{
    // On efface un peu plus large (12 au lieu de 10) pour supprimer les résidus
    int l = (v->direction == 'N' || v->direction == 'S') ? 6 : 15;
    int h = (v->direction == 'N' || v->direction == 'S') ? 6 : 4;

    int x_start = v->x - (l / 2);
    int y_start = v->y - (h / 2);

    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < l; j++)
        {
            int cx = x_start + j;
            int cy = y_start + i;
            if (cx >= 0 && cy >= 0 && cy < HAUTEUR_MAX && cx < LARGEUR_MAX)
            {
                goto_xy(cx, cy);
                putchar(map_logique[cy][cx]); // On répare avec le fond
            }
        }
    }
}

void spawner_vehicule(void)
{
    // Placement pile au centre du box 'D'
    int adjusted_spawn_x = spawn_x;
    int adjusted_spawn_y = spawn_y;

    Vehicule *nouveau = malloc(sizeof(Vehicule));
    if (!nouveau)
        return;

    nouveau->id = compteur_id_vehicule++;
    nouveau->x = adjusted_spawn_x;
    nouveau->y = adjusted_spawn_y;
    nouveau->tps = (unsigned long int)time(NULL);
    nouveau->type = rand() % 3;
    nouveau->etat = ETAT_CHERCHE_PLACE;
    nouveau->direction = 'N';

    // Recherche de la première place libre
    int choix = -1;
    for (int i = 0; i < TOTAL_SPOTS; i++)
    {
        if (!all_spots[i].is_occupied)
        {
            choix = i;
            break;
        }
    }

    if (choix != -1)
    {
        // Cible = la position exacte du '@' sur la map
        nouveau->cible_x = all_spots[choix].screen_x;
        nouveau->cible_y = all_spots[choix].screen_y + 5;
        all_spots[choix].is_occupied = 1;
        all_spots[choix].id_voiture = nouveau->id;
    }
    else
    {
        nouveau->etat = ETAT_SORTIE;
        nouveau->cible_x = sortie_x;
        nouveau->cible_y = sortie_y;
    }

    nouveau->suivant = liste_vehicules;
    liste_vehicules = nouveau;
}
// Fonction pour libérer une place occupée par un véhicule spécifique
void liberer_place(int id_voiture)
{
    for (int i = 0; i < TOTAL_SPOTS; i++)
    {
        if (all_spots[i].id_voiture == id_voiture)
        {
            all_spots[i].is_occupied = 0;
            all_spots[i].id_voiture = -1;
            break; // Une fois trouvée et libérée, on peut s'arrêter
        }
    }
}

void mettre_a_jour_vehicules(void)
{
    Vehicule *v = liste_vehicules;
    Vehicule *precedent = NULL;
    int ligne_autoroute = 22; // La ligne avec les flèches '→' pour circuler

    while (v != NULL)
    {
        effacer_vehicule(v); // Nettoyage 15x6

        if (v->etat != ETAT_GARE)
        {
            int a_bouge = 0;

            // --- ÉTAPE 0 : SORTIE FORCÉE DU BOX D ---
            // Si la voiture est encore en bas, on la fait monter quoi qu'il arrive
            if (v->y > ligne_autoroute)
            {
                v->y--;
                v->direction = 'N';
                a_bouge = 1;
            }
            // --- ÉTAPE 1 : NAVIGATION HORIZONTALE SUR L'AUTOROUTE ---
            else if (v->y == ligne_autoroute && v->x != v->cible_x)
            {
                int dx = (v->x < v->cible_x) ? 1 : -1;
                // Ici on vérifie les murs pour ne pas écraser les piliers centraux
                if (!est_obstacle(v->x + dx, v->y))
                {
                    v->x += dx;
                    v->direction = (dx > 0) ? 'E' : 'O';
                    a_bouge = 1;
                }
            }
            // --- ÉTAPE 2 : DESCENTE/MONTÉE VERS LA PLACE ---
            else if (v->x == v->cible_x && v->y != v->cible_y)
            {
                int dy = (v->y < v->cible_y) ? 1 : -1;
                if (!est_obstacle(v->x, v->y + dy))
                {
                    v->y += dy;
                    v->direction = (dy > 0) ? 'S' : 'N';
                    a_bouge = 1;
                }
            }
        }

        // --- ARRIVÉE ET STATIONNEMENT ---
        if (abs(v->x - v->cible_x) <= 1 && abs(v->y - v->cible_y) <= 1 && v->etat == ETAT_CHERCHE_PLACE)
        {
            v->etat = ETAT_GARE;
            v->temps_gare = 300;
            draw_all_spots(-1);
        }

        // --- SORTIE ---
        if (v->etat == ETAT_GARE)
        {
            v->temps_gare--;
            if (v->temps_gare <= 0)
            {
                v->etat = ETAT_SORTIE;
                v->cible_x = sortie_x;
                v->cible_y = ligne_autoroute; // On retourne d'abord sur la route
                liberer_place(v->id);
                draw_all_spots(-1);
            }
        }

        // SUPPRESSION SI SORTIE F ATTEINTE
        if (v->etat == ETAT_SORTIE && v->x == sortie_x && v->y <= sortie_y + 2)
        {
            Vehicule *tmp = v;
            if (precedent == NULL)
                liste_vehicules = v->suivant;
            else
                precedent->suivant = v->suivant;
            v = v->suivant;
            free(tmp);
            continue;
        }

        afficher_vehicule(v);
        precedent = v;
        v = v->suivant;
    }
    fflush(stdout);
}

void gerer_paiement(Vehicule *v)
{
    // Calcul du temps passé (différence entre maintenant et l'entrée)
    unsigned long duree = time(NULL) - v->tps;
    float tarif = duree * 0.15; // 0.15€ par seconde de simu

    // Affichage dans le cadre "TOTAL À PAYER" (Ligne 5-7 sur ta map)
    goto_xy(20, 5);
    printf("%.2f €", tarif);
    fflush(stdout);
}
void afficher_vehicule(Vehicule *v)
{
    // Application de la couleur selon le type de véhicule
    if (v->type == 0)
        printf("\033[1;31m"); // Rouge
    else if (v->type == 1)
        printf("\033[1;36m"); // Cyan
    else
        printf("\033[1;33m"); // Jaune

    // Choix du modèle selon l'orientation (Rotation 90°)
    if (v->direction == 'O' || v->direction == 'E')
    {
        // AFFICHAGE HORIZONTAL (3 lignes de haut)
        for (int i = 0; i < 3; i++)
        {
            goto_xy(v->x - 4, v->y - 1 + i);
            printf("%s", modeles[v->type].forme[i]);
        }
    }
    else
    {
        // AFFICHAGE VERTICAL (5 lignes de haut)
        for (int i = 0; i < 5; i++)
        {
            goto_xy(v->x - 2, v->y - 2 + i);
            printf("%s", modeles[v->type].forme_v[i]);
        }
    }
    printf("\033[0m"); // Reset de la couleur
}

void draw_all_spots(int selected_index)
{
    (void)selected_index;
    for (int i = 0; i < TOTAL_SPOTS; i++)
    {
        // On ne dessine le carré vert que si la place est LIBRE
        if (!all_spots[i].is_occupied)
        {
            // On dessine un petit rectangle de 1x3 centré sur le @
            for (int dy = -1; dy <= 1; dy++)
            {
                goto_xy(all_spots[i].screen_x, all_spots[i].screen_y + dy);
                printf("%s %s", BG_GREEN, RESET);
            }
        }
    }
}

void afficher_vehicules_dynamiques(void)
{
    Vehicule *v = liste_vehicules; // On part du début de la liste chaînée

    while (v != NULL)
    {
        // On appelle la fonction d'affichage individuelle que nous avons créée
        afficher_vehicule(v);
        v = v->suivant; // On passe à la voiture suivante (NXT)
    }
}

void liberer_memoire_vehicules()
{
    Vehicule *v = liste_vehicules;
    while (v != NULL)
    {
        Vehicule *temp = v;
        v = v->suivant;
        free(temp);
    }
    liste_vehicules = NULL;
}