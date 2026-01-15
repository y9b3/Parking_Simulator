# Simulateur de Parking en langage C :

Projet académique réalisé dans le cadre de notre 3A à l"ESIEA PARIS (2025/2026)
l'objectif est donc de programmer en langage C un jeu permettant de simuler le stationnement de voitures dans un parking payant
Tout se fait dans le **terminal sous GNU/LINUX**

## Présentation

Le jeu simule un parking avec :

- Une **entrée** et une **sortie** équipées de barrières automatiques.
- Une **borne d’entrée** distribuant des tickets.
- Une **borne de sortie** permettant le paiement.
- Des **places libres (vert)** et **occupées (rouge)**.
- Des **véhicules variés** (différents modèles et couleurs) :
  - déjà stationnés,
  - en recherche de place,
  - ou en train de sortir du parking.

Le plan du jeu est défini dans un fichier `.txt` et affiché directement dans le terminal.  
Les véhicules sont représentés par des **structures C** et gérés via des **listes chaînées**.

# Fonctionnalités Techniques :

1. Entrée et Gestion de Flotte
   Spawn dynamique : Dès le lancement, un premier véhicule apparaît au point d'entrée "D".

Ajout de véhicules : À tout moment, l'utilisateur peut générer une nouvelle voiture (Touche A) qui s'aligne parfaitement sur le point de spawn initial.

Contrôle multiple : Grâce au système de listes chaînées, l'utilisateur peut passer d'un véhicule à l'autre (Touche TAB). La voiture active passe en Jaune pour être immédiatement identifiable.

2. Navigation et Stationnement
   Pilotage : Les véhicules se déplacent avec précision (Z, Q, S, D) en respectant les voies de circulation tracées sur la map.

Système de Parking (G) : En se positionnant devant une place libre (Verte), l'utilisateur peut garer sa voiture. La place devient alors Rouge et mémorise l'ID du véhicule.

Libération de place : Le conducteur peut quitter son emplacement à tout moment, rendant la place à nouveau disponible pour les autres usagers.

3. Paiement et Sortie
   Borne de Sortie : Pour quitter le parking, l'utilisateur doit conduire son véhicule jusqu'à la zone "F".

Ticket de Sortie (F) : Une simple pression sur la touche F déclenche l'affichage d'un ticket détaillé. Ce ticket indique l'ID de la voiture, le temps passé en secondes et le tarif total à régler (0.50€ / sec).

Respawn après paiement : Une fois le paiement validé, le véhicule est automatiquement replacé au point d'entrée, prêt pour un nouveau cycle de stationnement.

Voici une nouvelle rubrique à ajouter à la fin de ton README.md. Elle est honnête, professionnelle et montre que vous avez analysé les limites de votre travail, ce qui est très apprécié par les professeurs lors des soutenances.

# Difficultés Rencontrées et Limites du Projet :

1 - La Gestion de la Mémoire (Double Free & Segmentation Fault)
L'une des plus grandes difficultés techniques a été la stabilisation des listes chaînées lors de la transition entre les différents menus du jeu. Nous avons rencontré plusieurs erreurs de type "double free detected" et des "Segmentation faults".

Ces crashs survenaient lorsque le programme tentait de libérer une zone mémoire déjà effacée lors de la sortie du mode solo.

Nous avons dû sécuriser les pointeurs et simplifier la procédure de nettoyage mémoire pour garantir que le terminal ne soit pas corrompu à la fermeture du simulateur.

2 - Problèmes d'Affichage et d'Encodage
Le rendu graphique via ncurses dans un conteneur Docker a posé des problèmes de compatibilité.

Le terminal affichait initialement des caractères incohérents au lieu des traits du parking.

Nous avons résolu cela en forçant l'utilisation des variables d'environnement locales (LANG=C.UTF-8 et TERM=xterm-256color) pour supporter l'encodage UTF-8 et les 256 couleurs.

3 - Le Mode Automatisé (Non implémenté)
L'une des ambitions initiales du projet était de créer un Mode Automatisé où les véhicules se déplaceraient seuls pour chercher une place et se garer sans intervention humaine.

Complexité des algorithmes : L'implémentation d'un algorithme de recherche de chemin (type A\*) dans une grille NCURSES s'est révélée trop complexe pour le temps imparti.

Gestion des priorités : Faire circuler plusieurs voitures automatiques en même temps sans qu'elles ne se bloquent mutuellement (deadlocks) représentait un défi logique majeur.

Résultat : Nous avons fait le choix de nous concentrer sur la stabilité du mode Solo et sur un système de contrôle multi-véhicules manuel performant (via TAB), plutôt que de livrer une automatisation buggée.

# Installation et Lancement (Docker) :

Le projet utilise un conteneur Docker pour garantir la stabilité de l'environnement de développement.

🛠️ Lancement du conteneur :

- docker-compose up -d
- docker compose exec dev bash
- Configuration du terminal (Crucial pour le rendu) :
  - Pour éviter que l'affichage ne soit corrompu (caractères bizarres), tapez ces commandes avant de lancer :
    - export TERM=xterm-256color
    - export LANG=C.UTF-8
    - export LC_ALL=C.UTF-8

🚀 Compilation et exécution :

make re
./parking_simulator

# Commandes de Jeu :

Z / Q / S / D : Déplacer la voiture active.

A : Ajouter une nouvelle voiture au point de spawn.

TAB : Changer de véhicule (la voiture active passe en JAUNE).

G : Garer le véhicule sur une place ou quitter une place.

F : Payer à la borne et sortir du parking.

E : Quitter le simulateur et revenir au menu.

# Contributeurs :

Projet réalisé en trinôme dans le cadre du module Projet Autonome en C.

- Cassandre
- Yanis
- Manal
