#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "../include/parking.h"

// Fonction pour capturer une touche sans bloquer (du PDF)
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
        printf("\033[2J\033[H"); // Clear screen
        printf("\n====================================\n");
        printf("   PARKING SIMULATOR 2025 (ESIEA)   \n");
        printf("====================================\n\n");
        printf("1. Mode FLUIDE (Peu de voitures)\n");
        printf("2. Mode CHARGE (Beaucoup de voitures)\n");
        printf("3. Quitter\n\n");
        printf("Votre choix : ");
        if (scanf("%d", &choix) != 1) {
            while(getchar() != '\n'); // Vider buffer si erreur
        }
    }
    return choix;
}

int main() {
    srand(time(NULL)); // Init aléatoire

    // 1. MENU
    int mode = afficher_menu();
    if (mode == 3) return 0;

    // Configuration selon le mode
    int chance_spawn = (mode == 1) ? 5 : 25; // % de chance par cycle
    
    // 2. INIT
    init_modeles();
    const char *map_path = "assets/parking_map.txt";
    
    // Chargement graphique
    display_static_map(map_path);
    init_spots_from_map(map_path); // Important pour connaître les places

    int timer = 0;
    char key = 0;

    // 3. BOUCLE DE JEU
    while (key != 'e') {
        
        // A. SPAWN AUTOMATIQUE
        timer++;
        if (timer > 10) { // On tente un spawn tous les 10 cycles
            if ((rand() % 100) < chance_spawn) {
                spawner_vehicule();
            }
            timer = 0;
        }

        // B. INTELLIGENCE ARTIFICIELLE
        mettre_a_jour_vehicules();

        // C. AFFICHAGE
        // On redessine les places (pour effacer les vieilles traces)
        draw_all_spots(-1); 
        // On dessine les voitures par dessus
        afficher_vehicules_dynamiques();

        // D. INPUT
        key = key_pressed();
        
        // Petit message de debug en bas
        goto_xy(0, 35);
        printf("Mode: %s | 'e' pour Quitter", (mode==1?"Fluide":"Charge"));

        fflush(stdout);
        usleep(50000); // 50ms (environ 20 FPS)
    }

    // Nettoyage avant de partir (Bonne pratique C)
    liberer_memoire_vehicules();
    
    goto_xy(0, 37);
    printf("Fin de la simulation.\n");
    return 0;
}