Guide d'installation et d'exécution:
Ce guide explique comment exécuter le programme de visualisation de structure de fichiers de type LOVC(ajouter, rechercher, supprimer, créer plusieurs enregistrements pour une structure LOVC avec GTK).
Prérequis
Bibliothèques nécessaires :
GTK (GTK 3 ).
Compilateur GCC.
Compiler le programme
la version finale de l'interface ce trouve dans le fichier interface.c
Placez le fichier versiofinale.c dans un répertoire de travail. Compilez le code avec la commande suivante :
gcc `pkg-config --cflags --libs gtk+-3.0` -o visualiseur versiofinale.c
Exécuter le programme
Une fois la compilation terminée, lancez le programme avec la commande suivante :
./visualiseur
Si tout se passe bien, l'interface graphique s'ouvrira.
vous trouvrer une video de l'interface de ce programme dans le lien suivant :

