#ifndef LOVC_H_INCLUDED
#define LOVC_H_INCLUDED

#define B 1000 //Doit etre au moins 36 + NB_TAILLE
#define NB_TAILLE 10
//Declaration des strucures de types ==================================================================================>
//Type de l'enregistrement
typedef struct Donnee Donnee;
struct Donnee{
    char numero[10]; //Le numéro du livret sur 10 caratères
    char wilaya[14]; //La wilaya sur 14 caractères
    char type; //Le type T/M/A/I
    char superficie[10]; //La superficie sur 10 caractères
    char* observation; //Observation: Un champ de taille variable
    char taille[NB_TAILLE]; //Un champ pour sauvegarder la taille de l'enregistrement
    char eff; //Un champ qui à 0 signifie que l'enregistrement n'est pas supprimé et à 1 dans le cas contraire
};

//Type du bloc/buffer
typedef struct TBloc Buffer;
struct TBloc{
    char tab[B]; //Un tableau de taille B qui contient les enregistrements
    int suivant; //Un champ pour l'adresse du suivant bloc
};

//Type de l'entete
typedef struct TypeEntete TypeEntete;
struct TypeEntete{
    int adr_premier; //Adresse du premier bloc
    int nb_inseres; //Nombre de caractères insérés
    int nb_supprimes; //Nombre de caractères supprimés (Supression logique)
    int nb_blocs; // Nombre total de blocs
    int adr_queue; // Adresse de la queue de la liste
    int pos_queue; // Dernière position libre dans la queue
};

//Type du fichier LOVC
typedef struct Fichier Fichier;
struct Fichier{
    TypeEntete entete; //Le champ de l'entete
    FILE* fich; //Le fichier
};

//Définition des ensembles ======================================================================================================
char types[4] = {'T','M','A','I'}; //Ensemble des types possibles
char wilayas[58][14]={"Adrar","Chlef","Laghouat","OumElBouaghi","Batna","Bejaia","Biskra","Bechar","Blida","Bouira","Tamanrasset",
"Tebessa","Tlemcen","Tiaret","Tizi_Ouzou","Alger","Djelfa","Jijel","Setif","Saida","Skikda","Sidi_Bel_Abbes","Annaba","Guelma",
"Constantine","Medea","Mostaganem","M'Sila","Mascara","Ouargla","Oran","Bayadh","Illizi","BorjBouAreridj","Boumerdes","El_Taref",
"Tindouf","Tissemsilt","El_Oued","Khenchela","Souk_Ahras","Tipaza","Mila","Defla","Naama","Temouchent","Ghardaia","Relizane",
"Timimoun","Bordj_Badji","Ouled_Djellal","Beni_Abbes","In_Salah","In_Guezzam","Touggourt","Djanet","M'Ghair","Meniaa"}; //Ensemble des wilayas possibles

//Implementation de la machine abstraite ========================================================================================
//Renvoie pour chaque numero le champ de l'entete qui correspond
int Entete(Fichier* fichier,int i)
{
    switch(i)
    {
        case 1: return fichier->entete.adr_premier;
        case 2: return fichier->entete.nb_inseres;
        case 3: return fichier->entete.nb_supprimes;
        case 4: return fichier->entete.nb_blocs;
        case 5: return fichier->entete.adr_queue;
        case 6: return fichier->entete.pos_queue;
        default:{
            printf("\t\t\tERREUR: Cette position dans l'entete n'existe pas\n");
            return 0;
        }
    }
}

//Affecte à un champ de l'entete une certaine valeur
void Aff_entete(Fichier* fichier,int i,int val)
{
    switch(i)
    {
        case 1: {
            fichier->entete.adr_premier = val;
            break;
        }
        case 2: {
            fichier->entete.nb_inseres = val;
            break;
        }
        case 3: {
            fichier->entete.nb_supprimes = val;
            break;
        }
        case 4: {
            fichier->entete.nb_blocs = val;
            break;
        }
        case 5: {
            fichier->entete.adr_queue = val;
            break;
        }
        case 6: {
            fichier->entete.pos_queue = val;
            break;
        }
        default: {
            printf("\t\t\tERREUR: Cette position dans l'entete n'existe pas\n");
            break;
        }
    }
}

