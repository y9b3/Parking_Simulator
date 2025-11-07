#ifndef MODELE_H
#define MODELE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Définition de la structure Modele
typedef struct Modele {
    char nom[50];
    char couleur[20];
    char type[20];
    float prix_horaire;
    struct Modele *suiv;
} Modele;

// Prototype de la fonction pour charger les modèles depuis un fichier
Modele* charger_modeles(const char *nom_fichier);

#endif // MODELE_H
