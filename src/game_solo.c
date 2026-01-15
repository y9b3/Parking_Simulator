#define _XOPEN_SOURCE 700
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include "../include/parking.h"

/* Liste chainee locale pour le mode solo */
static Vehicule *flotte_solo = NULL;

/* Sauvegarde de la position de depart */
static int spawn_ref_x = 0;
static int spawn_ref_y = 0;

/* Ajout d'un nouveau vehicule a la flotte */
void ajouter_voiture_manuel()
{
    int sx = spawn_ref_x;
    int sy = spawn_ref_y;

    /* Verification de la disponibilite de la zone de spawn */
    Vehicule *v = flotte_solo;
    while (v)
    {
        if (abs(v->x - sx) < 8 && abs(v->y - sy) < 5)
        {
            attron(COLOR_PAIR(3) | A_BOLD);
            mvprintw(52, 0, "!!! ZONE D'ENTREE OCCUPEE !!!");
            attroff(COLOR_PAIR(3) | A_BOLD);
            return;
        }
        v = v->suivant;
    }

    /* Allocation et initialisation du nouveau vehicule */
    Vehicule *new_v = malloc(sizeof(Vehicule));
    if (!new_v)
        return;

    new_v->x = sx;
    new_v->y = sy;
    new_v->direction = 'O';
    new_v->type = 0;
    new_v->etat = ETAT_CHERCHE_PLACE;
    new_v->heure_arrivee = time(NULL);
    new_v->suivant = NULL;

    /* Attribution d'un identifiant unique */
    int max = 0;
    Vehicule *t = flotte_solo;
    while (t)
    {
        if (t->id > max)
            max = t->id;
        t = t->suivant;
    }
    new_v->id = max + 1;

    /* Insertion en fin de liste chaînée */
    if (!flotte_solo)
        flotte_solo = new_v;
    else
    {
        Vehicule *last = flotte_solo;
        while (last->suivant)
            last = last->suivant;
        last->suivant = new_v;
    }

    voiture_joueur = new_v;
}

/* Changement de vehicule controle (Tabulation) */
void changer_focus()
{
    if (!voiture_joueur || !flotte_solo)
        return;

    if (voiture_joueur->suivant)
        voiture_joueur = voiture_joueur->suivant;
    else
        voiture_joueur = flotte_solo;
}