//Lire le bloc d'adresse i
void LireDir(Fichier* fichier, int i, Buffer* buf)
{
    fseek(fichier->fich,(sizeof(TypeEntete)+sizeof(Buffer)*(i-1)),SEEK_SET); //Positionner le curseur à l'adresse i
    fread(buf,sizeof(Buffer),1,fichier->fich); //Lire le bloc
}

//Ecrire le bloc à l'adresse i
void EcrireDir(Fichier* fichier, int i, Buffer* buf)
{
    fseek(fichier->fich,(sizeof(TypeEntete)+sizeof(Buffer)*(i-1)),SEEK_SET); //Positionner le curseur à l'adresse i
    fwrite(buf,sizeof(Buffer),1,fichier->fich); //Ecrire le bloc
}

//Ouvrir le fichier selon le mode
void Ouvrir(Fichier* fichier, char* nom_physique, char mode)
{
    if((mode == 'N') || (mode == 'n')) //Créer un nouveau fichier
    {
        fichier->fich = fopen(nom_physique,"wb+"); //Ouvrir le fichier en mode écriture (écrase le contenu du fichier s'il existe)
        //Initialiser tous les champs de l'entête
        Aff_entete(fichier,1,1);
        Aff_entete(fichier,2,0);
        Aff_entete(fichier,3,0);
        Aff_entete(fichier,4,0);
        Aff_entete(fichier,5,1);
        Aff_entete(fichier,6,0);
        fwrite(&(fichier->entete),sizeof(TypeEntete),1,fichier->fich); //Sauvegarder l'entête dans le fichier
    }
    else
    {
        if((mode == 'a') || (mode == 'A')) //Ouvrir un ancien fichier
        {
            fichier->fich = fopen(nom_physique,"rb+"); //Ouvrir le fichier en mode lecture/ecriture
            fread(&(fichier->entete),sizeof(TypeEntete),1,fichier->fich); //Lire l'entête et la charger dans le champ entête
        }
    }
}

//Ferme le fichier
void Fermer(Fichier* fichier)
{
    fseek(fichier->fich,0,SEEK_SET); //Positionne le curseur au début du fichier
    fwrite(&(fichier->entete),sizeof(TypeEntete),1,fichier->fich); //Sauvegarde l'entête
    fclose(fichier->fich); //Ferme le fichier
}

//Alloue un nouveau bloc et le chaine avec le reste
void AllocBloc(Fichier* fichier)
{
    Buffer* buf = malloc(sizeof(Buffer)); //Allouer de l'espace mémoire pour le buffer
    LireDir(fichier,Entete(fichier,5),buf); //Lire le dernier bloc de la liste
    buf->suivant = Entete(fichier,4) + 1; //Mettre à jour le champ suivant avec le nombre de blocs total de la liste + 1 (une nouvelle adresse)
    EcrireDir(fichier,Entete(fichier,5),buf); //Ecrire le dernier bloc
    strcpy(buf->tab,""); //Initialiser le buffer à une chaine de caractères vide
    buf->suivant = -1; //Initialiser le champ suivant à -1 (NIL)
    EcrireDir(fichier,Entete(fichier,4) + 1,buf); //Ecrire le buffer à l'adresse (nombre de blocs total de la liste + 1)
    Aff_entete(fichier,4,Entete(fichier,4)+1); //Incrémenter le nombre total de blocs de la liste
    Aff_entete(fichier,5,Entete(fichier,4)); //Mettre à jour le champ qui correspond à l'adresse du dernier bloc de la liste
}

