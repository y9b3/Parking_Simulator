# Utilisation d'Ubuntu 22.04 comme base pour l'environnement Linux
FROM ubuntu:22.04

# Empeche les interruptions interactives durant l'installation
ENV DEBIAN_FRONTEND=noninteractive

# Mise a jour et installation des outils de compilation et bibliotheques
RUN apt-get update && apt-get install -y \
    build-essential \
    gdb \
    valgrind \
    libncurses5-dev \
    libncursesw5-dev \
    sox \
    libsox-dev \
    ttf-ancient-fonts \
    && apt-get clean

# Repertoire de travail dans le conteneur
WORKDIR /app

# Maintien du conteneur en execution1
CMD ["tail", "-f", "/dev/null"]