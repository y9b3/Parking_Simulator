#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "../include/parking.h"

// --- FONCTION UTILITAIRE (Non-bloquante) ---
// Permet de lire une touche sans arrêter le programme
// Source: PDF du projet
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
        // Efface l'écran
        printf("\033[2J\033[H"); 
        printf("\n====================================\n");
        printf("   PARKING SIMULATOR 2025 (ESIEA)   \n");
        printf("====================================\n\n");
        printf("1. Mode FLUIDE (Peu de voitures)\n");
        printf("2. Mode CHARGE (Risque d'embouteillages)\n");
        printf("3. Quitter\n\n");
        printf("Votre choix : ");
        
        // Sécurité de saisie
        if (scanf("%d", &choix) != 1) {
            while(getchar() != '\n'); // Vider le buffer si l'utilisateur tape une lettre
        }
    }
    return choix;
}

// --- MAIN (Boucle du Jeu) ---
int main() {
    // 1. Initialisation de l'aléatoire (Important pour que ça change à chaque fois)
    srand(time(NULL));

    // 2. AFFICHER LE MENU
    int mode = afficher_menu();
    if (mode == 3) {
        printf("Fermeture du programme.\n");
        return 0;
    }

    // Configuration de la difficulté selon le mode
    // Mode 1: 5% de chance de spawn par cycle
    // Mode 2: 25% de chance de spawn par cycle
    int chance_spawn = (mode == 1) ? 5 : 25;

    // 3. CHARGEMENT DES ASSETS
    init_modeles();
    const char *map_path = "assets/parking_map.txt";
    
    // Affiche le décor (fichier texte) et charge la mémoire (collisions)
    display_static_map(map_path);
    init_spots_from_map(map_path);

    int timer = 0;
    char key = 0;

    // --- BOUCLE PRINCIPALE (Game Loop) ---
    while (key != 'e') {
        
        // A. GESTION DU SPAWN (Trafic)
        timer++;
        // On ne tente de faire apparaitre une voiture que tous les 10 cycles (pour pas saturer)
        if (timer > 10) { 
            // Tirage au sort selon la difficulté
            if ((rand() % 100) < chance_spawn) {
                spawner_vehicule();
            }
            timer = 0; // Reset du timer
        }

        // B. MOTEUR PHYSIQUE
        // Calcul des mouvements, collisions, et gestion des départs
        mettre_a_jour_vehicules();

        // C. AFFICHAGE
        // 1. On rafraichit les indicateurs de places (Vert/Rouge)
        draw_all_spots(-1); 
        // 2. On dessine les voitures par dessus
        afficher_vehicules_dynamiques();

        // D. ENTRÉES CLAVIER
        key = key_pressed();
        
        // Petit affichage de statut en bas
        goto_xy(0, 38); // Ajuste le Y si ton texte est trop haut/bas
        printf("\033[K"); // Efface la ligne
        printf("MODE: %s | Appuyez sur 'e' pour Quitter", (mode==1 ? "FLUIDE" : "CHARGE"));

        // E. SYNC (Frame Rate)
        fflush(stdout); // Force l'affichage immédiat
        usleep(50000);  // Pause de 50ms (~20 images par seconde)
    }

    // 4. NETTOYAGE
    // On libère la mémoire (malloc) avant de quitter, c'est propre.
    liberer_memoire_vehicules();
    
    // Message de fin
    printf("\033[2J\033[H"); // Clear screen final
    printf("Simulation terminee. Au revoir !\n");
    return 0;
}