//Outils utilisés
char* ChaineAlea(int taille)
{
    char* alpha = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.-#'?!"; //Une chaine qui contient tous les caractères possibles
    char* str = NULL; //La chaine aléatoire qu'on va générer
    str = (char*)malloc(sizeof(char)*(taille+2)); //Allouer de l'espace mémoire pour cette chaine
    for(int i=0;i<taille;i++)
    {
        str[i] = alpha[rand()%69]; //Choisir un caractère aléatoirement
    }
    str[taille] = '|'; //Ajouter un sépérateur
    str[taille+1] = '\0'; //Un caractère vide pour déterminer la fin de la chaine de caractères
    return str; //Retouner la chaine aléatoire
}

//Initialiser les enregistrements avec des séparateurs
void InitialiserDonnee(Donnee* d)
{
    strcpy(d->numero,"||||||||||");
    strcpy(d->wilaya,"||||||||||||||");
    strcpy(d->superficie,"||||||||||");
    d->taille[0] = '0';
    for(int i=1;i<NB_TAILLE;i++)
        d->taille[i] = '|';
    d->eff = '0';
}

//Concatène tous les champs de l'enregistrement en une chaine de caractères
char* ConcatDonnee(Donnee d)
{
    char* str;
    str = (char*)malloc(sizeof(char)*(37+NB_TAILLE+strlen(d.observation))); //Alloue l'espace mémoire pour la chaine de caractères
    int j=0;
    for(int i=0;i<NB_TAILLE;i++) //Le champ taille
    {
        if(d.taille[i]!='\0')
            str[j]=d.taille[i];
        else
            str[j]='|';
        j+=1;
    }
    str[j]=d.eff; //Le champ effacé
    j+=1;
    for(int i=0;i<10;i++) //Le champ numero
    {
        if(d.numero[i]!='\0')
            str[j]=d.numero[i];
        else
            str[j]='|';
        j+=1;
    }
    for(int i=0;i<14;i++) //Le champ wilaya
    {
        if(d.wilaya[i]!='\0')
            str[j]=d.wilaya[i];
        else
            str[j]='|';
        j+=1;
    }
    str[j]=d.type; //Le champ type
    j+=1;
    for(int i=0;i<10;i++) //Le champ superficie
    {
        if(d.superficie[i]!='\0')
            str[j]=d.superficie[i];
        else
            str[j]='|';
        j+=1;
    }
    for(int i=0;i<strlen(d.observation);i++) //Le champ observation
    {
        str[j]=d.observation[i];
        j+=1;
    }
    str[j] = '\0'; //Le dernier caratère de la chaine
    return str; //Retourner la chaine conacténée
}

//Recupère le champ de l'enregistrement à partir du fichier
void RecupChamp(Fichier* fichier,int n,Buffer* buf,int* i,int* j,char* donnee)
{
    int k = 0;
    while(k<n)
    {
        if(*j>=B) //En cas où le champ est divisé sur plus d'un bloc
        {
            *i = buf->suivant;
            *j = 0; //Rénitiliser le j à 0 (au début du nouveau bloc)
            LireDir(fichier,*i,buf); //Lire le bloc suivant
        }

        if(buf->tab[*j]!='|') //Si le caractère n'est pas un séparateur
        {
            donnee[k] = buf->tab[*j];
        }
        else //Sinon mettre des caractères vides
        {
            donnee[k] = '\0';
        }
        (*j)+=1;
        k+=1;
    }
}

//Récupère la chaine du fichier comme elle est sans enlever les séparateurs
void RecupChaine(Fichier* fichier,int n,Buffer* buf,int* i,int* j,char donnee[])
{
    int k = 0;
    while(k<n)
    {
        if(*j>=B) //En cas où la chaine est divisée sur plus d'un bloc
        {
            *i = buf->suivant;
            *j = 0; //Rénitiliser le j à 0 (au début du nouveau bloc)
            LireDir(fichier,*i,buf);
        }

        donnee[k] = buf->tab[*j];
        (*j)+=1;
        k+=1;
    }
    donnee[k] = '\0';
}

