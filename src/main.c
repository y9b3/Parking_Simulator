#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "../include/parking.h"

char key_pressed() {
    struct termios oldterm, newterm;
    int oldfd; int c; char result = 0;
    tcgetattr(STDIN_FILENO, &oldterm);
    newterm = oldterm; newterm.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newterm);
    oldfd = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldfd | O_NONBLOCK);
    c = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldterm);
    fcntl(STDIN_FILENO, F_SETFL, oldfd);
    if (c != EOF) { ungetc(c, stdin); result = getchar(); }
    return result;
}

int afficher_menu() {
    int choix = 0;
    while (choix < 1 || choix > 3) {
        printf("\033[2J\033[H\n====================================\n");
        printf("   PARKING SIMULATOR 2026 (ESIEA)   \n====================================\n\n");
        printf("1. Mode FLUIDE\n2. Mode CHARGE\n3. Quitter\n\nVotre choix : ");
        if (scanf("%d", &choix) != 1) while (getchar() != '\n');
    }
    return choix;
}

int main() {
    srand(time(NULL));
    while (1) {
        int mode = afficher_menu();
        if (mode == 3) break;
        int chance = (mode == 1) ? 5 : 25;

        init_modeles();
        init_spots_from_map("assets/parking_map.txt");
        display_static_map("assets/parking_map.txt");

        char key = 0;
        int timer = 0;
        while (key != 'e' && key != 'r') {
            if (++timer > 10) {
                if ((rand() % 100) < chance) spawner_vehicule();
                timer = 0;
            }
            mettre_a_jour_vehicules();
            draw_all_spots(-1);
            afficher_vehicules_dynamiques();
            
            goto_xy(0, 48);
            printf("\033[KMODE: %s | 'r': Menu | 'e': Quitter", (mode == 1 ? "FLUIDE" : "CHARGE"));
            fflush(stdout);
            usleep(50000);
            key = key_pressed();
        }
        liberer_memoire_vehicules();
        if (key == 'e') break;
    }
    printf("\033[2J\033[HSimulation terminee.\n");
    return 0;
}
