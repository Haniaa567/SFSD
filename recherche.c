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

int allocblock(fichier *f)
{
    //retourn le numero de block allouee
    block *bloc=malloc(sizeof(block));
    if (bloc == NULL)
    {
        fprintf(stderr, "Erreur d'allocation de mémoire pour le bloc.\n");
        exit(EXIT_FAILURE);
    }
    (bloc)->nb_enr=0;//normalemet on utilise la fonction affecter en tete block
    bloc->chevauchement=false;
    bloc->svt=NULL;
    bloc->res=f->taille_block;
    bloc->ocup=0;
    int cpt=1;
    if (f->nb_block!=0)
    {
        block *x=Entete(*f,1);//l'adresse de debut
        while ((x)->svt!=NULL)
        {
            cpt++;
            x=x->svt;
        }
        cpt++;
        x->svt=bloc;
    }
    else
    {
        f->debut = bloc;
    }
    f->fin=bloc;//normalemet on utilise la fonction affecter en tete
    f->nb_block++;
    return cpt;
}

bool* enteteblock(fichier f,int i,int num)
{
    int cpt=1;
    block *x=Entete(f,1);//l'adresse de debut
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

int Enteteblock(block x,int i)
{
    if (i==3)
    {
        return x.nb_enr;
    }
    return 0;
    
}

int compterOccurrences(char chaine[], char caractere) {
    int occurrences = 0;

    // Parcourir la chaîne caractère par caractère
    for (int i = 0; chaine[i] != '\0'; ++i) {
        // Vérifier si le caractère courant est égal au caractère recherché
        if (chaine[i] == caractere) {
            occurrences++;
        }
    }

    return occurrences;
}

void lireblock(fichier f,int i,char buffer[200])
{
    int cpt=1;
    block *n_block=Entete(f,1);
    while (cpt!=i && (n_block)!=NULL)
    {
        cpt++;
        n_block=n_block->svt;
    }
    if ((n_block)!=NULL)
    {
        strcpy(buffer,(n_block)->enregistrement);
    }
    else
    {
        strcpy(buffer,"le block n'existe pas");
        return;
    }
    
    

}

void ecrireblock(fichier f,int i,char buffer[],bool chevauchement)
{
    int cpt=1;
    int occ=compterOccurrences(buffer,'$');
    block *x=Entete(f,1);//l'adresse de debut
    block *prd;
    while (cpt!=i && (x)!=NULL)
    {
        cpt++;
        prd=x;
        x=x->svt;
    }
    if (x!=NULL)
    {
        strcpy(x->enregistrement,buffer);
        x->ocup=f.taille_block;
        x->res=0;
        if (chevauchement)
        {
            x->chevauchement=true;
        }
        if (occ!=0 && prd->chevauchement==false)
        {
            x->nb_enr=occ+1;
        }
        if (chevauchement && occ==0)
        {
            x->nb_enr=-1;
        }
        if (occ!=0 && prd->chevauchement==true)
        {
            x->nb_enr=occ;
        }
        
        
       
        for (int i = 0; i < x->nb_enr; i++)
        {
            x->suppresion[i]=0;
        }
        
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
    char *saveptr1=NULL;
    char *saveptr2=NULL;
    char tmp[100];
    while (!(*trouv) && !(stop) && *i<=nb_block)
    {
        lireblock(f,*i,buffer);
        
        strtoken1=strtok_r(buffer, "$", &saveptr1);//$ est le separateur d'enregistrement
        while ( strtoken1 != NULL ) {
            strcpy(tmp,strtoken1);
            strtoken2=strtok_r(tmp, "#", &saveptr2);//# est le separateur d'atribut
            strcpy(enregistremet,strtoken2);
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
            strtoken1 = strtok_r(NULL, "$", &saveptr1); // recuperer le prochain enregistrement            
            }
            (*i)++;
            
            chevauchement=enteteblock(f,*i,0);//le cas ou il y a un chevauchement dans le block
            if (strtoken1==NULL && !trouv && *chevauchement==true)
            {
                lireblock(f,*(i),buffer2);
                strtoken1=strtok_r(buffer2, "$",&saveptr1);//$ est le separateur d'enregistrement
                strcat(enregistremet,buffer2);
                strtoken2=strtok_r(enregistremet, "#",&saveptr2);
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
    fichier f;
    f.nb_block=0;
    f.taille_block=11;
    f.supp_logique=true;
    f.debut=NULL;
    f.fin=NULL;
    printf("test1\n");
    int i=allocblock(&f);
    printf("test2\n");
    ecrireblock(f,i,"1#$2#$23#$",false);
    printf("test3\n");
    i=allocblock(&f);
    printf("test3,5\n");
    ecrireblock(f,i,"26#$3465#$",false);
    i=allocblock(&f);
    printf("test4\n");
    ecrireblock(f,i,"45678902#$",false);
    bool trouv=false;
    int j;
    printf("test5\n");
    recherche("23",&trouv,&i,&j,f);
    printf("test6\n");
    if (trouv==true)
    {
        printf(" 2 ce trouve dans le block %d et l'enregistrement %d\n",i,j);
    }
}