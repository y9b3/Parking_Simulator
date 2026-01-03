#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "../include/parking.h"

// Fonction "Non-bloquante" pour lire le clavier (Source PDF)
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

// Menu de démarrage
int afficher_menu() {
    int choix = 0;
    while (choix != 1 && choix != 2 && choix != 3) {
        printf("\033[2J\033[H"); // Clear screen
        printf("\n====================================\n");
        printf("   PARKING SIMULATOR 2025 (ESIEA)   \n");
        printf("====================================\n\n");
        printf("1. Mode FLUIDE (Peu de voitures)\n");
        printf("2. Mode CHARGE (Embouteillages)\n");
        printf("3. Quitter\n\n");
        printf("Votre choix : ");
        if (scanf("%d", &choix) != 1) {
            while(getchar() != '\n'); // Vider buffer si erreur de saisie
        }
    }
    return choix;
}

int main() {
    srand(time(NULL)); // Initialisation de l'aléatoire

    // 1. MENU
    int mode = afficher_menu();
    if (mode == 3) return 0;

    // Config difficulté
    int chance_spawn = (mode == 1) ? 5 : 25; // % de chance d'apparition

    // 2. CHARGEMENT
    init_modeles();
    const char *map_path = "assets/parking_map.txt";
    
    // Affiche le décor et charge les collisions en mémoire
    display_static_map(map_path);
    init_spots_from_map(map_path);

    int timer = 0;
    char key = 0;

    // 3. BOUCLE DE JEU
    while (key != 'e') {
        
        // A. SPAWN (Génération de voiture)
        timer++;
        if (timer > 10) { // On teste toutes les 10 frames
            if ((rand() % 100) < chance_spawn) {
                spawner_vehicule();
            }
            timer = 0;
        }

        // B. PHYSIQUE (Mouvement et Collisions)
        mettre_a_jour_vehicules();

        // C. AFFICHAGE
        draw_all_spots(-1); // Rafraîchit les places (vert/rouge)
        afficher_vehicules_dynamiques(); // Dessine les voitures par dessus

        // D. INPUT
        key = key_pressed();
        
        // Debug info
        goto_xy(0, 36);
        printf("Mode: %s | [E] Quitter ", (mode==1?"Fluide":"Charge"));

        fflush(stdout);
        usleep(50000); // Vitesse du jeu (50ms = 20 FPS)
    }

    // Nettoyage propre
    liberer_memoire_vehicules();
    
    goto_xy(0, 38);
    printf("Simulation terminee.\n");
    return 0;
}