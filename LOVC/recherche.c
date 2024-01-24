#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include <gtk/gtk.h>
//#include <time.h>
#define B 100 //Doit etre au moins 36 + NB_TAILLE
#define NB_TAILLE 6//la taille de l'enregistrement ne vas pas depase 6 chiffres
//pour les calcules de la visualisation 
#define block_width 500.0
#define block_height 180.0
#define field_width 80.0
#define field_height 20.0
#define CELL_SPACING 85.0
#define BLOCK_PAR_LIGNE 2
#define STARTX 100
#define STARTY 300

GtkWidget *window;// la fenetre
GtkWidget *drawing_area;// zone de dessin 

int highlighted_block = -1;//par defaut 
int highlighted_record = -1;

char nom[100];
double x = STARTX;
double y = STARTY;

//Declaration des strucures de types ==================================================================================>
//Type de l'enregistrement
typedef struct Donnee Donnee;
struct Donnee{
    char numero[10]; //Le numéro on l'utilise comme cle
    char* data; //Observation: Un champ de taille variable
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

Fichier f;

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
    buf->tab[sizeof(buf->tab)] = '\0';
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
        Aff_entete(fichier,5,0);
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
    fflush(fichier->fich);//vide le tampon associé au flux de fichie
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

//Outils utilisés pour la creation 
char* ChaineAlea(int taille)
{
    char* alpha = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.-'?!"; //Une chaine qui contient tous les caractères possibles
    char* str = NULL; //La chaine aléatoire qu'on va générer
    str = (char*)malloc(sizeof(char)*(taille+2)); //Allouer de l'espace mémoire pour cette chaine
    for(int i=0;i<taille;i++)
    {
        str[i] = alpha[rand()%68]; //Choisir un caractère aléatoirement
    }
    str[taille] = '#'; //Ajouter un sépérateur
    str[taille] = '\0'; //Un caractère vide pour déterminer la fin de la chaine de caractères
    return str; //Retouner la chaine aléatoire
}

//Initialiser les enregistrements avec des séparateurs
void InitialiserDonnee(Donnee* d)
{
    strcpy(d->numero,"|||||||||");
    d->taille[0] = '0';
    for(int i=1;i<NB_TAILLE;i++)
        d->taille[i] = '#';
    d->taille[NB_TAILLE-1] = '\0';
    d->eff = '0';
}

//Concatène tous les champs de l'enregistrement en une chaine de caractères
char* ConcatDonnee(Donnee d)
{
    char* str;
    str = (char*)malloc(sizeof(char)*(37+NB_TAILLE+strlen(d.data))); //Alloue l'espace mémoire pour la chaine de caractères
    int j=0;
    strcpy(str, d.taille);
    j += strlen(d.taille);
    str[j] = '#';
    str[j + 1] = d.eff;
    j += 2;
    str[j] = '#';
    strcpy(str + j + 1, d.numero);
    j += strlen(d.numero) + 1;
    str[j] = '#';
    strcpy(str + j + 1, d.data);
    j += strlen(d.data) + 1;
    str[j] = '#';
    str[j+1] = '$';
    str[j+2] = '\0';
    return str; //Retourner la chaine conacténée
}

//Recupère le champ de l'enregistrement à partir du fichier
void RecupChamp(Fichier* fichier,int n,Buffer* buf,int* i,int* j,char* donnee)
{
    int k = 0;
    
    while(buf->tab[*j] != '#')
    {
        //printf("\nbbb %s %i\n", buf->tab, *j);
        if(*j>=sizeof(buf->tab)) //En cas où le champ est divisé sur plus d'un bloc
        {   
            (*i)+=1;
            *j = 0; //Rénitiliser le j à 0 (au début du nouveau bloc)
            LireDir(fichier,*i,buf); //Lire le bloc suivant
        }
        
        donnee[k] = buf->tab[*j];
        
        (*j)+=1;
        k+=1;
    }
    (*j)+=1;
    if(buf->tab[*j] == '$')
        (*j)+=1;
    donnee[k] = '\0';
}

//Récupère la chaine du fichier comme elle est sans enlever les séparateurs
//i bloc j pos
void RecupChaine(Fichier* fichier,int n,Buffer* buf,int* i,int* j,char donnee[])
{
    int k = 0;
    while(buf->tab[*j] != '#')
    {
        
        if(*j>=sizeof(buf->tab)) //En cas où la chaine est divisée sur plus d'un bloc
        {
            (*i)+=1;
            *j = 0; //Rénitiliser le j à 0 (au début du nouveau bloc)
            LireDir(fichier,*i,buf);
        }
        //printf("\nbbb %c\n", buf->tab[*j]);
        donnee[k] = buf->tab[*j];
        (*j)+=1;
        k+=1;
    }
    (*j)+=1;
    if(buf->tab[*j] == '$')
        (*j)+=1;
    if(buf->tab[*j] == '#')
        (*j)+=1;
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
            *j = 0; //Réinitialiser j à 0
            EcrireDir(fichier,*i,buf);
            if((*i)+1>Entete(&f,5)){
                AllocBloc(fichier); //Allouer un nouveau bloc à la fin du fichier et le chainer
                *i = Entete(fichier,5); //Mettre à jour i à l'adresse de la queue de la liste
                
            }else{

                (*i)++;
            }
            LireDir(fichier,*i,buf); //Lire le nouveau bloc
        }
        buf->tab[*j] = donnee[k]; //Mettre à jour un caractère du buffer avec un caractère de la chaine qu'on veut écrire
        (*j)+=1;
        k+=1;
    }
    donnee[k] = '\0';
    int l = *j;
    while (l<B){
        buf->tab[l] = '<';
        l++;
    }
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
    printf("\t\t\tEntrez data (sans espaces): ");
    d.data = (char*)malloc(sizeof(char)*250);
    scanf("%s",d.data); //Lire le champ observation
    sprintf(d.taille,"%d",NB_TAILLE+1+35+strlen(d.data));
    return ConcatDonnee(d); //Retouner les champs concaténés
}
static gboolean draw_file(GtkWidget *widget, cairo_t *cr, gpointer data) ;
//Operations sur LOVC ==============================================================================================================================
//Cette procédure recherche un numero dans le fichier d'après le numéro
//i: l'adresse du bloc et j: la position dans le bloc
//(i,j) représentent l'emplacement du livret s'il existe sinon où il est supposé placé
void RechercheLOVC(Fichier* fichier,char* nom_physique,int val,int* i,int* j,int* trouv, int *index)
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
    if(index)
        *index = -1;
    Ouvrir(fichier,nom_physique,'A'); //Ouvrir le fichier en mode ancien
    *trouv = 0; //Positionner trouv à Faux
    stop = 0; //Positionner stop à Faux
    *i = Entete(fichier,1);
    *j = 0;
    if(index)
    *index = 0;
    LireDir(fichier,*i,&buf); //Lire le premier bloc

    //Le cas d'un fichier vide
    if((*j>=Entete(fichier,6))&&(*i>=Entete(fichier,5)))
    {
        stop = 1; //stop à Vrai
    }
    gtk_widget_queue_draw(drawing_area);//redessin du widget GTK
    cairo_t *cr;//contexte de dessin.
    cr = gdk_cairo_create(gtk_widget_get_window(drawing_area));//obtenir un contexte de dessin Cairo 

    while((*trouv == 0)&&(stop == 0)) //Tant qu'on a pas trouvé le livret et que aucune condition n'a été vérifiée pour stopper la boucle
    {
       
        sauvi = *i; //Sauvegarder i
        sauvj = *j; //Sauvegarder j
        RecupChamp(fichier,NB_TAILLE,&buf,i,j,taille); //sa le champ taille (le i et j sont mis à jour après le champ)
        nb_taille = atoi(taille); //Récupérer le champ en tant qu'entier
        RecupChamp(fichier,1,&buf,i,j,&eff); //Récupérer le champ eff
        RecupChamp(fichier,10,&buf,i,j,numero); //savep champ numéro
        nb_numero = atoi(numero); //Récupérer le champ en tant qu'entier
        d = malloc(sizeof(char)*(nb_taille-NB_TAILLE-10));
        RecupChaine(fichier,nb_taille-NB_TAILLE-11,&buf,i,j,d); //Récupérer le reste de l'enregistrement
        

        if(nb_numero == val) //Le meme numéro a été trouvé
        {
            if(eff == '0') //il n'est pas effecé donc il a été trouvé
                *trouv = 1; //trouv à Vrai
            else
                stop = 1; //io a été supprimé donc on met stop à Vrai
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
        if(index){
            if(*i == sauvi)
                (*index)++;
            else if(*i > sauvi)
                *index = 0;
            
            highlighted_block = *i;
            highlighted_record = *index;
            draw_file(NULL, cr, NULL);  
            //for(int l = 0; l<100000000; l++);
        }
        if((*j>=Entete(fichier,6))&&(*i>=Entete(fichier,5))) //Si on arrive à la fin du fichier
        {
            stop = 1; //stop à Vrai
        }

        free(d); //On libère l'espace
    }
    cairo_destroy(cr);
    //Fermer(fichier); //Fermer le fichier
}

