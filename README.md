Guide d'installation et d'exécution:<br>
Ce guide explique comment exécuter le programme de visualisation de structure de fichiers de type LOVC(ajouter, rechercher, supprimer, créer plusieurs enregistrements pour une structure LOVC avec GTK).<br>
Prérequis:<br>
Bibliothèques nécessaires :<br>
GTK (GTK 3 ).<br>
Compilateur GCC.<br>
Compiler le programme<br>
la version finale de l'interface ce trouve dans le fichier interface.c<br>
Placez le fichier versiofinale.c dans un répertoire de travail. Compilez le code avec la commande suivante :<br>
gcc `pkg-config --cflags --libs gtk+-3.0` -o visualiseur versiofinale.c<br>
Exécuter le programme<br>
Une fois la compilation terminée, lancez le programme avec la commande suivante :<br>
./visualiseur<br>
Si tout se passe bien, l'interface graphique s'ouvrira.<br>
vous trouvrer une video de l'interface de ce programme dans le lien suivant :<br>
https://drive.google.com/file/d/1vGJGolSIQQw1xZE9sDzk2uU2n-tuT47C/view?usp=sharing<br>