//Ecrie la chaine de caractères de taille n à la fin d'un fichier à partir de l'adresse i et la position j
void EcrireChaine(Fichier* fichier,char* nom_physique,int n,Buffer* buf,int* i,int* j,char donnee[])
{
    int k = 0;
    Ouvrir(fichier,nom_physique,'A'); //Ouvrir le fichier en mode ancien
    LireDir(fichier,*i,buf); //Lire le bloc d'adresse i
    while(k<n) //Tant que la chaine n'est pas finie
    {
        if(*j>=B) //Si la chaine dépasse la capacité du bloc
        {
            EcrireDir(fichier,*i,buf); //Ecrire le bloc à l'adresse i
            AllocBloc(fichier); //Allouer un nouveau bloc à la fin du fichier et le chainer
            *i = Entete(fichier,5); //Mettre à jour i à l'adresse de la queue de la liste
            *j = 0; //Réinitialiser j à 0
            LireDir(fichier,*i,buf); //Lire le nouveau bloc
        }
        buf->tab[*j] = donnee[k]; //Mettre à jour un caractère du buffer avec un caractère de la chaine qu'on veut écrire
        (*j)+=1;
        k+=1;
    }
    donnee[k] = '\0';
    EcrireDir(fichier,*i,buf); //Ecrire le bloc à l'adresse i
    Aff_entete(fichier,6,*j); //Mettre à jour la dernière position libre dans la queue
    Aff_entete(fichier,2,Entete(fichier,2)+n); //Mettre à jour le nombre de caractères insérés
    Fermer(fichier); //Fermer le fichier
}

//Une fonction pour demander à l'utilisateur d'entrer chaque champ manuellement et les concaténer en une chaine de caractères
char* EntrerDonnee(int numero)
{
    Donnee d; //Déclarer d comme enregistrement
    InitialiserDonnee(&d); //Initialiser les champs de d
    sprintf(d.numero,"%d",numero); //Affecter numero(entier) à d.numero(chaine de caractères)
    printf("\t\t\tEntrez la wilaya : ");
    scanf("%s",d.wilaya); //Lire le champ wilaya
    printf("\t\t\tEntrez le type : ");
    scanf(" %c",&(d.type)); //Lire le champ type
    printf("\t\t\tEntrez la superficie : ");
    scanf("%s",d.superficie); //Lire le champ superficie
    printf("\t\t\tEntrez l'observation (sans espaces): ");
    d.observation = (char*)malloc(sizeof(char)*250);
    scanf("%s",d.observation); //Lire le champ observation
    sprintf(d.taille,"%d",NB_TAILLE+1+35+strlen(d.observation));
    return ConcatDonnee(d); //Retouner les champs concaténés
}