char * GenererContenu(char* temp, int num)
{

    //LireDir(fichier,i,&buf); //Lire le bloc  
    Donnee nouvelle_donnee; //Créer une nouvelle donnée
    InitialiserDonnee(&nouvelle_donnee); //L'initialiser
    sprintf(nouvelle_donnee.numero,"%d",num); //Générer le numéro
    nouvelle_donnee.data = strdup(temp); //Générer la donnee
    sprintf(nouvelle_donnee.taille,"%d",NB_TAILLE+1+35+strlen(nouvelle_donnee.data)); //Calculer la taille
    char* str = ConcatDonnee(nouvelle_donnee); //Concaténer tous les champs
    return str;
}

int len(char * temp){
    int i =0;
    
    while(temp[i] != '<' && temp[i] != '\0'){
        i++;
    }
    return i;
}
//procedure d'insertion
void insert(Fichier* fichier,char* nom_physique,int numero,char* s)
 {
    Buffer buf;
    int trouv;
    int stop = 0;
    int i = 1;
    int j = 0;
    int l = 0;
    int rest;

    char *tmpChar = malloc(B * (Entete(fichier, 5) + 1));
    tmpChar[0] = '\0';

    RechercheLOVC(fichier,nom_physique,numero,&i,&j,&trouv, NULL); //On effectue une recherche pour avoir l'adresse i et la position j où insérer
    if (trouv)
    {
        printf("insertion impossible,numero existant!\n");//pas d'insertion
        return;
    }

    int bloc = i;
    int charpos = j;
    int sauvi = i;
    int sauvj = j;

    int k = j;
    j = 0;
    

    while (i <= Entete(fichier, 5))
    {
        int l = k;
        LireDir(fichier, i, &buf);//lire le bloc i dans le buffer
        int ll = len(buf.tab);
        
        for ( l; l < ll; l++)
        {
            tmpChar[j++] = buf.tab[l];//sauvegarder les donnes dans la variable temporaire pour pouvoir faire l'insertion apres
        }
        i++;
        k = 0;
     }

    tmpChar[j] = '\0';
    LireDir(fichier, sauvi, &buf);//lire le bloc ou on doit inserer notre enregistrement
    j = 0;
    EcrireChaine(fichier, nom_physique, strlen(s), &buf, &sauvi, &sauvj, s);//ecrire la nouvelle donnee insere dans le bloc a sa position
    if(tmpChar[0] != '\0')//remettre les donnes qui se trouvent dans la variable temp a leurs places apres que l'insertion soit termine
        EcrireChaine(fichier, nom_physique, strlen(tmpChar), &buf, &sauvi, &sauvj, tmpChar);
    
    free(tmpChar);
    Fermer(&f);
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
        //LireDir(fichier,i,&buf); //Lire le bloc
        
        Donnee nouvelle_donnee; //Créer une nouvelle donnée
        InitialiserDonnee(&nouvelle_donnee); //L'initialiser
        sprintf(nouvelle_donnee.numero,"%d",k); //Générer le numéro
        nouvelle_donnee.data = ChaineAlea(10); //Générer l'observation

        sprintf(nouvelle_donnee.taille,"%d",NB_TAILLE+1+35+strlen(nouvelle_donnee.data)); //Calculer la taille
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
                //buf.tab[j] = '\0';
                j=0;
                EcrireDir(fichier,i,&buf); //On écrit le bloc
                strcpy(buf.tab,"\0");
                AllocBloc(fichier); //Alloue un nouveau bloc
                i=Entete(fichier,5); //Met à jour l'adresse i
                //LireDir(fichier,i,&buf); //Lire le nouveau bloc
            }
        }
        buf.tab[j] = '\0';
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
    char t[NB_TAILLE];
    char eff;
    char* d;
    char* donnee;
    char num[100];
    index = 0;
    int stop = 0;
    Ouvrir(fichier,nom_physique,'A'); //Ouvrir le fichier en mode ancien
    i = Entete(fichier,1);
    j = 0;
    while(stop == 0)
    {
        LireDir(fichier,i,&buf); //Lire le bloc
        RecupChamp(fichier,NB_TAILLE,&buf,&i,&j,t); //Récupère le champ taille
        nb_taille = atoi(t);
        donnee = malloc(sizeof(char)*(nb_taille+1));
        index = 0;
        int k = 0;
        while(t[k] != '\0')
        {
            donnee[index] = t[k];
            index++;
            k++;
        }
        RecupChamp(fichier,1,&buf,&i,&j,&eff); //Récupère le champ effacé
        RecupChamp(fichier,NB_TAILLE,&buf,&i,&j, num); //Récupère le champ taille
        d = malloc(sizeof(char)*(nb_taille-strlen(num)));
        if(!d){
            fprintf(stderr, "erreur\n");
            exit(EXIT_FAILURE);
        }
        
        RecupChaine(fichier,nb_taille-1-NB_TAILLE,&buf,&i,&j,d); //Recupère le reste de la donnée
        

        donnee[index] = '#';
        index++;
        donnee[index] = eff;
        index++;
        donnee[index] = '#';
        index++;
        k=0;
        while(num[k] != '\0')
        {
            donnee[index] = num[k];
            index++;
            k++;
        }
        donnee[index] = '#';
        index++;
        for(int k=0;k<strlen(d);k++)
        {
            donnee[index] = d[k];
            index++;
        }
        donnee[index] = '#';
        index++;
        donnee[index] = '$';
        donnee[nb_taille] = '\0';
        printf("%s\n",donnee); //Afficher l'enregistrement en entier
        if((i == Entete(fichier,5))&&(j >= Entete(fichier,6))) //Si on arrive à la fin du fichier on stoppe l'affichage
            stop = 1;
    }
    Fermer(fichier);
}



