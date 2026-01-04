#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "../include/parking.h"

// --- FONCTION UTILITAIRE (Non-bloquante) ---
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

// --- MENU DE DÉMARRAGE ---
int afficher_menu() {
    int choix = 0;
    while (choix != 1 && choix != 2 && choix != 3) {
        printf("\033[2J\033[H"); 
        printf("\n====================================\n");
        printf("   PARKING SIMULATOR 2026 (ESIEA)   \n");
        printf("====================================\n\n");
        printf("1. Mode FLUIDE (Peu de trafic automatique)\n");
        printf("2. Mode CHARGE (Gros trafic automatique)\n");
        printf("3. Quitter\n\n");
        printf("Votre choix : ");
        if (scanf("%d", &choix) != 1) {
            while(getchar() != '\n'); 
        }
    }
    return choix;
}

// --- MAIN (Boucle Mixte : Auto + Manuel) ---
int main() {
    srand(time(NULL));

    int mode = afficher_menu();
    if (mode == 3) return 0;

    // Configuration de la probabilité de spawn automatique
    int chance_spawn = (mode == 1) ? 5 : 25;

    // Initialisation
    init_modeles();
    const char *map_path = "assets/parking_map.txt";
    display_static_map(map_path);
    init_spots_from_map(map_path);

    int selected_spot_index = 0;
    int timer_auto = 0;
    char key = 0;

    while (key != 'e') {
        key = key_pressed();

        // --- PARTIE A : TRAFIC AUTOMATIQUE (Code 1) ---
        timer_auto++;
        if (timer_auto > 10) { 
            if ((rand() % 100) < chance_spawn) {
                spawner_vehicule();
            }
            timer_auto = 0;
        }

        // --- PARTIE B : GESTION MANUELLE ZQSD (Code 2) ---
        if (key != 0) {
            int old_selection = selected_spot_index;
            int selection_changed = 0;

            switch (key) {
                case 'q': case 'z': // Gauche ou Haut
                    if (selected_spot_index > 0) {
                        selected_spot_index--;
                        selection_changed = 1;
                    }
                    break;
                case 'd': case 's': // Droite ou Bas
                    if (selected_spot_index < TOTAL_SPOTS - 1) {
                        selected_spot_index++;
                        selection_changed = 1;
                    }
                    break;
                case ' ': // ESPACE : Changer manuellement le véhicule sur le spot
                    if (!all_spots[selected_spot_index].is_occupied) {
                        all_spots[selected_spot_index].is_occupied = 1;
                        all_spots[selected_spot_index].type_vehicule = 0;
                    } else {
                        all_spots[selected_spot_index].type_vehicule++;
                        if (all_spots[selected_spot_index].type_vehicule > 2) {
                            all_spots[selected_spot_index].is_occupied = 0;
                            all_spots[selected_spot_index].type_vehicule = 0;
                        }
                    }
                    draw_spot(all_spots[selected_spot_index], 1);
                    break;
            }

            if (selection_changed) {
                draw_spot(all_spots[old_selection], 0);
                draw_spot(all_spots[selected_spot_index], 1);
            }
        }

        // --- PARTIE C : MISE À JOUR MOTEUR & AFFICHAGE ---
        mettre_a_jour_vehicules(); // Moteur physique
        draw_all_spots(selected_spot_index); // Dessine les places (avec curseur de sélection)
        afficher_vehicules_dynamiques(); // Dessine les voitures qui roulent

        // Status bar en bas
        goto_xy(0, 38); 
        printf("\033[KMODE: %s | SPOT: %d | ZQSD: Naviguer | ESPACE: Modifier | E: Quitter", 
               (mode==1 ? "FLUIDE" : "CHARGE"), selected_spot_index);

        fflush(stdout);
        usleep(50000); 
    }

    liberer_memoire_vehicules();
    printf("\033[2J\033[HSimulation terminee.\n");
    return 0;
}
