#define _XOPEN_SOURCE 700
#include <ncurses.h>
#include <locale.h>
#include <stdlib.h>
#include <time.h>
#include "../include/parking.h"

/* --- GESTION DU MENU PRINCIPAL --- */
int afficher_menu_ncurses()
{
    char *choix_txt[] = {
        "1. Mode SOLO (Conduite Libre)",
        "2. Mode MULTI (Reseau)",
        "3. Quitter"};
    int n_choix = 3;
    int selection = 0;
    int ch;

    /* Configuration NCURSES pour le menu */
    nodelay(stdscr, FALSE);
    keypad(stdscr, TRUE);

    while (1)
    {
        clear();

        /* Affichage de l'en-tete */
        attron(A_BOLD | COLOR_PAIR(1));
        mvprintw(2, 4, "====================================");
        mvprintw(3, 4, "   PARKING SIMULATOR 2026 (NCURSES) ");
        mvprintw(4, 4, "====================================");
        attroff(A_BOLD | COLOR_PAIR(1));

        /* Affichage des options avec mise en evidence de la selection */
        for (int i = 0; i < n_choix; i++)
        {
            if (i == selection)
            {
                attron(A_REVERSE);
                mvprintw(7 + i * 2, 6, "-> %s", choix_txt[i]);
                attroff(A_REVERSE);
            }
            else
            {
                mvprintw(7 + i * 2, 9, "%s", choix_txt[i]);
            }
        }
        mvprintw(15, 4, "Utilisez HAUT/BAS et ENTREE");

        /* Lecture de l'entree utilisateur */
        ch = getch();
        switch (ch)
        {
        case KEY_UP:
            selection--;
            if (selection < 0)
                selection = n_choix - 1;
            break;
        case KEY_DOWN:
            selection++;
            if (selection >= n_choix)
                selection = 0;
            break;
        case 10: /* Touche ENTREE */
            return selection + 1;
        }
    }
}

/* --- POINT D'ENTREE DU PROGRAMME --- */
int main()
{
    /* Initialisation de l'environnement et des caracteres etendus */
    setlocale(LC_ALL, "");
    initscr();
    resize_term(HAUTEUR_MAX, LARGEUR_MAX);
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);

    /* Initialisation des paires de couleurs demandees par le sujet */
    if (has_colors())
    {
        start_color();
        init_pair(1, COLOR_CYAN, COLOR_BLACK);  /* Decor */
        init_pair(2, COLOR_GREEN, COLOR_BLACK); /* Places libres */
        init_pair(3, COLOR_RED, COLOR_BLACK);   /* Places occupees */
    }

    srand(time(NULL));
    int continuer_programme = 1;

    /* Boucle principale de l'application */
    while (continuer_programme)
    {
        int mode = afficher_menu_ncurses();
        switch (mode)
        {
        case 1:
            jouer_mode_solo(); /* Lance le mode de conduite manuelle */
            break;
        case 2:
            jouer_mode_multi(); /* Lance le mode automatique / reseau */
            break;
        case 3:
            continuer_programme = 0;
            break;
        }
    }

    /* Fermeture propre de NCURSES */
    endwin();
    printf("Fin du programme.\n");
    return 0;
}