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
    int nb_attribut;
    block *debut;
    block *fin;
}fichier;

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
        return NULL;
        break;
    } 
} 



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

int Enteteblock(fichier f,int i,int num)
{
    int cpt=1;
    block *x=Entete(f,1);//l'adresse de debut
    //chercher la bonne position de bloc
    while (cpt!=i && (x)!=NULL)
    {
        cpt++;
        x=x->svt;
    }
    if (num==3)
    {
        return x->nb_enr;
    }
    if (num==4)
    {
        return x->res;
    }

    
    
    return 0;
    
}
}
void insertion(char c[],char info[],fichier f)
{
   bool trouv,stop;
   int i,j,k,l,reste; 
   char buffer[];

recherche(c,&trouv,&i,&j,f);

 if (entete(f,0)==0){
            ajout_entete(f,1,1);
            int x = allocblock(&f);
            ecrireblock(f,x,info);
            
    }

if (trouv==false)
{
    int taille_eng=strlen(info);

    char* temp_info= (char*)malloc((taille_eng)*sizeof(char));
   

    while (stop==false)
    {
       
       lireblock(f,i,&buffer);
       if (taille_eng <= Enteteblock(f,i,4))
       {
       for (int l = 0; l < taille_eng; l++)
       {
        temp_info[l]=buffer[j+l];
        buffer[j+l]=info[l];
       }
       temp_info[taille_eng]="$";
       ecrireblock(f,i,buffer);
       j+=taille_eng;
       strcpy(info,temp_info);

       }else{
            reste=j+taille_eng-f.taille_block;
            k=0;
            while (j<f.taille_block)
            {
                temp_info[k]=buffer[k];
                buffer[j]=info[l];
                j++;
                k++;
            }
            ecrireblock(f,i,buffer);
            i=f.fin->svt;
            j=0;
            if ()
            {
                /* code */
            }
            lireblock(f,i,&buffer);
            
       }
       


    if (i==Entete(f,3) && j==f.taille_block-1)
    {
        stop==true;
    }
    


    
    }
    


}else{
    printf("insertion impossible,cle deja existante!");
}



}
int main()
{
    fichier f;
     
    f.nb_block=0;
    f.taille_block=10;
    f.supp_logique=false;
    f.debut=NULL;
    f.fin=NULL;
    bool *test;
    block *debut=f.debut;
    insertion("2","12#123#$24#$56",f);
    insertion("3","12#123#$",f);
    
    return 0;
}