gboolean left_to_right = TRUE;
gboolean top_to_down = FALSE;
gboolean chvchmnt = FALSE;
// dessiner des blocks

static gboolean draw_block(GtkWidget *widget, cairo_t *cr, gpointer data) {
    int block_index = GPOINTER_TO_INT(data);
    Buffer current_block;
    for (int i = Entete(&f,1); i <= block_index; i++) {
        LireDir(&f, i, &current_block);
        if (i > Entete(&f,5)) {
            return FALSE;
        }  
    }


    cairo_rectangle(cr, x, y, block_width, block_height +  field_width);
    cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
    cairo_fill_preserve(cr);
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_stroke(cr);

    cairo_move_to(cr, x + 10.0, y + 20.0);

    if (block_index + 1 <= Entete(&f,5)) {
        if(!top_to_down){
            double arrowStartX = left_to_right ? x + block_width : x;
            double arrowStartY = y + block_height / 2.0;
            double arrowEndX = left_to_right ? x + block_width + CELL_SPACING : x - CELL_SPACING;
            double arrowEndY = arrowStartY;

            cairo_move_to(cr, arrowStartX, arrowStartY);
            cairo_line_to(cr, arrowEndX, arrowEndY);
            cairo_stroke(cr);

            double arrowTipX = left_to_right ? arrowEndX - 10 :  arrowEndX + 10;
            double arrowTipY1 = arrowEndY - 5;
            double arrowTipY2 = arrowEndY + 5;

            cairo_move_to(cr, arrowTipX, arrowTipY1);
            cairo_line_to(cr, arrowEndX, arrowEndY);
            cairo_line_to(cr, arrowTipX, arrowTipY2);
            cairo_fill(cr);
        }else{
            double arrowStartX = x + block_width / 2.0;
            double arrowStartY = y + block_height +  field_width;
            double arrowEndX = arrowStartX;
            double arrowEndY = y + block_height + CELL_SPACING + 50.0;

            cairo_move_to(cr, arrowStartX, arrowStartY);
            cairo_line_to(cr, arrowEndX, arrowEndY);
            cairo_stroke(cr);

            double arrowTipY = arrowEndY - 10;
            double arrowTipX1 = arrowEndX - 5;
            double arrowTipX2 = arrowEndX + 5;

            cairo_move_to(cr, arrowTipX1, arrowTipY);
            cairo_line_to(cr, arrowEndX, arrowEndY);
            cairo_line_to(cr, arrowTipX2, arrowTipY);
            cairo_fill(cr);
            top_to_down = FALSE;
        }
    }

    return FALSE;
}

