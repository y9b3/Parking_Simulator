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
        printf("   PARKING SIMULATOR 2026 (ESIEA)   \n");
        printf("====================================\n\n");
        printf("1. Mode FLUIDE (Peu de voitures)\n");
        printf("2. Mode CHARGE (Risque d'embouteillages)\n");
        printf("3. Quitter\n\n");
        printf("Votre choix : ");

        if (scanf("%d", &choix) != 1)
        {
            while (getchar() != '\n'); 
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

        // Réglage de la difficulté selon le mode choisi
        int chance_spawn = (mode == 1) ? 5 : 20;

        // INITIALISATION
        init_modeles();
        const char *map_path = "assets/parking_map.txt";

        // Nettoyage de la mémoire si c'est un restart ('r')
        liberer_memoire_vehicules();
        liste_vehicules = NULL;

        display_static_map(map_path);
        init_spots_from_map(map_path);

        int timer = 0;
        char key = 0;

        // --- BOUCLE DE SIMULATION ---
        while (key != 'e' && key != 'r')
        {
            timer++;
            // On spawn une voiture tous les X cycles si la place est libre
            if (timer > 35) 
            {
                if ((rand() % 100) < chance_spawn)
                {
                    spawner_vehicule();
                }
                timer = 0;
            }

            mettre_a_jour_vehicules();
            draw_all_spots(-1);
            afficher_vehicules_dynamiques();

            key = key_pressed();

            // Affichage des infos en bas d'écran (Ligne 48)
            goto_xy(0, 48);
            printf("\033[K"); 
            printf("MODE: %s | 'r': Menu/Reload | 'e': Quitter", (mode == 1 ? "FLUIDE" : "CHARGE"));

            fflush(stdout);
            usleep(50000); // 50ms par cycle pour une animation fluide
        }

        if (key == 'e')
        {
            continuer_programme = 0;
        }

        liberer_memoire_vehicules();
    }

    printf("\033[2J\033[H");
    printf("Simulation terminee. Au revoir !\n");
    return 0;
}
