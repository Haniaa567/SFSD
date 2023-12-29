#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>


typedef struct 
{
    char enregistrement[200];
    int nb_enr;
    bool suppresion[100];//pour savoir le numero des enregistrement supprimmer
    bool chevauchement;//si il y a un chevauchement dans le block
    block *svt;
}block;
typedef struct 
{
    int nb_block;
    int taille_block;
    bool supp_logique;
    block *debut;
    block *fin;
}fichier;
int entete(fichier F,int i)
{
    switch (i)
    {
    case 0:
        return F.nb_block;
        break;
    case 2:
        return F.taille_block;
        break;      
    
    default:
        break;
    }
}

block* Entete(fichier F, int i)
{
    switch (i)
    {
    case 1:
        return F.debut;
        break;
    case 3:
        return F.fin;
        break; 
    default:
        break;
    }
    
} 
void lireblock(fichier f,int i,char buffer[200])
{
    int cpt=1;
    block *n_block=Entete(f,1);
    while (cpt!=i && (n_block)!=NULL)
    {
        n_block=n_block->svt;
    }
    if ((n_block)!=NULL)
    {
        strcpy(buffer,(n_block)->enregistrement);
    }
    else
    {
        strcpy(buffer,"le block n'existe pas");
    }
    
    

}


//en entrée la clé (c) à chercher et le fichier.
//en sortie le booleen Trouv, le numéro de bloc (i) contenant la clé et le déplacement (j)
void recherche(char c[],bool *trouv,int *i,int *j ,fichier f)
{
    //c est l'element rechercher
    //i le num de block ou devrait ce trouver c
    //j la position de c dans le block
    int *bi;//borne inferieur
    int bs;//borne sup
    bool stop;
    int nb_block=entete(f,0);
    int sizeblock=entete(f,2);
    stop=false;
    *trouv=false;
    (*bi)=entete(f,1);
    char buffer[200];
    char *strtoken1;
    char *strtoken2;
    *j=0;
    *i=1;
    while (!(*trouv) && !(stop) && *i<=nb_block)
    {
        lireblock(f,*i,buffer);
        strtoken1=strtok(buffer, "$");//$ est le separateur d'enregistrement
        while ( strtoken1 != NULL ) {
            strtoken2=strtok(buffer, "#");//# est le separateur d'atribut
            if (strcmp(strtoken2,c)==0)//la cle se trouve dans le premier champs
            {
                trouv=true;
            }
            strtoken1 = strtok ( NULL, "$" );//recuperer le prochain enregistrement
        }
        (*i)++;
    }
        

    



}