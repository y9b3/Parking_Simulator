#define _XOPEN_SOURCE 700
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include "../include/parking.h"

#define MAX_BOTS 50

Vehicule *bots[MAX_BOTS];
int nb_bots = 0;

void nettoyer_bots()
{
    for (int i = 0; i < nb_bots; i++)
    {
        if (bots[i] != NULL)
            free(bots[i]);
        bots[i] = NULL;
    }
    nb_bots = 0;
}

// --- COLLISION STRICTE (MÊME LOGIQUE QUE LE JOUEUR) ---
int est_position_valide(int x, int y)
{
    // 1. Limites de l'écran
    if (x < 1 || x > LARGEUR_MAX - 7)
        return 0;
    if (y < 1 || y > HAUTEUR_MAX - 5)
        return 0;

    // 2. Murs (On teste les 4 coins de la voiture + milieux)
    // C'est ça qui empêche de traverser les murs !
    if (est_obstacle(x, y))
        return 0; // Coin Haut-Gauche
    if (est_obstacle(x + 5, y))
        return 0; // Coin Haut-Droit
    if (est_obstacle(x, y + 2))
        return 0; // Coin Bas-Gauche
    if (est_obstacle(x + 5, y + 2))
        return 0; // Coin Bas-Droit

    return 1;
}

// Vérifie si on ne rentre pas dans une AUTRE voiture
int est_libre_de_bots(Vehicule *me, int x, int y)
{
    for (int i = 0; i < nb_bots; i++)
    {
        if (bots[i] != NULL && bots[i] != me)
        {
            // Hitbox large pour éviter qu'elles se chevauchent
            if (abs(bots[i]->x - x) < 8 && abs(bots[i]->y - y) < 4)
            {
                return 0;
            }
        }
    }
    return 1;
}

void deplacer_bot(Vehicule *v)
{
    if (v == NULL || v->etat == ETAT_GARE)
        return;

    int cible_x = all_spots[v->id_place_visee].screen_x;
    int cible_y = all_spots[v->id_place_visee].screen_y;
    int dx = cible_x - v->x;
    int dy = cible_y - v->y;

    // --- ARRIVÉE ---
    if (abs(dx) <= 2 && abs(dy) <= 2)
    {
        effacer_vehicule(v); // On efface proprement
        v->etat = ETAT_GARE;
        all_spots[v->id_place_visee].is_occupied = 1;
        v->x = cible_x;
        v->y = cible_y;
        v->direction = 'S'; // On se gare droit
        afficher_vehicule(v);
        return;
    }

    // --- CALCUL DU MOUVEMENT ---
    int next_x = v->x;
    int next_y = v->y;
    char next_dir = v->direction;
    int a_bouge = 0;

    int step_x = (dx > 0) ? 1 : -1;
    int step_y = (dy > 0) ? 1 : -1;

    // Priorité X (Colonnes)
    if (abs(dx) > 2)
    {
        // On vérifie si la prochaine case est un mur OU une voiture
        if (est_position_valide(v->x + step_x, v->y) && est_libre_de_bots(v, v->x + step_x, v->y))
        {
            next_x = v->x + step_x;
            next_dir = (step_x > 0) ? 'E' : 'O';
            a_bouge = 1;
        }
    }

    // Si pas bougé en X, on tente Y
    if (!a_bouge && abs(dy) > 1)
    {
        if (est_position_valide(v->x, v->y + step_y) && est_libre_de_bots(v, v->x, v->y + step_y))
        {
            next_y = v->y + step_y;
            next_dir = (step_y > 0) ? 'S' : 'N';
            a_bouge = 1;
        }
    }

    // --- APPLICATION DU MOUVEMENT (AVEC GOMME) ---
    if (a_bouge)
    {
        // 1. J'EFFACE l'ancienne position (Indispensable pour la traînée)
        effacer_vehicule(v);

        // 2. Je METS A JOUR
        v->x = next_x;
        v->y = next_y;
        v->direction = next_dir;

        // 3. Je DESSINE la nouvelle position
        afficher_vehicule(v);
    }
    else
    {
        // Si je suis bloqué (embouteillage), je redessine quand même pour ne pas disparaître
        afficher_vehicule(v);
    }
}

void spawner_bot()
{
    if (nb_bots >= MAX_BOTS)
        return;

    // --- SPAWN EXACT DU MODE SOLO (Zone D) ---
    // D'après tes images, l'entrée est en bas à droite.
    // X=175, Y=48 est juste devant la barrière.
    int spawn_x = 175;
    int spawn_y = 48;

    // Si l'entrée est bouchée par une autre voiture, on n'apparait pas (sécurité)
    if (!est_libre_de_bots(NULL, spawn_x, spawn_y))
        return;

    Vehicule *v = malloc(sizeof(Vehicule));
    if (!v)
        return;

    v->x = spawn_x;
    v->y = spawn_y;
    v->type = 0;        // Type 0 = CYAN (Ta voiture standard)
    v->direction = 'N'; // Regarde vers le HAUT pour rentrer
    v->etat = ETAT_CHERCHE_PLACE;
    v->id = nb_bots + 100;
    v->heure_arrivee = time(NULL);

    // Trouver une place
    int id_place = -1;
    for (int i = 0; i < TOTAL_SPOTS; i++)
    {
        if (!all_spots[i].is_occupied && all_spots[i].id_voiture == -1)
        {
            id_place = i;
            all_spots[i].id_voiture = v->id;
            all_spots[i].is_occupied = 0;
            break;
        }
    }

    if (id_place != -1)
    {
        v->id_place_visee = id_place;
        bots[nb_bots] = v;
        nb_bots++;
        afficher_vehicule(v); // Affichage immédiat !
    }
    else
    {
        free(v); // Parking complet
    }
}

void jouer_mode_multi()
{
    clear();
    nettoyer_bots();

    init_modeles();
    display_static_map("assets/parking_map.txt");
    init_spots_from_map("assets/parking_map.txt");
    draw_all_spots(-1);
    sauvegarder_background();

    nodelay(stdscr, TRUE);
    time_t last_spawn = time(NULL);
    double intervalle = 10.0; // 10 secondes pile

    int running = 1;
    while (running)
    {
        int ch = getch();
        if (ch == 'e' || ch == 'r')
            running = 0;

        // --- SPAWN ---
        time_t now = time(NULL);
        double diff = difftime(now, last_spawn);
        if (diff >= intervalle)
        {
            spawner_bot();
            last_spawn = now;
        }

        // --- DESSIN DU DECOR ---
        // On redessine les lignes blanches pour effacer les traces noires ("carrés transparents")
        // laissées par effacer_vehicule() quand la voiture roule sur une ligne.
        draw_all_spots(-1);

        // --- GESTION DES BOTS ---
        for (int i = 0; i < nb_bots; i++)
        {
            if (bots[i] != NULL)
            {
                deplacer_bot(bots[i]);

                // Debug Prix
                if (bots[i]->etat == ETAT_GARE)
                {
                    mvprintw(bots[i]->y, bots[i]->x + 2, "$");
                }
            }
        }

        // --- HUD ---
        move(0, 0);
        attron(A_REVERSE);
        // Affiche les coords du premier bot pour debug
        if (nb_bots > 0)
            printw(" MODE AUTO | Bot 1: %d,%d | Next: %.0fs ", bots[0]->x, bots[0]->y, (intervalle - diff));
        else
            printw(" MODE AUTO | Attente Spawn... | Next: %.0fs ", (intervalle - diff));
        attroff(A_REVERSE);

        refresh();
        usleep(40000);
    }
    nettoyer_bots();
}