static gboolean draw_enregistrement(GtkWidget *widget, cairo_t *cr, gpointer data) {

    int block_index = GPOINTER_TO_INT(data);
    Buffer current_block;
           
    if (block_index > Entete(&f,5)) {
        return FALSE;
    }

    LireDir(&f, block_index, &current_block);

    char *saveptr1=NULL;
    
    char *enregistrement = strdup(current_block.tab);
    char *token = strtok_r(enregistrement, "$", &saveptr1);
    int field_index = 1;
    Buffer tempb;
    if(block_index > 1){
        LireDir(&f, block_index - 1, &tempb);
        if (tempb.tab[strlen(tempb.tab) - 1] != '$')
        {
            field_index=0;
        }
    }
    char *eff;
    eff = strdup("0");
    while (token != NULL && token[0] != '<') {
        char *saveptr3=NULL;
        char *t = strdup(token);
        if(field_index == 0){
            strtok_r(t, "#", &saveptr3);
            strtok_r(NULL, "#", &saveptr3);
        }else{
            eff = strtok_r(t, "#", &saveptr3);
            eff = strtok_r(NULL, "#", &saveptr3);
        }
        
        if(eff[0] == '0'){
            char *saveptr2=NULL;
            double x_field = x + 120.0 + 0.3 * field_width;
            double y_field = y + 20.0 + field_index * 0.3 * field_width;
            char temp[30];
            sprintf(temp, "Enregistrement num %i :", field_index);
            cairo_move_to(cr, x + 10.0, y_field - 2.0);
            cairo_show_text(cr, temp);

            char *tmp = strdup(token);
            char *tkn = strtok_r(tmp, "#", &saveptr2);
            int i = 0;
            while(tkn ){
                double xc = x_field + i * field_width + 0.2 * sizeof(tkn);
                cairo_rectangle(cr, xc, y_field, field_width + sizeof(tkn) / 2.0, field_height);
                cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
                cairo_fill_preserve(cr);
                cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
                cairo_stroke(cr);
                
                cairo_move_to(cr, xc + 5.0, y_field + field_height - 0.3 * field_height);
                
                cairo_show_text(cr, tkn);
                    
                tkn = strtok_r(NULL, "#", &saveptr2);
                i++;
            }
            free(tmp);
            free(t);
            field_index++;
        }
        token = strtok_r(NULL, "$", &saveptr1);      
    }

    free(enregistrement);
    return FALSE;
}

void update_gui() {
    gtk_widget_queue_draw(drawing_area);
}


void on_refresh_button_clicked(GtkWidget *widget, gpointer data) {
    highlighted_block = -1;
    highlighted_record = -1;
    update_gui();
}

