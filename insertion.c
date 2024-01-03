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

void ajout_entete(fichier f,int i,int val)
{
switch (i)
{
case 1:{
    f.nb_block=val;
    break;
}
case 2:{
    f.taille_block=val;
    break;
}
case 3:{
    f.debut->nb_enr=val;
    break;
}
case 4:{
    f.debut->ocup=val;
    break;
}
case 5:{
    f.debut->res=val;
    break;
}
default:
    break;
}

}
void insertion(char c[],char info[],fichier f)
{
   bool trouv,stop;
   int i,j,k; 
   char buffer[200];

recherche(c,&trouv,&i,&j,f);


if (trouv==false)
{
    int taille_eng=strlen(info);

    char* temp_info= (char*)malloc((taille_eng)*sizeof(char));
    ajout_entete(f,,)

    while (stop==false)
    {
        if (entete(f,0)==0){
        int x = allocblock(&f);
        ecrireblock(f,x,info);
        stop=true;
    }
       lireblock(f,i,buffer);
       if (j+taille_eng <= f.fin->res)
       {
       ecrireblock(f,i,buffer);
{
       }else{

       }
       





    
    }
    


}else{
    printf("insertion impossible,cle deja existante!");
}



}
}