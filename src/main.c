#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "../include/parking.h"

char key_pressed()
{
    struct termios oldterm, newterm;
    int oldfd;
    int c;
    char result = 0;
    tcgetattr(STDIN_FILENO, &oldterm);
    newterm = oldterm;
    newterm.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newterm);
    oldfd = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldfd | O_NONBLOCK);
    c = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldterm);
    fcntl(STDIN_FILENO, F_SETFL, oldfd);
    if (c != EOF)
    {
        ungetc(c, stdin);
        result = getchar();
    }
    return result;
}

int afficher_menu()
{
    int choix = 0;
    while (choix != 1 && choix != 2 && choix != 3)
    {
        printf("\033[2J\033[H");
        printf("\n====================================\n");
        printf("   PARKING SIMULATOR 2026 (PILOTE)  \n");
        printf("====================================\n\n");
        printf("1. Mode SOLO (Conduite Libre)\n");
        printf("2. Mode MULTI (Futur)\n");
        printf("3. Quitter\n\n");
        printf("Votre choix : ");
        if (scanf("%d", &choix) != 1)
        {
            while (getchar() != '\n')
                ;
        }
    }
    return choix;
}

int main()
{
    srand(time(NULL));
    int continuer_programme = 1;

    while (continuer_programme)
    {
        int mode = afficher_menu();
        if (mode == 3)
            break;

        init_modeles();
        liberer_memoire_vehicules();
        voiture_joueur = NULL; // On s'assure que c'est bien vide

        // Initialisation de la map et des places
        display_static_map("assets/parking_map.txt");
        init_spots_from_map("assets/parking_map.txt");
        draw_all_spots(-1);

        char key = 0;
        while (key != 'e' && key != 'r')
        {
            key = key_pressed();

            // --- GESTION DE LA TOUCHE ESPACE (Spawn et Parking) ---
            if (key == ' ')
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
                        if (voiture_joueur->etat != ETAT_GARE)
                        {
                            all_spots[id_p].is_occupied = 1;
                            all_spots[id_p].id_voiture = voiture_joueur->id;
                            voiture_joueur->etat = ETAT_GARE; // Verrouille la voiture
                        }
                        else
                        {
                            all_spots[id_p].is_occupied = 0;
                            all_spots[id_p].id_voiture = -1;
                            voiture_joueur->etat = ETAT_CHERCHE_PLACE; // Libère la voiture
                        }
                        draw_all_spots(-1);
                    }
                }
            }

            // --- GESTION DES DÉPLACEMENTS (Sécurisée) ---
            if (voiture_joueur != NULL && voiture_joueur->etat != ETAT_GARE)
            {
                if (key == 'z')
                    deplacer_joueur(0, -1, 'N');
                if (key == 's')
                    deplacer_joueur(0, 1, 'S');
                if (key == 'q')
                    deplacer_joueur(-2, 0, 'O');
                if (key == 'd')
                    deplacer_joueur(2, 0, 'E');
            }

            mettre_a_jour_vehicules();

            // --- DEBUG SÉCURISÉ : ÉVITE LE SEGMENTATION FAULT ---
            goto_xy(0, 53);
            if (voiture_joueur != NULL)
            {
                int id_p = -1;
                // On cherche la place la plus proche techniquement
                for (int i = 0; i < TOTAL_SPOTS; i++)
                {
                    if (abs(voiture_joueur->x - all_spots[i].screen_x) < 10)
                    {
                        id_p = i;
                        break;
                    }
                }

                if (id_p != -1)
                {
                    printf("\033[K[DEBUG] VOITURE(%d,%d) | PROCHE PLACE %d(%d,%d)",
                           voiture_joueur->x, voiture_joueur->y, id_p,
                           all_spots[id_p].screen_x, all_spots[id_p].screen_y);
                }
                else
                {
                    printf("\033[K[DEBUG] VOITURE(%d,%d) | AUCUNE PLACE À PROXIMITÉ",
                           voiture_joueur->x, voiture_joueur->y);
                }
            }

            // HUD Status en bas de l'écran
            goto_xy(0, 55);
            printf("\033[K MODE: %d | 'r': Menu/Reload | 'e': Quitter | ESPACE: Spawn/Park", mode);
            fflush(stdout);

            usleep(25000);
        }

        if (key == 'e')
            continuer_programme = 0;

        liberer_memoire_vehicules();
    }
    return 0;
}