void on_insert_button_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *entry, *entry2;
    gint result;

    
    dialog = gtk_dialog_new_with_buttons("Insert",
        GTK_WINDOW(gtk_widget_get_toplevel(widget)),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "OK", GTK_RESPONSE_OK,
        "Annuler", GTK_RESPONSE_CANCEL,
        NULL);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "num");
    gtk_container_add(GTK_CONTAINER(content_area), entry);
    gtk_widget_show_all(dialog);
    entry2 = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry2), "donnee");
    gtk_container_add(GTK_CONTAINER(content_area), entry2);
    gtk_widget_show_all(dialog);

    result = gtk_dialog_run(GTK_DIALOG(dialog));

    if (result == GTK_RESPONSE_OK) {
        const char *tmp = gtk_entry_get_text(GTK_ENTRY(entry));
        const char *tmp2 = gtk_entry_get_text(GTK_ENTRY(entry2));
        int nb = atoi(tmp);
        
        insert(&f, nom, nb, GenererContenu(tmp2, nb));
        Ouvrir(&f, nom, 'A');
        on_refresh_button_clicked(NULL, NULL);
    }
    gtk_widget_destroy(dialog);
}


void on_delete_button_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *entry;
    gint result;

    bool trouv = FALSE;
    
    dialog = gtk_dialog_new_with_buttons("Supprimer un élément",
        GTK_WINDOW(gtk_widget_get_toplevel(widget)),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "OK", GTK_RESPONSE_OK,
        "Annuler", GTK_RESPONSE_CANCEL,
        NULL);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Clef");
    gtk_container_add(GTK_CONTAINER(content_area), entry);
    gtk_widget_show_all(dialog);

    result = gtk_dialog_run(GTK_DIALOG(dialog));

    if (result == GTK_RESPONSE_OK) {}
    on_refresh_button_clicked(NULL, NULL);
    gtk_widget_destroy(dialog);
}

void on_creat_button_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *entry;
    GtkWidget *entry1;
    gint result;

    
    dialog = gtk_dialog_new_with_buttons("le nombre d'enregistrement",
        GTK_WINDOW(gtk_widget_get_toplevel(widget)),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "OK", GTK_RESPONSE_OK,
        "Annuler", GTK_RESPONSE_CANCEL,
        NULL);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    entry1 = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry1), "Nom fichier");
    gtk_container_add(GTK_CONTAINER(content_area), entry1);

    entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "le nombre");
    gtk_container_add(GTK_CONTAINER(content_area), entry);
    
    gtk_widget_show_all(dialog);

    result = gtk_dialog_run(GTK_DIALOG(dialog));

    if(result == GTK_RESPONSE_OK) {
        const char *tmp = gtk_entry_get_text(GTK_ENTRY(entry));
        int nb = atoi(tmp);
        const char *nm = gtk_entry_get_text(GTK_ENTRY(entry1));;
        if(nm){
            Fermer(&f);
            strcpy(nom, nm);
        }
        if(nb >= 0)
            GenererContenuAlea(&f, nom, nb);
        else
            GenererContenuAlea(&f, nom, 0);
        Ouvrir(&f, nom, 'A');
        update_gui();
    }
    on_refresh_button_clicked(NULL, NULL);
    gtk_widget_destroy(dialog);
}
void calculate_block_position(int block_index, double *x, double *y){
    int j = 1;
    int tmp;
    *x = STARTX;
    *y = STARTY;
    //printf("\n 2ww %f %f \n", *x, *y);
    left_to_right = TRUE;
    top_to_down = FALSE;
    for (int i = Entete(&f, 1); i < block_index; i++) {
        tmp = (j % BLOCK_PAR_LIGNE);
        if ( tmp == BLOCK_PAR_LIGNE - 1)
            top_to_down = TRUE;
        else
            top_to_down = FALSE;

        if (left_to_right) {
            *x += block_width + CELL_SPACING;
            if (tmp == 0){
                *x -= block_width + CELL_SPACING;
                *y += block_height + CELL_SPACING + 50.0;
                left_to_right = FALSE;
            }
        } else {
            *x -= block_width + CELL_SPACING;
            if (tmp == 0) {
                *x = CELL_SPACING;
                *y += block_height + CELL_SPACING + 50.0;
                left_to_right = TRUE;
            }
        }
        j++;
    }
     //printf("\n ww %f %f \n", *x, *y);
    left_to_right = TRUE;
    
}

