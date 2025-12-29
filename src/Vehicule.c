#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "vehicule.h"

// Liste chaînée pour gérer les véhicules
VEHICULE *tete_de_liste = NULL;

// Création d’un véhicule
VEHICULE* creer_vehicule(char direction, int posx, int posy, int vitesse,
                         char alignement, char type, char *carrosserie,
                         int code_couleur) {
    VEHICULE *nouveau_vehicule = (VEHICULE *)malloc(sizeof(VEHICULE));
    if (nouveau_vehicule == NULL) {
        perror("Erreur d'allocation mémoire pour un véhicule.");
        exit(EXIT_FAILURE);
    }

    // Initialisation des attributs
    nouveau_vehicule->direction = direction;
    nouveau_vehicule->posx = posx;
    nouveau_vehicule->posy = posy;
    nouveau_vehicule->vitesse = vitesse;
    nouveau_vehicule->alignement = alignement;
    nouveau_vehicule->type = type;

    // Copie de la carrosserie (ici une seule ligne pour simplifier)
    strncpy(nouveau_vehicule->carrosserie[0], carrosserie, 30);

    nouveau_vehicule->code_couleur = code_couleur;
    nouveau_vehicule->etat = 1; // actif
    nouveau_vehicule->tps = time(NULL); // temps d’entrée dans le parking
    nouveau_vehicule->next = NULL;

    return nouveau_vehicule;
}

// Ajout d’un véhicule à la liste
void ajouter_vehicule(VEHICULE *nouveau_vehicule) {
    if (tete_de_liste == NULL) {
        tete_de_liste = nouveau_vehicule;
    } else {
        VEHICULE *temp = tete_de_liste;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = nouveau_vehicule;
    }
}

// Suppression d’un véhicule
void supprimer_vehicule(VEHICULE *vehicule_a_supprimer) {
    VEHICULE *temp = tete_de_liste;
    VEHICULE *precedent = NULL;

    if (temp != NULL && temp == vehicule_a_supprimer) {
        tete_de_liste = temp->next;
        free(temp);
        printf("Véhicule supprimé.\n");
        return;
    }

    while (temp != NULL && temp != vehicule_a_supprimer) {
        precedent = temp;
        temp = temp->next;
    }

    if (temp == NULL) return;

    precedent->next = temp->next;
    free(temp);
    printf("Véhicule supprimé.\n");
}

// Affichage de tous les véhicules
void afficher_vehicules() {
    VEHICULE *temp = tete_de_liste;
    while (temp != NULL) {
        printf("Véhicule : %c | Position : (%d, %d) | Vitesse : %d | Couleur : %d\n",
               temp->type, temp->posx, temp->posy, temp->vitesse, temp->code_couleur);
        temp = temp->next;
    }
}

// Recherche d’un véhicule par son nom (carrosserie[0])
VEHICULE* chercher_vehicule(char *nom) {
    VEHICULE *temp = tete_de_liste;
    while (temp != NULL) {
        if (strcmp(temp->carrosserie[0], nom) == 0) {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

// Déplacement d’un véhicule selon sa direction
void deplacer_vehicule(VEHICULE *v) {
    if (v == NULL) return;
    switch (v->direction) {
        case 'N': v->posy--; break;
        case 'S': v->posy++; break;
        case 'E': v->posx++; break;
        case 'O': v->posx--; break;
    }
}

// Calcul du tarif à la sortie
float calculer_tarif(VEHICULE *v) {
    if (v == NULL) return 0.0;
    unsigned long duree = time(NULL) - v->tps;
    float tarif = duree * 0.25; // exemple : 0.25 €/seconde
    return tarif;
}

// Déplacement de tous les véhicules de la liste
void deplacer_tous_vehicules() {
    VEHICULE *temp = tete_de_liste;
    while (temp != NULL) {
        deplacer_vehicule(temp);
        temp = temp->next;
    }
}

void afficher_vehicules_sur_map() {
    // Crée une grille vide
    char grille[20][40]; // à adapter selon ta map

    // Remplir la grille avec des espaces
    for (int y = 0; y < 20; y++) {
        for (int x = 0; x < 40; x++) {
            grille[y][x] = ' ';
        }
    }

    // Placer les véhicules
    VEHICULE *temp = tete_de_liste;
    while (temp != NULL) {
        if (temp->posx >= 0 && temp->posx < 40 && temp->posy >= 0 && temp->posy < 20) {
            grille[temp->posy][temp->posx] = temp->type; // 'v' ou 'c'
        }
        temp = temp->next;
    }

    // Afficher la grille
    system("clear"); // ou "cls" sur Windows
    for (int y = 0; y < 20; y++) {
        for (int x = 0; x < 40; x++) {
            printf("%c", grille[y][x]);
        }
        printf("\n");
    }
}



