#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <math.h> // Pour abs()
#include "../include/parking.h"

// Couleurs et Codes ANSI
#define RESET "\033[0m"
#define CLEAR_SCREEN "\033[2J"
#define CURSOR_HOME "\033[H"
#define RED_TEXT "\033[91m"
#define BG_GREEN "\033[42m"
#define BG_RED "\033[41m"

#define SPOT_CHAR '@' 

// --- GLOBALES ---
ParkingSpot all_spots[TOTAL_SPOTS];
Vehicule *liste_vehicules = NULL; // Tête de liste chaînée
ModeleVehicule modeles[3];
int compteur_id_vehicule = 0;

// Init des dessins ASCII
void init_modeles(void) {
    modeles[0].id = 0; modeles[0].largeur = 9;
    modeles[0].forme[0] = "┌═╦═════╗"; modeles[0].forme[1] = "║ ║▆   ║"; modeles[0].forme[2] = "└═╩═════╝";

    modeles[1].id = 1; modeles[1].largeur = 12;
    modeles[1].forme[0] = "╔════════╦═┐"; modeles[1].forme[1] = "║      ▅║ │"; modeles[1].forme[2] = "╚════════╩═┘";

    modeles[2].id = 2; modeles[2].largeur = 10;
    modeles[2].forme[0] = "┌──┬───┬─╗"; modeles[2].forme[1] = "│  ║ ║ ║ │"; modeles[2].forme[2] = "└──┴───┴─╝";
}

void goto_xy(int x, int y) {
    printf("\033[%d;%dH", y + 1, x + 1);
}

void display_static_map(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return;
    printf(CLEAR_SCREEN); printf(CURSOR_HOME);
    int c;
    while ((c = fgetc(file)) != EOF) putchar(c);
    fclose(file);
    printf(RESET); fflush(stdout);
}

void init_spots_from_map(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return;
    int x = 0, y = 0, idx = 0;
    int byte;
    while ((byte = fgetc(file)) != EOF) {
        if (byte == '\r') continue; // Fix Windows
        if (byte == '\n') { y++; x = 0; continue; }
        
        // Si c'est une place de parking (@)
        if (byte == SPOT_CHAR) {
            if (idx < TOTAL_SPOTS) {
                all_spots[idx].screen_x = x;
                all_spots[idx].screen_y = y;
                all_spots[idx].is_occupied = 0;
                all_spots[idx].id_voiture = -1;
                idx++;
            }
        }
        x++; // On avance même pour les caractères non-UTF8 simples
    }
    fclose(file);
}

// --- FONCTIONS LOGIQUES (C'EST ICI QUE CA SE JOUE) ---

void spawner_vehicule(void) {
    // 1. Allocation dynamique (MALLOC)
    Vehicule *nouveau = malloc(sizeof(Vehicule));
    if (nouveau == NULL) return;

    // 2. Initialisation
    nouveau->id = compteur_id_vehicule++;
    // COORDONNEES DE L'ENTREE (A AJUSTER SELON TA MAP)
    nouveau->x = 90; 
    nouveau->y = 25; 
    nouveau->type = rand() % 3;
    nouveau->etat = ETAT_CHERCHE_PLACE;
    nouveau->temps_gare = 0;
    
    // 3. Trouver une place cible libre
    int place_trouvee = -1;
    for (int i = 0; i < TOTAL_SPOTS; i++) {
        if (!all_spots[i].is_occupied) {
            place_trouvee = i;
            break; 
        }
    }

    if (place_trouvee != -1) {
        nouveau->cible_x = all_spots[place_trouvee].screen_x;
        nouveau->cible_y = all_spots[place_trouvee].screen_y;
        // On réserve la place tout de suite pour pas que 2 voitures y aillent
        all_spots[place_trouvee].is_occupied = 1;
        all_spots[place_trouvee].id_voiture = nouveau->id;
    } else {
        // Parking complet : elle va direct à la sortie
        nouveau->etat = ETAT_SORTIE;
        nouveau->cible_x = 90; // Sortie X
        nouveau->cible_y = 5;  // Sortie Y
    }

    // 4. Ajout en TÊTE de liste chaînée
    nouveau->suivant = liste_vehicules;
    liste_vehicules = nouveau;
}