static void draw_highlighted_block(cairo_t *cr, int block_index) {
    
   

    //calculate_block_position(block_index, &x, &y);
    //g_print("\n %lf %lf\n", x, x);
    //printf("\n %f %f \n", x, y);
    cairo_rectangle(cr, x, y, block_width , block_height +  field_width );
    cairo_set_source_rgb(cr, 0.35, 0.35, 0.5);
    cairo_fill_preserve(cr);
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_stroke(cr);

    cairo_move_to(cr, x + 10.0, y + 20.0);
    

    if (block_index + 1 <= Entete(&f,5)) {
        if(!top_to_down){
            double arrowStartX = left_to_right ? x + block_width : x;
            double arrowStartY = y + block_height / 2.0;
            double arrowEndX = left_to_right ? x + block_width + CELL_SPACING : x - CELL_SPACING;
            double arrowEndY = arrowStartY;

            cairo_move_to(cr, arrowStartX, arrowStartY);
            cairo_line_to(cr, arrowEndX, arrowEndY);
            cairo_stroke(cr);

            double arrowTipX = left_to_right ? arrowEndX - 10 :  arrowEndX + 10;
            double arrowTipY1 = arrowEndY - 5;
            double arrowTipY2 = arrowEndY + 5;

            cairo_move_to(cr, arrowTipX, arrowTipY1);
            cairo_line_to(cr, arrowEndX, arrowEndY);
            cairo_line_to(cr, arrowTipX, arrowTipY2);
            cairo_fill(cr);
        }else{
            double arrowStartX = x + block_width / 2.0;
            double arrowStartY = y + block_height +  field_width;
            double arrowEndX = arrowStartX;
            double arrowEndY = y + block_height + CELL_SPACING + 50.0;

            cairo_move_to(cr, arrowStartX, arrowStartY);
            cairo_line_to(cr, arrowEndX, arrowEndY);
            cairo_stroke(cr);

            double arrowTipY = arrowEndY - 10;
            double arrowTipX1 = arrowEndX - 5;
            double arrowTipX2 = arrowEndX + 5;

            cairo_move_to(cr, arrowTipX1, arrowTipY);
            cairo_line_to(cr, arrowEndX, arrowEndY);
            cairo_line_to(cr, arrowTipX2, arrowTipY);
            cairo_fill(cr);
            top_to_down = FALSE;
        }
    }
    
}

static void draw_highlighted_record(cairo_t *cr, int block_index, int record_index) {
    
    Buffer current_block;
    
    for (int i = Entete(&f,1); i <= block_index; i++) {
        LireDir(&f, i, &current_block);
        if (i > Entete(&f,5)) {
            return /*FALSE */;
        }
        
    }

    

   
    //calculate_block_position(block_index, &x, &y);

    
char *saveptr1=NULL;
    
    char *enregistrement = strdup(current_block.tab);
    char *token = strtok_r(enregistrement, "$", &saveptr1);
    int field_index = 1;
    Buffer tempb;
    if(block_index > 1){
        LireDir(&f, block_index - 1, &tempb);
        if (tempb.tab[strlen(tempb.tab) - 1] != '$')
        {
            field_index=0;
        }
    }
    char *eff;
    eff = strdup("0");
    while (token != NULL && token[0] != '<') {
        char *saveptr3=NULL;
        char *t = strdup(token);
        if(field_index == 0){
            strtok_r(t, "#", &saveptr3);
            strtok_r(NULL, "#", &saveptr3);
        }else{
            eff = strtok_r(t, "#", &saveptr3);
            eff = strtok_r(NULL, "#", &saveptr3);
        }
        
        if(eff[0] == '0'){
            char *saveptr2=NULL;
            double x_field = x + 120.0 + 0.3 * field_width;
            double y_field = y + 20.0 + field_index * 0.3 * field_width;
            char temp[30];
            sprintf(temp, "Enregistrement num %i :", field_index);
            cairo_move_to(cr, x + 10.0, y_field - 2.0);
            cairo_show_text(cr, temp);

            char *tmp = strdup(token);
            char *tkn = strtok_r(tmp, "#", &saveptr2);
            int i = 0;
            while(tkn){
                double xc = x_field + i * field_width + 0.2 * sizeof(tkn);
                
                if(field_index == record_index)
                    cairo_set_source_rgb(cr, 1.0, 0, 0);
                else{
                    cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
                }
                cairo_rectangle(cr, xc, y_field, field_width + sizeof(tkn) / 2.0, field_height);
                cairo_fill_preserve(cr);
                cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
                cairo_stroke(cr);
                
                cairo_move_to(cr, xc + 5.0, y_field + field_height - 0.3 * field_height);
                
                cairo_show_text(cr, tkn);
                    
                tkn = strtok_r(NULL, "#", &saveptr2);
                i++;
            }
            free(tmp);
        }
        field_index++;
        token = strtok_r(NULL, "$", &saveptr1);
    }

    free(enregistrement);
}

void highlight_block_and_record(int block_index, int record_index) {
    
    gtk_widget_queue_draw(drawing_area);
    cairo_t *cr;
    cr = gdk_cairo_create(gtk_widget_get_window(drawing_area));
    //GdkWindow *window = gtk_widget_get_window(drawing_area);
    //GdkDrawingContext *context = gdk_window_begin_draw_frame(window, NULL);
    //cr = gdk_drawing_context_get_cairo_context(context);
    //g_print("\n\n %i %i \n", block_index, record_index);
    draw_highlighted_block(cr, block_index);
    draw_highlighted_record(cr, block_index, record_index);
    
    
    cairo_destroy(cr);
}


