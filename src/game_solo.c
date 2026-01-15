#define _XOPEN_SOURCE 700
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include "../include/parking.h"

// Liste locale
static Vehicule *flotte_solo = NULL;

// VARIABLES POUR SAUVEGARDER LE SPAWN PARFAIT
static int spawn_ref_x = 0;
static int spawn_ref_y = 0;

// --- AJOUT MANUEL (Touche A) ---
void ajouter_voiture_manuel()
{
    // ON UTILISE LES COORDONNEES ENREGISTREES DU PREMIER VEHICULE
    int sx = spawn_ref_x;
    int sy = spawn_ref_y;

    // Vérif si l'entrée est libre
    Vehicule *v = flotte_solo;
    while (v)
    {
        // Zone de sécurité
        if (abs(v->x - sx) < 8 && abs(v->y - sy) < 5)
        {
            attron(COLOR_PAIR(3) | A_BOLD);
            mvprintw(52, 0, "!!! ENTREE OCCUPEE !!! BOUGEZ LA VOITURE D'ABORD");
            attroff(COLOR_PAIR(3) | A_BOLD);
            return;
        }
        v = v->suivant;
    }

    // Création
    Vehicule *new_v = malloc(sizeof(Vehicule));
    if (!new_v)
        return;

    new_v->x = sx;
    new_v->y = sy;
    new_v->direction = 'O'; // OUEST (Gauche), comme la voiture 1
    new_v->type = 0;
    new_v->etat = ETAT_CHERCHE_PLACE;
    new_v->heure_arrivee = time(NULL);
    new_v->suivant = NULL;

    // ID
    int max = 0;
    Vehicule *t = flotte_solo;
    while (t)
    {
        if (t->id > max)
            max = t->id;
        t = t->suivant;
    }
    new_v->id = max + 1;

    // Ajout liste
    if (!flotte_solo)
        flotte_solo = new_v;
    else
    {
        Vehicule *last = flotte_solo;
        while (last->suivant)
            last = last->suivant;
        last->suivant = new_v;
    }

    // Focus
    voiture_joueur = new_v;
}

// --- TABULATION ---
void changer_focus()
{
    if (!voiture_joueur || !flotte_solo)
        return;
    if (voiture_joueur->suivant)
        voiture_joueur = voiture_joueur->suivant;
    else
        voiture_joueur = flotte_solo;
}

void jouer_mode_solo()
{
    clear();
    init_modeles();
    liberer_memoire_vehicules();
    flotte_solo = NULL;
    voiture_joueur = NULL;

    display_static_map("assets/parking_map.txt");
    init_spots_from_map("assets/parking_map.txt");
    draw_all_spots(-1);
    sauvegarder_background();

    // 1. SPAWN ORIGINAL
    spawner_vehicule();

    if (voiture_joueur != NULL)
    {
        // === ICI LA MAGIE : ON COPIE LES COORDONNEES PARFAITES ===
        spawn_ref_x = voiture_joueur->x;
        spawn_ref_y = voiture_joueur->y;

        voiture_joueur->heure_arrivee = time(NULL);
        voiture_joueur->id = 1;
        voiture_joueur->suivant = NULL;

        // On force la direction vers la gauche (Ouest) si ce n'est pas le cas
        voiture_joueur->direction = 'O';

        flotte_solo = voiture_joueur;
    }

    nodelay(stdscr, TRUE);
    int ch = 0;
    int running = 1;

    while (running)
    {
        ch = getch();
        if (ch == 'e')
            running = 0;

        // --- COMMANDES ---
        if (ch == 'a' || ch == 'A')
        {
            draw_all_spots(-1);
            ajouter_voiture_manuel();
        }
        if (ch == 9)
            changer_focus(); // TAB

        // --- LOGIQUE ---
        if (voiture_joueur != NULL)
        {
            // GARER
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

            // TICKET (F)
            if (ch == 'f' || ch == 'F')
            {
                if (voiture_joueur->x < 30 && voiture_joueur->y < 15)
                {
                    time_t now = time(NULL);
                    double sec = difftime(now, voiture_joueur->heure_arrivee);
                    if (sec < 0 || sec > 100000)
                        sec = 0;
                    double prix = sec * 0.50;

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

                    // Respawn au point de référence
                    effacer_vehicule(voiture_joueur);

                    // ON UTILISE LES COORDONNEES COPIEES
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

            // DEPLACEMENTS (ORIGINAUX)
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

        // AFFICHAGE
        Vehicule *p = flotte_solo;
        while (p)
        {
            afficher_vehicule(p);
            p = p->suivant;
        }

        // HUD
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
        printw(" COMMANDES | 'A': Ajouter Voiture | 'TAB': Changer | 'F': Payer | 'G': Garer ");
        attroff(A_REVERSE);

        refresh();
        usleep(30000);
    }

    flotte_solo = NULL;
    voiture_joueur = NULL;
}