//Operations sur LOVC ==============================================================================================================================
//Cette procédure recherche un livret dans le fichier d'après le numéro
//i: l'adresse du bloc et j: la position dans le bloc
//(i,j) représentent l'emplacement du livret s'il existe sinon où il est supposé placé
void RechercheLOVC(Fichier* fichier,char* nom_physique,int val,int* i,int* j,int* trouv)
{
    Buffer buf; //Pour lire et écrire les blocs
    int stop; //Pour stopper la boucle dans certaines conditions
    int sauvi; //Pour sauvegarder i
    int sauvj; //Pour sauvegarder j
    char taille[NB_TAILLE];
    char numero[10];
    char eff;
    char* d; //La chaine de caratères d'un enregistrement après le champ taille, eff et numero
    int nb_taille,nb_numero; //Les champs taille et numero en entiers

    Ouvrir(fichier,nom_physique,'A'); //Ouvrir le fichier en mode ancien
    *trouv = 0; //Positionner trouv à Faux
    stop = 0; //Positionner stop à Faux
    *i = Entete(fichier,1);
    *j = 0;
    LireDir(fichier,*i,&buf); //Lire le premier bloc

    //Le cas d'un fichier vide
    if((*j>=Entete(fichier,6))&&(*i>=Entete(fichier,5)))
    {
        stop = 1; //stop à Vrai
    }

    while((*trouv == 0)&&(stop == 0)) //Tant qu'on a pas trouvé le livret et que aucune condition n'a été vérifiée pour stopper la boucle
    {
        sauvi = *i; //Sauvegarder i
        sauvj = *j; //Sauvegarder j
        RecupChamp(fichier,NB_TAILLE,&buf,i,j,taille); //Recupérer le champ taille (le i et j sont mis à jour après le champ)
        nb_taille = atoi(taille); //Récupérer le champ en tant qu'entier
        RecupChamp(fichier,1,&buf,i,j,&eff); //Récupérer le champ eff
        RecupChamp(fichier,10,&buf,i,j,numero); //Récupérer le champ numéro
        nb_numero = atoi(numero); //Récupérer le champ en tant qu'entier
        d = malloc(sizeof(char)*(nb_taille-NB_TAILLE-10));
        RecupChaine(fichier,nb_taille-NB_TAILLE-11,&buf,i,j,d); //Récupérer le reste de l'enregistrement

        if(nb_numero == val) //Le meme numéro a été trouvé
        {
            if(eff == '0') //Le livret n'est pas effecé donc il a été trouvé
                *trouv = 1; //trouv à Vrai
            else
                stop = 1; //Le livret a été supprimé donc on met stop à Vrai
            *i = sauvi; //Récupérer i avant d'avoir lu cet enregistrement
            *j = sauvj; //Récupérer j avant d'avoir lu cet enregistrement
        }
        else
        {
            if(nb_numero>val) //Si la valeur recherchée est inférieure à la valeur lue il ne sert à rien de continuer la recherche donc il faut la stopper
            {
                stop = 1; //stop à Vrai
                *i = sauvi; //Récupérer i avant d'avoir lu cet enregistrement
                *j = sauvj; //Récupérer j avant d'avoir lu cet enregistrement
            }
        }
        if((*j>=Entete(fichier,6))&&(*i>=Entete(fichier,5))) //Si on arrive à la fin du fichier
        {
            stop = 1; //stop à Vrai
        }

        free(d); //On libère l'espace
    }
    Fermer(fichier); //Fermer le fichier
}

