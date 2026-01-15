#define _XOPEN_SOURCE 700
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include "../include/parking.h"

#define MAX_BOTS 50

/* Gestion de la flotte automatique */
Vehicule *bots[MAX_BOTS];
int nb_bots = 0;

/* Nettoyage de la memoire des vehicules automatiques */
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

/* Verification des collisions avec le decor */
int est_position_valide(int x, int y)
{
    if (x < 1 || x > LARGEUR_MAX - 7)
        return 0;
    if (y < 1 || y > HAUTEUR_MAX - 5)
        return 0;

    /* Test des coins du vehicule */
    if (est_obstacle(x, y))
        return 0;
    if (est_obstacle(x + 5, y))
        return 0;
    if (est_obstacle(x, y + 2))
        return 0;
    if (est_obstacle(x + 5, y + 2))
        return 0;

    return 1;
}

/* Verification des collisions entre vehicules */
int est_libre_de_bots(Vehicule *me, int x, int y)
{
    for (int i = 0; i < nb_bots; i++)
    {
        if (bots[i] != NULL && bots[i] != me)
        {
            if (abs(bots[i]->x - x) < 8 && abs(bots[i]->y - y) < 4)
            {
                return 0;
            }
        }
    }
    return 1;
}

/* Logique de deplacement des vehicules vers leur place */
void deplacer_bot(Vehicule *v)
{
    if (v == NULL || v->etat == ETAT_GARE)
        return;

    int cible_x = all_spots[v->id_place_visee].screen_x;
    int cible_y = all_spots[v->id_place_visee].screen_y;
    int dx = cible_x - v->x;
    int dy = cible_y - v->y;

    /* Arrivee a destination et stationnement */
    if (abs(dx) <= 2 && abs(dy) <= 2)
    {
        effacer_vehicule(v);
        v->etat = ETAT_GARE;
        all_spots[v->id_place_visee].is_occupied = 1;
        v->x = cible_x;
        v->y = cible_y;
        v->direction = 'S';
        afficher_vehicule(v);
        return;
    }

    int next_x = v->x;
    int next_y = v->y;
    char next_dir = v->direction;
    int a_bouge = 0;

    int step_x = (dx > 0) ? 1 : -1;
    int step_y = (dy > 0) ? 1 : -1;

    /* Priorite au deplacement horizontal */
    if (abs(dx) > 2)
    {
        if (est_position_valide(v->x + step_x, v->y) && est_libre_de_bots(v, v->x + step_x, v->y))
        {
            next_x = v->x + step_x;
            next_dir = (step_x > 0) ? 'E' : 'O';
            a_bouge = 1;
        }
    }

    /* Deplacement vertical */
    if (!a_bouge && abs(dy) > 1)
    {
        if (est_position_valide(v->x, v->y + step_y) && est_libre_de_bots(v, v->x, v->y + step_y))
        {
            next_y = v->y + step_y;
            next_dir = (step_y > 0) ? 'S' : 'N';
            a_bouge = 1;
        }
    }

    if (a_bouge)
    {
        effacer_vehicule(v);
        v->x = next_x;
        v->y = next_y;
        v->direction = next_dir;
        afficher_vehicule(v);
    }
    else
    {
        afficher_vehicule(v);
    }
}

/* Generation d'un nouveau vehicule automatique */
void spawner_bot()
{
    if (nb_bots >= MAX_BOTS)
        return;

    int spawn_x = 175;
    int spawn_y = 48;

    if (!est_libre_de_bots(NULL, spawn_x, spawn_y))
        return;

    Vehicule *v = malloc(sizeof(Vehicule));
    if (!v)
        return;

    v->x = spawn_x;
    v->y = spawn_y;
    v->type = 0;
    v->direction = 'N';
    v->etat = ETAT_CHERCHE_PLACE;
    v->id = nb_bots + 100;
    v->heure_arrivee = time(NULL);

    /* Recherche d'une place disponible */
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
        afficher_vehicule(v);
    }
    else
    {
        free(v);
    }
}

/* Boucle principale du mode automatique */
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
    double intervalle = 10.0;

    int running = 1;
    while (running)
    {
        int ch = getch();
        if (ch == 'e' || ch == 'r')
            running = 0;

        time_t now = time(NULL);
        double diff = difftime(now, last_spawn);
        if (diff >= intervalle)
        {
            spawner_bot();
            last_spawn = now;
        }

        draw_all_spots(-1);

        for (int i = 0; i < nb_bots; i++)
        {
            if (bots[i] != NULL)
            {
                deplacer_bot(bots[i]);
            }
        }

        /* Interface utilisateur */
        move(0, 0);
        attron(A_REVERSE);
        if (nb_bots > 0)
            printw(" MODE AUTO | Vehicules: %d | Prochain spawn: %.0fs ", nb_bots, (intervalle - diff));
        else
            printw(" MODE AUTO | Attente Spawn... | Prochain: %.0fs ", (intervalle - diff));
        attroff(A_REVERSE);

        refresh();
        usleep(40000);
    }
    nettoyer_bots();
}