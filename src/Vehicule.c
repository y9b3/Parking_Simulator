#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vehicule.h"

// prof demande liste chainées pour gerer les vehicules 
VEHICULE *tete_de_liste = NULL;

VEHICULE* creer_vehicule(char direction, int posx, int posy, int vitesse, char alignement, char type, char *carrosserie, int code_couleur) {
    VEHICULE *nouveau_vehicule = (VEHICULE *)malloc(sizeof(VEHICULE));
    if (nouveau_vehicule == NULL) {
        perror("Erreur d'allocation mémoire pour un véhicule.");
        exit(1);
    }

    // initialisation pour les attributs vehicule 
    nouveau_vehicule->direction = direction;
    nouveau_vehicule->posx = posx;
    nouveau_vehicule->posy = posy;
    nouveau_vehicule->vitesse = vitesse;
    nouveau_vehicule->alignement = alignement;
    nouveau_vehicule->type = type;
    strcpy(nouveau_vehicule->carrosserie[0], carrosserie);
    nouveau_vehicule->code_couleur = code_couleur;
    nouveau_vehicule->etat = 1;
    nouveau_vehicule->tps = 0;
    nouveau_vehicule->next = NULL;

    return nouveau_vehicule;
}

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

void supprimer_vehicule(VEHICULE *vehicule_a_supprimer) {
    VEHICULE *temp = tete_de_liste;
    VEHICULE *precedent = NULL;

    if (temp != NULL && temp == vehicule_a_supprimer) {
        tete_de_liste = temp->next;
        free(temp);
        return;
    }

    while (temp != NULL && temp != vehicule_a_supprimer) {
        precedent = temp;
        temp = temp->next;
    }

    if (temp == NULL) return;

    precedent->next = temp->next;
    free(temp);
}

void afficher_vehicules() {
    VEHICULE *temp = tete_de_liste;
    while (temp != NULL) {
        printf("Véhicule : %c | Position : (%d, %d) | Vitesse : %d\n", temp->type, temp->posx, temp->posy, temp->vitesse);
        temp = temp->next;
    }
}