int nb_enr(char *tmp, int indx){
    char *save;
    char *div;
    int i = 0;
    div = strtok_r(tmp, "$", &save);
    while(div){
        i++;
        div = strtok_r(NULL, "$", &save);
    }
    Buffer tempb;
    LireDir(&f, indx - 1, &tempb);
    if ((indx > 1) && tempb.tab[strlen(tempb.tab) - 1] != '$')
    {
        i--;
    }
    return i;
}
static gboolean draw_file(GtkWidget *widget, cairo_t *cr, gpointer data) ;
void on_search_button_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *entry;
    gint result;

    bool trouv = FALSE;
    
    dialog = gtk_dialog_new_with_buttons("Rechercher un élément",
        GTK_WINDOW(gtk_widget_get_toplevel(widget)),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "OK", GTK_RESPONSE_OK,
        "Annuler", GTK_RESPONSE_CANCEL,
        NULL);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Clef");
    gtk_container_add(GTK_CONTAINER(content_area), entry);
    gtk_widget_show_all(dialog);

    result = gtk_dialog_run(GTK_DIALOG(dialog));

    if (result == GTK_RESPONSE_OK) {
        
        const char *key = gtk_entry_get_text(GTK_ENTRY(entry));
        char *tmp = strdup(key);
        int tmpj;
        RechercheLOVC(&f, nom, atoi(tmp), &highlighted_block, &tmpj, &trouv, &highlighted_record);
        g_print("\n%i %i\n",highlighted_block,highlighted_record);
        free(tmp);
        Buffer tempb;
        LireDir(&f, highlighted_block, &tempb);
        bool ch = tempb.tab[strlen(tempb.tab) - 1] != '$';
        int k=highlighted_block;
        int nb_enrg = nb_enr(tempb.tab, highlighted_block);
        
        if (trouv) {
            //highlight_block_and_record(highlighted_block, highlighted_record);
            int j = 1;
            //chvchmnt = ch && (highlighted_record == nb_enrg-1);
            //if (chvchmnt)
            //{
            //    do{
            //        highlight_block_and_record(highlighted_block + j, -1);
            //        LireDir(&f, highlighted_block + j, &tempb);
            //        j++;
            //        k++;
            //    }while( nb_enr(tempb.tab) == 1 && tempb.tab[strlen(tempb.tab) - 1] != '$' && k <= Entete(&f, 4));
             
            //}
                gtk_widget_queue_draw(drawing_area);
                cairo_t *cr;
                cr = gdk_cairo_create(gtk_widget_get_window(drawing_area));
                draw_file(NULL, cr, NULL);
                cairo_destroy(cr);
        } else {
            on_refresh_button_clicked(NULL, NULL);
            GtkWidget *info_dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                             GTK_DIALOG_MODAL,
                                                             GTK_MESSAGE_INFO,
                                                             GTK_BUTTONS_OK,
                                                             "Enregistrement non trouve");
            gtk_dialog_run(GTK_DIALOG(info_dialog));
            gtk_widget_destroy(info_dialog);
        }
    }

    gtk_widget_destroy(dialog);
}

static void on_size_allocate(GtkWidget *widget, GdkRectangle *allocation, gpointer data) {
    update_gui();
}

static void on_scrolled(GtkAdjustment *adjustment, gpointer data) {
    update_gui();
}

static gboolean on_focus_in_event(GtkWidget *widget, GdkEventFocus *event, gpointer user_data) {
    update_gui();
    return FALSE;
}

static gboolean on_focus_out_event(GtkWidget *widget, GdkEventFocus *event, gpointer user_data) {
    update_gui();
    return FALSE;
}

static double calculate_required_height(int num_blocks) {
    int rows = (num_blocks + BLOCK_PAR_LIGNE - 1) / BLOCK_PAR_LIGNE;
    return rows * (block_height + CELL_SPACING + 50) + STARTY;
}

static double calculate_required_widht(int num_blocks) {
    return BLOCK_PAR_LIGNE * block_width + (BLOCK_PAR_LIGNE - 1) * CELL_SPACING + STARTX;
}