//Cette procédure insère un nouveau livret d'après le numéro dans la bonne position
void InsertionLOVC(Fichier* fichier,char* nom_physique,int numero,char* donnee)
{
    Buffer buf;
    int trouv;
    int stop = 0;
    int i = 1;
    int j = 0;
    int l = 0;
    int rest;
    RechercheLOVC(fichier,nom_physique,numero,&i,&j,&trouv); //On effectue une recherche pour avoir l'adresse i et la position j où insérer
    if(trouv == 0) //Si le numéro n'existe pas déjà dans le fichier
    {
        Ouvrir(fichier,nom_physique,'A'); //Ouvrir le fichier en mode ancien
        int sauvtaille = strlen(donnee); //On sauvegarde la taille du nouveau enregistrement
        Aff_entete(fichier,2,Entete(fichier,2)+sauvtaille); //Mettre à jour le champ (nombre de caractères insérés) de l'entête
        Aff_entete(fichier,6,(Entete(fichier,6)+sauvtaille)%B); //Mettre à jour le champ (dernière position libre de la queue) de l'entête
        char* temp_donnee = (char*)malloc((sauvtaille)*sizeof(char)); //Allouer un espace mémoire de la même taille que le nouveau enregistrement
        while(stop == 0) //Tant que l'insertion n'est pas finie
        {
            LireDir(fichier,i,&buf); //Lire le bloc à l'adresse i
            if(j+sauvtaille<=B) //Si il y a assez de place pour l'enregistrement dans le bloc courant
            {
                for(int k=0;k<sauvtaille;k++)
                {
                    temp_donnee[k] = buf.tab[j+k]; //Sauvegarder les caractères qui se trouvaient déjà dans la position où on veut insérer
                    buf.tab[j+k] = donnee[k]; //Insérer un caratère du nouveau enregistrement
                }
                temp_donnee[sauvtaille] = '\0';
                EcrireDir(fichier,i,&buf); //Ecrire le bloc
                j+=sauvtaille; //Faire avancer la position
                strcpy(donnee,temp_donnee); //Copier les caractères sauvegardés précédemment dans la donnée courante qu'on veut insérer
            }
            else //Si il n'y a pas assez de place pour l'enregistrement dans le bloc courant
            {
                rest = j+sauvtaille - B; //Nombre de caractères qu'on insèrera dans le bloc suivant
                l = 0;
                while(j<B) //Insérer les caractères jusqu'à la fin du bloc
                {
                    temp_donnee[l] = buf.tab[j];
                    buf.tab[j] = donnee[l];
                    j++;
                    l++;
                }
                EcrireDir(fichier,i,&buf); //Ecrire le bloc
                i = buf.suivant; //Passer au prochain bloc
                j=0;
                if(i == -1) //Si le bloc courant était le dernier bloc
                {
                    AllocBloc(fichier); //On alloue un nouveau bloc
                    i = Entete(fichier,5); //Mettre à jour le champ (adresse de la queue) de l'entête
                    stop = 1; //On stop l'insertion car on est arrivé au dernier bloc
                }
                LireDir(fichier,i,&buf); //Lire le nouveau bloc
                for(int k=0;k<rest;k++) //Insérer les caractères dans ce nouveau bloc
                {
                    temp_donnee[l] = buf.tab[j];
                    buf.tab[j] = donnee[l];
                    j++;
                    l++;
                }
                temp_donnee[strlen(donnee)] = '\0';
                EcrireDir(fichier,i,&buf); //Ecrire le bloc
                strcpy(donnee,temp_donnee); //Copier les caractères sauvegardés précédemment dans la donnée courante qu'on veut insérer
            }
            if((i == Entete(fichier,5))&&(j>Entete(fichier,6))) //Si on arrive au dernier bloc et à la dernière position
            {
                stop = 1; //On stoppe l'insertion
            }
        }
        free(temp_donnee);
        Fermer(fichier);
    }
    else //Si le numéro existe déjà dans le fichier
    {
        printf("\t\t\tUn livret avec le meme numero existe deja\n");
    }
}

