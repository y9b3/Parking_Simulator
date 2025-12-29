#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#include "../include/parking.h"
#include "../include/vehicule.h"
#include "../include/chargementvehicule.h"

// Fonction pour lire une touche sans bloquer l'affichage
char key_pressed() {
    struct termios oldterm, newterm;
    int oldfd;
    char c, result = 0;

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

// Mode fluide : peu de véhicules
void lancer_mode_fluide() {
    printf("\n--- Mode Fluide ---\n");

    const char *map_path = "assets/parking_map.txt";
    display_static_map(map_path);
    init_spots();
    draw_all_spots(0);

    VEHICULE *v1 = creer_vehicule('N', 5, 5, 60, 'g', 'v', "Voiture1", 1);
    ajouter_vehicule(v1);

    // Boucle de simulation simple
    for (int step = 0; step < 10; step++) {
        printf("\nÉtape %d\n", step+1);
        deplacer_tous_vehicules();
        afficher_vehicules();
        usleep(500000);
    }
}

// Mode chargé : beaucoup de véhicules
void lancer_mode_charge() {
    printf("\n--- Mode Chargé ---\n");

    const char *map_path = "assets/parking_map.txt";
    display_static_map(map_path);
    init_spots();
    draw_all_spots(0);

    for (int i = 0; i < 5; i++) {
        VEHICULE *v = creer_vehicule('S', rand()%20, rand()%20,
                                     40 + rand()%30, 'd', 'c', "Camion", i+2);
        ajouter_vehicule(v);
    }

     for (int step = 0; step < 10; step++) {
    deplacer_tous_vehicules();
    afficher_vehicules_sur_map(); // ← affichage visuel
    usleep(500000);
    }

}



int main() {
    srand(time(NULL));

    printf("=== SIMULATEUR DE PARKING ===\n");
    printf("Choisissez un mode :\n");
    printf("1. Fluide\n");
    printf("2. Chargé\n");
    printf("e. Quitter\n");

    char choix = 0;
    while (choix != 'e') {
        choix = key_pressed();
        if (choix != 0) {
            switch (choix) {
                case '1': lancer_mode_fluide(); break;
                case '2': lancer_mode_charge(); break;
                case 'e': printf("\nFin de la simulation.\n"); break;
            }
        }
        usleep(50000);
    }

    return 0;
}
