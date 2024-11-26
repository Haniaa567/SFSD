void SuppressionLogiqueLOVC(Fichier* fichier,char* nom_physique,int numero)
{
    int trouv;
    int i,j;
   
    RechercheLOVC(fichier,nom_physique,numero,&i,&j,&trouv, NULL); //On effectue d'abord une recherche
    if(trouv == 1) //Si le livret a été trouvé
    {
        int nb_taille;
        Buffer buf;
        char taille[NB_TAILLE];
        Ouvrir(fichier,nom_physique,'A'); //On ouvre le fichier en mode ancien , il existe deja
        LireDir(fichier,i,&buf); //lire le bloc dans buf
        RecupChamp(fichier,NB_TAILLE,&buf,&i,&j,taille);  //On récupère le champ taille
        nb_taille = atoi(taille); //On le convertit en entier
        // cas 1 : la position du champ eff est dans le bloc courant
        if(j<B) 
        {
            buf.tab[j] = '1'; //On met à jour le champ à 1 ce qui signifie que la donnée a été effacé
            EcrireDir(fichier,i,&buf); //Ecrire le bloc
        }
        // cas 2 : la position du champ eff est dans le bloc suivant
        else 
        {
            i = buf.suivant; //On passe au bloc suivant
            if(i == -1) //S'il n'existe pas c'est une erreur de conception dans la strcture
            {
                printf("\t\t\tERREUR: Le champ efface n'existe pas\n");
            }
            else
            {
                LireDir(fichier,i,&buf); //On lie ce bloc
                buf.tab[B-j] = '1'; //On met à jour le champ à 1 ce qui signifie que la donnée a été effacé
                EcrireDir(fichier,i,&buf); //On écrit le bloc
            }
        }
        Aff_entete(fichier,3,Entete(fichier,3)+nb_taille); //On met à jour le champ (nombre de caractères supprimés) de l'entête
    }
    else //Si le livret n'a pas été trouvé
    {
        printf("\t\t\tLe livret que vous voulez supprimer n'existe pas\n");
    }

    /*if(automatic == 1) //Si la réorganisation automatique était activée
    {
        Fichier fichier2;
        if((float)Entete(fichier,3)/(float)Entete(fichier,2)>=seuil) //Si le seuil a été atteint ((nombre de caractères supprimés/nombre de caractères insérés)>=seuil)
        {
            Reorganisation(fichier,&fichier2,nom_physique,"Reo_Livrets_National.bin");
            printf("\t\t\tReorganisation effectuee !\n");
        }
    }*/
    Fermer(fichier);
}

