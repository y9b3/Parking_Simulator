#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Modele {
    char nom[50];
    char couleur[20];
    char type[20];$
    float prix_horaire;
    struct Modele *suiv;
} Modele;

Modele* charger_modeles(const char *nom_fichier) {
    FILE *f = fopen(nom_fichier, "r");
    if (!f) {
        perror("Erreur d’ouverture du fichier de modèles");
        return NULL;
    }

    Modele *tete = NULL, *courant = NULL;
    char ligne[128];

    while (fgets(ligne, sizeof(ligne), f)) {
        if (ligne[0] == '#' || ligne[0] == '\n')
            continue; // ignorer les commentaires et lignes vides

        Modele *nouveau = malloc(sizeof(Modele));
        if (!nouveau) {
            perror("Erreur d’allocation mémoire");
            fclose(f);
            return tete;
        }

        sscanf(ligne, "%s %s %s %f",
               nouveau->nom,
               nouveau->couleur,
               nouveau->type,
               &nouveau->prix_horaire);

        nouveau->suiv = NULL;

        if (tete == NULL)
            tete = nouveau;
        else
            courant->suiv = nouveau;

        courant = nouveau;
    }

    fclose(f);
    return tete;
}