static gboolean draw_file(GtkWidget *widget, cairo_t *cr, gpointer data) {

    double x_f = STARTX, y_f = 20.0;
    cairo_rectangle(cr, x_f, y_f, block_width, block_height);
    cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
    cairo_fill_preserve(cr);
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_stroke(cr);

    cairo_move_to(cr, x_f + 10.0, y_f + 20.0);
    cairo_show_text(cr, "Fichier Headers :");
    cairo_move_to(cr, x_f + 10.0, y_f + 60.0);
    char tmp[30];
    sprintf(tmp, "Nombre blocks : %i", Entete(&f, 4));
    cairo_show_text(cr, tmp);
    cairo_move_to(cr, x_f + 10.0, y_f + 80.0);
    //sprintf(tmp, "nb insert : %i", Entete(&f, 2));
    //cairo_show_text(cr, tmp);

    if (Entete(&f, 4)) {
        double arrowStartX = x_f + block_width / 2.0;
        double arrowStartY = y_f + block_height;
        double arrowEndX = arrowStartX;
        double arrowEndY = STARTY;

        cairo_move_to(cr, arrowStartX, arrowStartY);
        cairo_line_to(cr, arrowEndX, arrowEndY);
        cairo_stroke(cr);

        double arrowTipY = arrowEndY - 10;
        double arrowTipX1 = arrowEndX - 5;
        double arrowTipX2 = arrowEndX + 5;

        cairo_move_to(cr, arrowTipX1, arrowTipY);
        cairo_line_to(cr, arrowEndX, arrowEndY);
        cairo_line_to(cr, arrowTipX2, arrowTipY);
        cairo_fill(cr);
    }

    x = STARTX;
    y = STARTY;
    left_to_right = TRUE;
    top_to_down = FALSE;

    int num_blocks = Entete(&f, 4);

    double required_height = calculate_required_height(num_blocks) + 100;
    double current_widht;
    double current_height;
    gtk_widget_get_size_request(drawing_area, &current_widht, &current_height);
    if (required_height > current_height) {
        gtk_widget_set_size_request(drawing_area, -1, required_height);
    }

    double required_widht = calculate_required_widht(num_blocks) + 100;

    if (required_widht > current_widht) {
        gtk_widget_set_size_request(drawing_area, -1, required_height);
    }

    int j = 0;
    Buffer tempb;
    for (int i = Entete(&f,1); i <= Entete(&f,5) && Entete(&f,5); i++) {
        
    
        if (j % BLOCK_PAR_LIGNE == BLOCK_PAR_LIGNE - 1)
            top_to_down = TRUE;
        if(i != highlighted_block && !chvchmnt){
            draw_block(widget, cr, GINT_TO_POINTER(i));
            draw_enregistrement(widget, cr, GINT_TO_POINTER(i));
        }else{
            if(!chvchmnt)
                highlight_block_and_record(i, highlighted_record);
            else
                highlight_block_and_record(i, 0);
            if(i == highlighted_block){
                LireDir(&f, i, &tempb);
                chvchmnt = tempb.tab[strlen(tempb.tab)-1] != '$' && (highlighted_record == nb_enr(tempb.tab, i));
            }
            else{
                LireDir(&f, i, &tempb);
                chvchmnt = tempb.tab[strlen(tempb.tab)-1] != '$' && nb_enr(tempb.tab, i) == 1;
            }
        }
        if (left_to_right) {
            x += block_width + CELL_SPACING;
            if (j % BLOCK_PAR_LIGNE == BLOCK_PAR_LIGNE - 1){
                x -= block_width + CELL_SPACING;
                y += block_height + CELL_SPACING + 50.0;
                left_to_right = FALSE;
            }
        } else {
            x -= block_width + CELL_SPACING;
            if (j % BLOCK_PAR_LIGNE == BLOCK_PAR_LIGNE - 1) {
                x = CELL_SPACING;
                y += block_height + CELL_SPACING + 50.0;
                left_to_right = TRUE;
            }
        }
        j++;
    }

    return FALSE;
}

int main(int argc, char* argv[])
{
    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "File Visualization");
    gtk_window_set_default_size(GTK_WINDOW(window), 1500, 1000);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);


    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, 1500, 1000);

    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled_window), drawing_area);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

    g_signal_connect(drawing_area, "draw", G_CALLBACK(draw_file), NULL);
    g_signal_connect(drawing_area, "size-allocate", G_CALLBACK(on_size_allocate), NULL);

    GtkAdjustment *vadjustment;
    vadjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scrolled_window));
    g_signal_connect(vadjustment, "value-changed", G_CALLBACK(on_scrolled), NULL);

    GtkAdjustment *hadjustment;
    hadjustment = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(scrolled_window));
    g_signal_connect(hadjustment, "value-changed", G_CALLBACK(on_scrolled), NULL);

    g_signal_connect(window, "focus-in-event", G_CALLBACK(on_focus_in_event), NULL);
    g_signal_connect(window, "focus-out-event", G_CALLBACK(on_focus_out_event), NULL);


    GtkWidget *insert_button = gtk_button_new_with_label("Inserer");
    GtkWidget *delete_button = gtk_button_new_with_label("Supprimer");
    GtkWidget *search_button = gtk_button_new_with_label("Rechercher");
    GtkWidget *creat_button = gtk_button_new_with_label("Creation");
    GtkWidget *refresh_button = gtk_button_new_with_label("Refresh");

    g_signal_connect(insert_button, "clicked", G_CALLBACK(on_insert_button_clicked), NULL);
    g_signal_connect(delete_button, "clicked", G_CALLBACK(on_delete_button_clicked), NULL);
    g_signal_connect(search_button, "clicked", G_CALLBACK(on_search_button_clicked), NULL);
    g_signal_connect(creat_button, "clicked", G_CALLBACK(on_creat_button_clicked), NULL);
    g_signal_connect(refresh_button, "clicked", G_CALLBACK(on_refresh_button_clicked), NULL);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(hbox), insert_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), delete_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), search_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), creat_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), refresh_button, TRUE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
    strcpy(nom, "test");
    GenererContenuAlea(&f, nom, 0);
    Ouvrir(&f, nom, 'A');
    update_gui();

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}