void mettre_a_jour_vehicules(void) {
    Vehicule *v = liste_vehicules;
    Vehicule *precedent = NULL;

    while (v != NULL) {
        
        // --- 1. GESTION DU MOUVEMENT (Seulement si PAS garé) ---
        if (v->etat != ETAT_GARE) {
            
            // Mouvement en X
            if (v->x < v->cible_x) v->x++;
            else if (v->x > v->cible_x) v->x--;
            
            // Mouvement en Y (On essaie de prioriser les allées)
            // On ne bouge en Y que si on est aligné en X ou si on est loin
            if (v->y < v->cible_y) v->y++;
            else if (v->y > v->cible_y) v->y--;
        }

        // --- 2. VERIFICATION ARRIVÉE (Hitbox plus large : 3 cases) ---
        
        // Si on cherche une place et qu'on est tout près
        if (v->etat == ETAT_CHERCHE_PLACE && 
            abs(v->x - v->cible_x) <= 2 && abs(v->y - v->cible_y) <= 2) {
            
            // HOP ! On force la position exacte (Snap) pour éviter qu'elle tremble
            v->x = v->cible_x;
            v->y = v->cible_y;
            
            // Changement d'état
            v->etat = ETAT_GARE;
            v->temps_gare = 50 + (rand() % 100); // Reste garée entre 2 et 5 secondes
        }

        // --- 3. GESTION DU TEMPS DE STATIONNEMENT ---
        else if (v->etat == ETAT_GARE) {
            v->temps_gare--;
            
           
            if (v->temps_gare <= 0) {
                v->etat = ETAT_SORTIE;
                
                
                v->cible_x = 90; 
                v->cible_y = 5;  
                
                // IMPORTANT : On libère la place dans le tableau pour qu'une autre voiture puisse venir
                for (int i=0; i<TOTAL_SPOTS; i++) {
                    if (all_spots[i].id_voiture == v->id) {
                        all_spots[i].is_occupied = 0;
                        all_spots[i].id_voiture = -1;
                    }
                }
            }
        }

        
        if (v->etat == ETAT_SORTIE && 
            abs(v->x - v->cible_x) <= 3 && abs(v->y - v->cible_y) <= 3) {
            
            // Suppression de la liste chaînée
            Vehicule *a_supprimer = v;
            if (precedent == NULL) {
                liste_vehicules = v->suivant;
                v = liste_vehicules;
            } else {
                precedent->suivant = v->suivant;
                v = v->suivant;
            }
            free(a_supprimer);
            continue; // On passe direct au suivant
        }

        precedent = v;
        v = v->suivant;
    }
}

void afficher_vehicules_dynamiques(void) {
    Vehicule *v = liste_vehicules;
    while (v != NULL) {
        int largeur = modeles[v->type].largeur;
        // On centre le dessin sur la position X,Y
        int draw_x = v->x - (largeur/2);
        int draw_y = v->y - 1; 

        if (draw_x < 0) draw_x = 0;
        if (draw_y < 0) draw_y = 0;

        // On dessine les 3 lignes de la voiture
        for (int i=0; i<3; i++) {
            goto_xy(draw_x, draw_y + i);
            // Couleur différente selon l'état (Rouge=Roule, Bleu=Garé)
            if (v->etat == ETAT_GARE) printf("\033[34m"); // Bleu
            else printf(RED_TEXT); // Rouge
            
            printf("%s", modeles[v->type].forme[i]);
            printf(RESET);
        }
        v = v->suivant;
    }
}

void draw_spot(ParkingSpot spot, int is_selected) {
    (void)is_selected;
    int base_y = spot.screen_y - 1;
    if (base_y < 0) base_y = 0;

    // Si la place est occupée, on ne dessine RIEN (c'est la voiture qui se dessine par dessus)
    // Sinon on dessine le carré VERT
    if (!spot.is_occupied) {
        for (int dy = 0; dy < 3; dy++) {
            goto_xy(spot.screen_x, base_y + dy);
            printf("%s ", BG_GREEN);
            printf("%s", RESET);
        }
    } else {
        
    }
}

void draw_all_spots(int selected_index) {
    for (int i = 0; i < TOTAL_SPOTS; i++)
        draw_spot(all_spots[i], i == selected_index);
}

void liberer_memoire_vehicules() {
    Vehicule *v = liste_vehicules;
    while (v != NULL) {
        Vehicule *temp = v;
        v = v->suivant;
        free(temp);
    }
}