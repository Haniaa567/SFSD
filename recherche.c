#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>


typedef struct 
{
    char enregistrement[200];
    int nb_enr;
    bool suppresion[100];//pour savoir le numero des enregistrement supprimmer
    bool chevauchement;//si il y a un chevauchement dans le block
}block;
typedef struct 
{
    int nb_block;
    int taille_block;
    bool supp_logique;
    block* debut;
}fichier;
int entete(fichier F,int i)
{
    switch (i)
    {
    case 0:
        return F.nb_block;
        break;
    case 1:
        return F.debut;
        break;
    case 3:
        return F.taille_block;
        break;    
    default:
        break;
    }
}


//en entrée la clé (c) à chercher et le fichier.
//en sortie le booleen Trouv, le numéro de bloc (i) contenant la clé et le déplacement (j)
void recherche(char c[],bool *trouv,int *i,int *j ,fichier f)
{
    //c est l'element rechercher
    //i le num de block ou devrait ce trouver c
    //j la position de c dans le block
    int bi;//borne inferieur
    int bs;//borne sup
    bool stop;
    stop=false;
    *trouv=false;
    while (!(*trouv) || !(stop))
    {
        /* code */
    }
    



}