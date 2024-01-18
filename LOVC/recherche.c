#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include <gtk/gtk.h>
#include <time.h>


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