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
        return -1;
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
        return NULL;
        break;
    } 
} 
bool* enteteblock(fichier f,int i,int num)
{
    int cpt=1;
    block *x=Entete(f,1);
    while (cpt!=i && (x)!=NULL)
    {
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
        return -1;
        break;
    }

}
int Enteteblock(block x,int i)
{
    if (i==3)
    {
        return x.nb_enr;
    }
    return 0;
    
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
    char buffer2[200];
    bool stop;
    int nb_block=entete(f,0);
    int sizeblock=entete(f,2);
    stop=false;
    *trouv=false;
    char buffer[200];
    char *strtoken1;
    char *strtoken2;
    char enregistremet[200];
    bool *chevauchement;
    *j=0;
    *i=1;
    while (!(*trouv) && !(stop) && *i<=nb_block)
    {
        lireblock(f,*i,buffer);
        strtoken1=strtok(buffer, "$");//$ est le separateur d'enregistrement
        while ( strtoken1 != NULL ) {
            strtoken2=strtok(buffer, "#");//# est le separateur d'atribut
            strcpy(enregistremet,strtoken1);
            if (strcmp(strtoken2,c)==0)//la cle se trouve dans le premier champs
            {
                (*trouv)=true;
            }
            if (strcmp(strtoken2,c)>0)
            {
                (stop)=true;
                (*j)--;
            }
            
            (*j)++;
            strtoken1 = strtok ( NULL, "$" );//recuperer le prochain enregistrement 
            
            }
            (*i)++;
            
            chevauchement=enteteblock(f,*i,0);//le cas ou il y a un chevauchement dans le block
            if (strtoken1==NULL && !trouv && *chevauchement)
            {
                lireblock(f,*(i),buffer2);
                strtoken1=strtok(buffer2, "$");//$ est le separateur d'enregistrement
                strcat(enregistremet,buffer2);
                strtoken2=strtok(enregistremet, "#");
                if (strcmp(strtoken2,c)==0)//la cle se trouve dans le premier champs
                {
                    (*trouv)=true;
                }   
                if (strcmp(strtoken2,c)>0)
                {
                    stop=true;
                    (*j)--;
                } 
            }   
            
    }
}
int main()
{
    
}