/* Fonction principale du mode solo */
void jouer_mode_solo()
{
    clear();
    init_modeles();
    liberer_memoire_vehicules();
    flotte_solo = NULL;
    voiture_joueur = NULL;

    /* Chargement du decor et des emplacements */
    display_static_map("assets/parking_map.txt");
    init_spots_from_map("assets/parking_map.txt");
    draw_all_spots(-1);
    sauvegarder_background();

    /* Premier spawn et initialisation des references */
    spawner_vehicule();

    if (voiture_joueur != NULL)
    {
        spawn_ref_x = voiture_joueur->x;
        spawn_ref_y = voiture_joueur->y;
        voiture_joueur->heure_arrivee = time(NULL);
        voiture_joueur->id = 1;
        voiture_joueur->suivant = NULL;
        voiture_joueur->direction = 'O';
        flotte_solo = voiture_joueur;
    }

    nodelay(stdscr, TRUE);
    int ch = 0;
    int running = 1;

    /* Boucle de jeu principale */
    while (running)
    {
        ch = getch();
        if (ch == 'e')
            running = 0;

        /* Gestion des commandes utilisateur */
        if (ch == 'a' || ch == 'A')
        {
            draw_all_spots(-1);
            ajouter_voiture_manuel();
        }
        if (ch == 9)
            changer_focus();

        if (voiture_joueur != NULL)
        {
            /* Interaction avec les places de parking */
            if (ch == 'g' || ch == ' ')
            {
                int id_p = verifier_place_proche(voiture_joueur);
                if (id_p != -1)
                {
                    if (voiture_joueur->etat == ETAT_CHERCHE_PLACE)
                    {
                        all_spots[id_p].is_occupied = 1;
                        all_spots[id_p].id_voiture = voiture_joueur->id;
                        voiture_joueur->etat = ETAT_GARE;
                    }
                    else if (voiture_joueur->etat == ETAT_GARE)
                    {
                        if (all_spots[id_p].id_voiture == voiture_joueur->id)
                        {
                            all_spots[id_p].is_occupied = 0;
                            all_spots[id_p].id_voiture = -1;
                            voiture_joueur->etat = ETAT_CHERCHE_PLACE;
                        }
                    }
                    draw_all_spots(-1);
                }
            }

            /* Gestion du ticket et de la facturation */
            if (ch == 'f' || ch == 'F')
            {
                if (voiture_joueur->x < 30 && voiture_joueur->y < 15)
                {
                    time_t now = time(NULL);
                    double sec = difftime(now, voiture_joueur->heure_arrivee);
                    if (sec < 0 || sec > 100000)
                        sec = 0;
                    double prix = sec * 0.50;

                    /* Affichage du ticket de sortie */
                    nodelay(stdscr, FALSE);
                    attron(COLOR_PAIR(3) | A_BOLD);
                    int bx = 50, by = 2;
                    mvprintw(by, bx, "#########################################");
                    mvprintw(by + 1, bx, "#          TICKET DE SORTIE             #");
                    mvprintw(by + 2, bx, "#  VOITURE %d                            #", voiture_joueur->id);
                    mvprintw(by + 3, bx, "#  A PAYER : %-6.2f EUR                 #", prix);
                    mvprintw(by + 4, bx, "#########################################");
                    attroff(COLOR_PAIR(3) | A_BOLD);
                    refresh();
                    while (getch() != 10)
                        ;

                    /* Sortie et reinitialisation du vehicule au spawn */
                    effacer_vehicule(voiture_joueur);
                    voiture_joueur->x = spawn_ref_x;
                    voiture_joueur->y = spawn_ref_y;
                    voiture_joueur->direction = 'O';
                    voiture_joueur->etat = ETAT_CHERCHE_PLACE;
                    voiture_joueur->heure_arrivee = time(NULL);

                    clear();
                    display_static_map("assets/parking_map.txt");
                    draw_all_spots(-1);
                    nodelay(stdscr, TRUE);
                }
            }

            /* Deplacements du vehicule actif */
            if (voiture_joueur->etat != ETAT_GARE)
            {
                if (ch == 'z' || ch == KEY_UP)
                    deplacer_joueur(0, -1, 'N');
                if (ch == 's' || ch == KEY_DOWN)
                    deplacer_joueur(0, 1, 'S');
                if (ch == 'q' || ch == KEY_LEFT)
                    deplacer_joueur(-2, 0, 'O');
                if (ch == 'd' || ch == KEY_RIGHT)
                    deplacer_joueur(2, 0, 'E');
            }
        }

        mettre_a_jour_vehicules();

        /* Affichage de l'ensemble de la flotte */
        Vehicule *p = flotte_solo;
        while (p)
        {
            afficher_vehicule(p);
            p = p->suivant;
        }

        /* Interface utilisateur (HUD) */
        move(53, 0);
        clrtoeol();
        if (voiture_joueur)
        {
            time_t now = time(NULL);
            double t = difftime(now, voiture_joueur->heure_arrivee);
            if (t < 0 || t > 100000)
                t = 0;
            attron(COLOR_PAIR(2));
            mvprintw(53, 2, "VOITURE %d (ACTIVE) | Temps: %.0fs | Prix: %.2f E", voiture_joueur->id, t, t * 0.50);
            attroff(COLOR_PAIR(2));
        }

        move(55, 0);
        clrtoeol();
        attron(A_REVERSE);
        printw(" COMMANDES | 'A': Ajouter | 'TAB': Changer | 'F': Payer | 'G': Garer ");
        attroff(A_REVERSE);

        refresh();
        usleep(30000);
    }

    /* Nettoyage des pointeurs avant sortie */
    flotte_solo = NULL;
    voiture_joueur = NULL;
}