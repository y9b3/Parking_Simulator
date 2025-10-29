# Utiliser une image Ubuntu 22.04 comme base
FROM ubuntu:22.04

# Éviter les questions interactives pendant l'installation
ENV DEBIAN_FRONTEND=noninteractive

# Mettre à jour les paquets et installer les dépendances du projet
RUN apt-get update && apt-get install -y \
    build-essential \
    gdb \
    valgrind \
    libncurses-dev \
    sox \
    libsox-dev \
    ttf-ancient-fonts \
    && apt-get clean # Nettoyer le cache

# Créer un répertoire de travail
WORKDIR /app

# Commande pour garder le conteneur en vie
# (Il attendra que tu t'y connectes)
CMD ["tail", "-f", "/dev/null"]