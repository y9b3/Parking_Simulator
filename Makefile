# ==============================================================================
# 
# DEFINITIONS DES VARIABLES
# 
# ==============================================================================

# Nom de l'exécutable final (le simulateur)
TARGET = parking_simulator

# Compilateur C
CC = gcc

# Options de compilation :
# -Wall : Active tous les warnings (très important !)
# -Wextra : Active des warnings supplémentaires
# -std=c11 : Utilisation d'une norme C moderne
# -Iinclude : Indique au compilateur de chercher les headers (.h) dans le dossier 'include'
CFLAGS = -Wall -Wextra -std=c11 -Iinclude

# Options de l'éditeur de liens (Linker)
# -lncursesw : Indispensable pour lier la bibliothèque ncurses (version widechar)
LDLIBS = -lncursesw -lm

# Fichiers sources (.c) à compiler
SRCS = src/main.c src/parking.c src/game_solo.c src/game_multi.c

# Fichiers objets (.o) générés à partir des sources
OBJS = $(SRCS:.c=.o)

# ==============================================================================
# 
# REGLES DE COMPILATION
# 
# ==============================================================================

# Règle par défaut : construire l'exécutable
all: $(TARGET)

# Règle principale : Lie les fichiers objets pour créer l'exécutable
# On ajoute $(LDLIBS) à la fin pour lier ncurses
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LDLIBS)

# Règle pour compiler chaque fichier source (.c) en fichier objet (.o)
# $< : Le premier prérequis (le fichier .c)
# $@ : La cible (le fichier .o)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Règle pour nettoyer le répertoire des fichiers générés
.PHONY: clean
clean:
	@echo "Nettoyage des fichiers objets et de l'exécutable..."
	rm -f $(OBJS) $(TARGET)
	@echo "Nettoyage terminé."

# Règle pour forcer la recompilation complète
.PHONY: re
re: clean all