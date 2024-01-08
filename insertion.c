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

bool* enteteblock(fichier f,int i,int num)
{
    int cpt=1;
    block *x=Entete(f,1);//l'adresse de debut
    //chercher la bonne position de bloc
    while (cpt!=i && (x)!=NULL)
    {
        cpt++;
        x=x->svt;
    }
    switch (num)
    {
    case 0:
        return &x->chevauchement;
        break;
    case 1:
        return x->suppresion;
        break;
    default:
        return false;
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
   char buffer[1000];

recherche(c,&trouv,&i,&j,f);// chercher la cle ou inserer et recuperer le bloc i et la position j


if (trouv==false)//la cle n'as pas ete trouve
{
    int taille_eng=strlen(info);//enregistrer la taille de l'eng
    ajout_entete(f,4,taille_eng);

     if (entete(f,0)==0){
            ajout_entete(f,1,1);// mise a jour du nb de blocs
            ajout_entete(f,4,taille_eng);// mise a jour de l'espace occupe
            ajout_entete(f,5,(f.taille_block-taille_eng));//mise a jour de l'espace restant
            int x = allocblock(&f);
            ecrireblock(f,x,info);
            
    }



    char* temp_info= (char*)malloc((taille_eng)*sizeof(char));
   

    while (stop==false)//boucle d'insertion
    {
       
       lireblock(f,i,&buffer);
       if (taille_eng <= Enteteblock(f,i,4))//si la taille de l'espace restant est suffisante pour l'insertion
       {
       for (int l = 0; l < taille_eng; l++)//boucle de decalage jusqu'au dernier bloc
       {
        temp_info[l]=buffer[j+l];
        buffer[j+l]=info[l];
       }
       temp_info[taille_eng]="$";
       ecrireblock(f,i,buffer);
       j+=taille_eng;
       strcpy(info,temp_info);

       }else{//si l'espace n'est pas suffisant pour inserer l'eng
            reste=j+taille_eng-f.taille_block;
            k=0;
            while (j<f.taille_block)
            {
                temp_info[k]=buffer[j];
                buffer[j]=info[k];
                j++;
                k++;
            }
            ecrireblock(f,i,buffer);//ecrire dans le bloc
            i=f.fin->svt;//on avance vers le prochain bloc
            j=0;
            if (/* condition */)
            {
                /* code */
            }
             lireblock(f,i,&buffer);
            for (int k = 0; k < reste; k++)
            {
                  temp_info[l]=buffer[j];
                buffer[j]=info[l];
                j++;
                l++;
            }
             temp_info[strlen(info)]="$";
             ecrireblock(f,i,buffer);
            strcpy(info,temp_info);  
       }

    if (i==Entete(f,3) && j==f.taille_block-1)//si on arrive au dernier bloc et a la derniere position
    {
        stop==true;//on arrete l'insertion
    }

    }
    free(temp_info);
    


}else{
    if (enteteblock(f,i,4)==true)//si le champs supprime de la cle ou on veut inserer est egal a vrai donc la donnne a ete supp logiquement
    {
     ajout_Entete(f,i,1);//rendre le champs supprime a faux (restorer)
    }else{
    printf("insertion impossible,cle deja existante!");// si la cle existe deja 
    }
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
