#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include <gtk/gtk.h>
#include <time.h>

#define B 100 //Doit etre au moins 36 + NB_TAILLE
#define NB_TAILLE 6

#define block_width 500.0
#define block_height 180.0
#define field_width 80.0
#define field_height 20.0
#define CELL_SPACING 85.0
#define BLOCK_PAR_LIGNE 4
#define STARTX 100
#define STARTY 300
GtkWidget *window;
GtkWidget *drawing_area;

//Declaration des strucures de types ==================================================================================>
//Type de l'enregistrement
typedef struct Donnee Donnee;
struct Donnee{
    char numero[100]; //Le numéro du livret sur 10 caratères
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
        //printf("\nbbb %s\n", buf->tab);
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

void highlight_block_and_record(int block_index, int record_index) ;

//Operations sur LOVC ==============================================================================================================================
//Cette procédure recherche un livret dans le fichier d'après le numéro
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
    LireDir(fichier,*i,&buf); //Lire le premier bloc

    //Le cas d'un fichier vide
    if((*j>=Entete(fichier,6))&&(*i>=Entete(fichier,5)))
    {
        stop = 1; //stop à Vrai
    }

    while((*trouv == 0)&&(stop == 0)) //Tant qu'on a pas trouvé le livret et que aucune condition n'a été vérifiée pour stopper la boucle
    {
        if(index){
            (*index)++;
            highlight_block_and_record(*i, *index);
            for(int l = 0; l<100000000; l++);
        }
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
    //Fermer(fichier); //Fermer le fichier

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

Fichier f;
char nom[100];
double x = STARTX;
double y = STARTY;
int highlighted_block = -1;
int highlighted_record = -1;
gboolean left_to_right = TRUE;
gboolean top_to_down = FALSE;
gboolean chvchmnt = FALSE;


static gboolean draw_block(GtkWidget *widget, cairo_t *cr, gpointer data) {
    int block_index = GPOINTER_TO_INT(data);
    Buffer current_block;
    for (int i = Entete(&f,1); i <= block_index; i++) {
        LireDir(&f, i, &current_block);
        if (i > Entete(&f,5)) {
            return FALSE;
        }  
    }
    int i = block_index, j = 0;

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

}
