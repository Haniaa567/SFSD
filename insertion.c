#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>


typedef struct block
{
    char enregistrement[200];
    int nb_enr;
    bool suppresion[100];//pour savoir le numero des enregistrement supprimmer
    bool chevauchement;//si il y a un chevauchement dans le block
    int res;//espace restant
    int ocup;//espace occupee
    struct block* svt;
}block;

typedef struct 
{
    int nb_block;
    int taille_block;
    bool supp_logique;
    block *debut;
    block *fin;
}fichier;

void insertion(char c[],fichier f)
{
   bool trouv;
   int i,j,k; 

recherche(c[],&trouv,&i,&j,f);

if (entete(f,0)==0)
{
   int x = allocblock(&f);
   ecrireblock(f,i,c[]);
}else 
if (trouv==false)
{
    if f.fin->res!=0
}






}