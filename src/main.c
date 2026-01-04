#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "../include/parking.h"

// --- FONCTION UTILITAIRE (Non-bloquante) ---
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

// --- MENU DE DÉMARRAGE ---
int afficher_menu()
{
    int choix = 0;
    while (choix != 1 && choix != 2 && choix != 3)
    {
        printf("\033[2J\033[H");
        printf("\n====================================\n");
        printf("   PARKING SIMULATOR 2025 (ESIEA)   \n");
        printf("====================================\n\n");
        printf("1. Mode TEST (1 voiture à la fois)\n");
        printf("2. Mode CHARGE (Risque d'embouteillages)\n");
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

// --- MAIN (Boucle du Jeu) ---
int main()
{
    srand(time(NULL));
    int continuer_programme = 1;

    while (continuer_programme)
    {
        int mode = afficher_menu();
        if (mode == 3)
        {
            continuer_programme = 0;
            break;
        }

        int chance_spawn = (mode == 1) ? 5 : 25;

        init_modeles();
        const char *map_path = "assets/parking_map.txt";

        liberer_memoire_vehicules();
        liste_vehicules = NULL;

        // --- AFFICHAGE STATIQUE (Une seule fois !) ---
        display_static_map(map_path);
        init_spots_from_map(map_path);
        draw_all_spots(-1); // On dessine les places une seule fois au début

        int timer = 0;
        char key = 0;

        while (key != 'e' && key != 'r')
        {
            timer++;
            if (timer > 10)
            {
                if (mode == 1)
                {
                    if (liste_vehicules == NULL)
                    {
                        spawner_vehicule();
                    }
                }
                else
                {
                    if ((rand() % 100) < chance_spawn)
                    {
                        spawner_vehicule();
                    }
                }
                timer = 0;
            }

            // --- ORDRE DE DESSIN CRUCIAL ---
            // 1. On met à jour (qui contient effacer_vehicule)
            mettre_a_jour_vehicules();

            // 2. On n'appelle PLUS draw_all_spots(-1) ici !
            // On ne redessine que si une place change de couleur (géré dans mettre_a_jour)

            // 3. On affiche les voitures
            // Note: Si mettre_a_jour appelle déjà afficher_vehicule, tu peux commenter cette ligne
            // afficher_vehicules_dynamiques();

            key = key_pressed();

            // --- TEXTE DE STATUT ---
            goto_xy(0, 48);
            printf("\033[K");
            printf("MODE: %s | 'r': Menu/Reload | 'e': Quitter", (mode == 1 ? "TEST (Unique)" : "CHARGE"));

            fflush(stdout);
            usleep(50000); // Pause de 50ms pour la fluidité
        }

        if (key == 'e')
            continuer_programme = 0;
        liberer_memoire_vehicules();
    }

    printf("\033[2J\033[H");
    printf("Simulation terminee. Au revoir !\n");
    return 0;
}