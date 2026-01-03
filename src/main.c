#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "../include/parking.h"

// Fonction utilitaire pour lire le clavier sans bloquer
char key_pressed() {
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
    if (c != EOF) {
        ungetc(c, stdin);
        result = getchar();
    }
    return result;
}

int afficher_menu() {
    int choix = 0;
    while (choix != 1 && choix != 2 && choix != 3) {
        printf("\033[2J\033[H");
        printf("\n====================================\n");
        printf("   PARKING SIMULATOR 2025 (ESIEA)   \n");
        printf("====================================\n\n");
        printf("1. Mode FLUIDE (Peu de voitures)\n");
        printf("2. Mode CHARGE (Beaucoup de voitures)\n");
        printf("3. Quitter\n\n");
        printf("Votre choix : ");
        if (scanf("%d", &choix) != 1) {
            while(getchar() != '\n');
        }
    }
    return choix;
}

int main() {
    srand(time(NULL));

    int mode = afficher_menu();
    if (mode == 3) return 0;

    int chance_spawn = (mode == 1) ? 5 : 20; // % de chance

    init_modeles();
    const char *map_path = "assets/parking_map.txt";
    
    // 1. Charger la map
    display_static_map(map_path);
    init_spots_from_map(map_path);

    int timer = 0;
    char key = 0;

    // --- BOUCLE DE JEU ---
    while (key != 'e') {
        
        // A. SPAWN
        timer++;
        if (timer > 10) {
            if ((rand() % 100) < chance_spawn) {
                spawner_vehicule();
            }
            timer = 0;
        }

        // B. UPDATE (Mouvement + Effacement des traces)
        mettre_a_jour_vehicules();

        // C. DRAW
        draw_all_spots(-1); // On redessine les places
        afficher_vehicules_dynamiques(); // On dessine les voitures

        // D. INPUT
        key = key_pressed();
        
        // Info debug
        goto_xy(0, 35);
        printf("Mode: %s | 'e' pour Quitter | Vehicules actifs", (mode==1?"Fluide":"Charge"));

        fflush(stdout);
        usleep(50000); // 50ms pause
    }

    liberer_memoire_vehicules();
    goto_xy(0, 37);
    printf("Fin de la simulation.\n");
    return 0;
}