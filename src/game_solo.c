#define _XOPEN_SOURCE 700
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include "../include/parking.h"

void jouer_mode_solo()
{
    // 1. Initialisation
    clear();
    init_modeles();
    liberer_memoire_vehicules();
    voiture_joueur = NULL;

    display_static_map("assets/parking_map.txt");
    init_spots_from_map("assets/parking_map.txt");
    draw_all_spots(-1);

    sauvegarder_background();

    nodelay(stdscr, TRUE);

    int ch = 0;
    int running = 1;

    // 2. Boucle de jeu
    while (running)
    {
        ch = getch();

        if (ch == 'e')
            running = 0;
        if (ch == 'r')
            running = 0;

        // --- GESTION PARKING (Touche 'G' ou Espace) ---
        if (ch == 'g' || ch == ' ')
        {
            if (voiture_joueur == NULL)
            {
                spawner_vehicule();
            }
            else
            {
                int id_p = verifier_place_proche(voiture_joueur);
                if (id_p != -1)
                {
                    if (voiture_joueur->etat == ETAT_CHERCHE_PLACE)
                    {
                        // === SE GARER ===
                        all_spots[id_p].is_occupied = 1;
                        all_spots[id_p].id_voiture = voiture_joueur->id;
                        voiture_joueur->etat = ETAT_GARE;
                        // Pas de téléportation (comme convenu)
                    }
                    else if (voiture_joueur->etat == ETAT_GARE)
                    {
                        // === REPARTIR ===
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
        }

        // --- DEPLACEMENTS ---
        if (voiture_joueur != NULL && voiture_joueur->etat != ETAT_GARE)
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

        mettre_a_jour_vehicules();

        // --- DEBUG & HUD ---
        move(53, 0);
        clrtoeol();
        if (voiture_joueur != NULL)
        {
            int id_p = verifier_place_proche(voiture_joueur);
            if (id_p != -1)
            {
                mvprintw(53, 0, "[DEBUG] VOITURE(%d,%d) | SUR PLACE %d (Lock)",
                         voiture_joueur->x, voiture_joueur->y, id_p);
            }
            else
            {
                mvprintw(53, 0, "[DEBUG] VOITURE(%d,%d)",
                         voiture_joueur->x, voiture_joueur->y);
            }
        }

        move(54, 0);
        clrtoeol();
        if (voiture_joueur != NULL)
        {
            if (voiture_joueur->etat == ETAT_GARE)
            {
                attron(COLOR_PAIR(1) | A_BOLD);
                mvprintw(54, 2, ">>> VOITURE GAREE ! APPUYEZ SUR 'G' POUR REPARTIR <<<");
                attroff(COLOR_PAIR(1) | A_BOLD);
            }
            else if (verifier_place_proche(voiture_joueur) != -1)
            {
                attron(COLOR_PAIR(2) | A_BLINK);
                mvprintw(54, 2, ">>> PLACE DETECTEE ! APPUYEZ SUR 'G' POUR VOUS GARER <<<");
                attroff(COLOR_PAIR(2) | A_BLINK);
            }
        }

        move(55, 0);
        clrtoeol();
        attron(A_REVERSE);
        mvprintw(55, 0, " SOLO MODE | 'r': Retour Menu | 'G': Action | Fleches: Bouger ");
        attroff(A_REVERSE);

        refresh();
        usleep(30000);
    }

    liberer_memoire_vehicules();
}