//Cette procédure réalise la réorganisation du fichier dans un nouveau fichier
void Reorganisation(Fichier* fichier,Fichier* nouveau_fichier,char* nom_physique,char* nouveau_nom)
{
    Buffer buf;
    int i,j,index_i,index_j,index;
    int nb_taille;
    char taille[NB_TAILLE];
    char eff;
    char* d;
    char* donnee;
    int stop = 0;
    Ouvrir(fichier,nom_physique,'A'); //Ouvrir l'ancien fichier en mode ancien
    i = Entete(fichier,1);
    j = 0;
    Ouvrir(nouveau_fichier,nouveau_nom,'N'); //Ouvrir un nouveau fichier en mode nouveau
    AllocBloc(nouveau_fichier); //Allouer le premier bloc du nouveau fichier
    index_i = Entete(nouveau_fichier,5); //Recupérer l'adresse indexi du nouveau fichier
    index_j = Entete(nouveau_fichier,6); //Recupérer la position indexj du nouveau fichier
    Fermer(nouveau_fichier);
    while(stop == 0) //Tant que la réorganisation n'est pas finie
    {
        LireDir(fichier,i,&buf); //Lire le premier bloc de l'ancien fichier
        RecupChamp(fichier,NB_TAILLE,&buf,&i,&j,taille); //Recupérer le champ taille
        RecupChamp(fichier,1,&buf,&i,&j,&eff); //Récupérer le champ effacé
        nb_taille = atoi(taille); //Convertir taille en entier
        d = malloc(sizeof(char)*(nb_taille-NB_TAILLE));
        RecupChaine(fichier,nb_taille-1-NB_TAILLE,&buf,&i,&j,d); //Récupérer le reste de l'enregistrement
        donnee = malloc(sizeof(char)*(nb_taille+1));
        index = 0;
        //Concaténer tous les champs en une chaine de caractères (donnee)
        for(int k=0;k<NB_TAILLE;k++)
        {
            if(taille[k] != '\0')
            {
                donnee[index] = taille[k];
            }
            else
            {
                donnee[index] = '|';
            }
            index++;
        }
        donnee[index] = eff;
        index++;
        for(int k=0;k<strlen(d);k++)
        {
            donnee[index] = d[k];
            index++;
        }
        donnee[nb_taille] = '\0';
        if(eff == '0') //Si la donnee n'était pas éffacé logiquement
        {
            EcrireChaine(nouveau_fichier,nouveau_nom,nb_taille,&buf,&index_i,&index_j,donnee); //Ecrire la chaine de caracères (donnee) à la fin du nouveau fichier
        }
        free(d);

        if((i == Entete(fichier,5))&&(j >= Entete(fichier,6))) //Si on arrive à la fin de l'ancien fichier
            stop = 1; //On arrete la réorganisation
    }
    Fermer(fichier);
}

//Cette procédure supprime logiquement un livret d'après son numéro
void SuppressionLogique(Fichier* fichier,char* nom_physique,int numero,int automatic,float seuil)
{
    int trouv;
    int i,j;
    int nb_taille;
    Buffer buf;
    char taille[NB_TAILLE];
    RechercheLOVC(fichier,nom_physique,numero,&i,&j,&trouv); //On effectue d'abord une recherche
    Ouvrir(fichier,nom_physique,'A'); //On ouvre le fichier en mode ancien
    LireDir(fichier,i,&buf); //On lie le bloc
    RecupChamp(fichier,NB_TAILLE,&buf,&i,&j,taille); //On récupère le champ taille
    nb_taille = atoi(taille); //On le convertit en entier
    if(trouv == 1) //Si le livret a été trouvé
    {
        if(j<B) //la position du champ eff est dans le bloc courant
        {
            buf.tab[j] = '1'; //On met à jour le champ à 1 ce qui signifie que la donnée a été effacé
            EcrireDir(fichier,i,&buf); //Ecrire le bloc
        }
        else //la position du champ eff est dans le bloc suivant
        {
            i = buf.suivant; //On passe au bloc suivant
            if(i == -1) //S'il n'existe pas c'est une erreur de conception
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

    if(automatic == 1) //Si la réorganisation automatique était activée
    {
        Fichier fichier2;
        if((float)Entete(fichier,3)/(float)Entete(fichier,2)>=seuil) //Si le seuil a été atteint ((nombre de caractères supprimés/nombre de caractères insérés)>=seuil)
        {
            Reorganisation(fichier,&fichier2,nom_physique,"Reo_Livrets_National.bin");
            printf("\t\t\tReorganisation effectuee !\n");
        }
    }
    Fermer(fichier);
}

//Procédure pour générer des livrets aléatoires
void GenererContenuAlea(Fichier* fichier,char* nom_physique,int nb_livret)
{
    srand(time(NULL)); //Pour les fonctions aléatoires
    int i=1; //Se positionner au début du fichier
    int j=0;
    Buffer buf;
    Ouvrir(fichier,nom_physique,'N'); //Ouvrir le fichier en mode nouveau
    AllocBloc(fichier); //Allouer le premier bloc du fichier
    for(int k=0;k<nb_livret;k++) //Insérer les livrets un par un
    {
        LireDir(fichier,i,&buf); //Lire le bloc

        Donnee nouvelle_donnee; //Créer une nouvelle donnée
        InitialiserDonnee(&nouvelle_donnee); //L'initialiser
        sprintf(nouvelle_donnee.numero,"%d",k); //Générer le numéro
        strcpy(nouvelle_donnee.wilaya,wilayas[rand()%58]); //Générer la wilaya
        nouvelle_donnee.type = types[rand()%4]; //Générer le type
        sprintf(nouvelle_donnee.superficie,"%d",rand()%10000000000); //Générer la superficie
        nouvelle_donnee.observation = ChaineAlea(rand()%(B-36-NB_TAILLE)); //Générer l'observation
        //nouvelle_donnee.observation = ChaineAlea(rand()%20); //Générer l'observation

        sprintf(nouvelle_donnee.taille,"%d",NB_TAILLE+1+35+strlen(nouvelle_donnee.observation)); //Calculer la taille
        char* str = ConcatDonnee(nouvelle_donnee); //Concaténer tous les champs
        int index = 0;
        while(index<strlen(str)) //Insérer la donnée caractère par caractère
        {
            if(j<B) //Si la position est inférieure à la taille du bloc
            {
                buf.tab[j] = str[index]; //Insérer le caractère
                index+=1;
                j+=1;
            }
            else //Si le caractère doit s'insérer dans le bloc suivant
            {
                j=0;
                EcrireDir(fichier,i,&buf); //On écrit le bloc
                AllocBloc(fichier); //Alloue un nouveau bloc
                i=Entete(fichier,5); //Met à jour l'adresse i
                LireDir(fichier,i,&buf); //Lire le nouveau bloc
            }
        }
        EcrireDir(fichier,i,&buf); //Ecrit le dernier bloc
        Aff_entete(fichier,2,Entete(fichier,2)+strlen(str)); //Met à jour le nombre de caractères insérés dans l'entête
        Aff_entete(fichier,6,j); //Met à jour la dernière position dans la queue dans l'entête
    }
    Fermer(fichier);
}

//Procédure pour afficher le fichier en entier
void AfficherFichier(Fichier* fichier,char* nom_physique)
{
    Buffer buf;
    int i,j,index;
    int nb_taille;
    char taille[NB_TAILLE];
    char eff;
    char* d;
    char* donnee;
    index = 0;
    int stop = 0;
    Ouvrir(fichier,nom_physique,'A'); //Ouvrir le fichier en mode ancien
    i = Entete(fichier,1);
    j = 0;
    while(stop == 0)
    {
        LireDir(fichier,i,&buf); //Lire le bloc
        RecupChamp(fichier,NB_TAILLE,&buf,&i,&j,taille); //Récupère le champ taille
        RecupChamp(fichier,1,&buf,&i,&j,&eff); //Récupère le champ effacé
        nb_taille = atoi(taille); //Convertie taille en entier
        d = malloc(sizeof(char)*(nb_taille-NB_TAILLE));
        donnee = malloc(sizeof(char)*(nb_taille+1));
        RecupChaine(fichier,nb_taille-1-NB_TAILLE,&buf,&i,&j,d); //Recupère le reste de la donnée
        index = 0;
        //Concaténer tous les champs de l'enregistrement
        for(int k=0;k<NB_TAILLE;k++)
        {
            if(taille[k] != '\0')
            {
                donnee[index] = taille[k];
            }
            else
            {
                donnee[index] = '|';
            }
            index++;
        }
        donnee[index] = eff;
        index++;
        for(int k=0;k<strlen(d);k++)
        {
            donnee[index] = d[k];
            index++;
        }
        donnee[nb_taille] = '\0';
        printf("%s\n",donnee); //Afficher l'enregistrement en entier
        if((i == Entete(fichier,5))&&(j >= Entete(fichier,6))) //Si on arrive à la fin du fichier on stoppe l'affichage
            stop = 1;
    }
    Fermer(fichier);
}

#endif // LOVC_H_INCLUDED