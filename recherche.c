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

void ecrireblock(fichier f,int i,char buffer[])
{
    //on ne peut pas ecraser un block cette fonction ne marche pas dans ce cas
    int cpt=1;
    int occ=compterOccurrences(buffer,'$');
    block *x=Entete(f,1);//l'adresse de debut
    block *prd=x;
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
        if (x->enregistrement[f.taille_block-1]=='$')
        {
            x->chevauchement=false;
        }
        else
        {
            x->chevauchement=true;
        }
        if (occ!=0 && prd->chevauchement==false)
        {
            x->nb_enr=occ+1;
        }
        if (x->chevauchement && occ==0)
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
    bool *chevauchement,*prd_ch;
    *j=0;
    *i=1;
    char *saveptr1=NULL;
    char *saveptr2=NULL;
    char tmp[100];
    int prd=*i;//dans le cas ou il y a un chevauchemant dans le block d'avant
    while (!(*trouv) && !(stop) && *i<=nb_block)
    {
        lireblock(f,*i,buffer);
        
        strtoken1=strtok_r(buffer, "$", &saveptr1);//$ est le separateur d'enregistrement
        *j=0;//la position de l'enregistrement
        prd_ch=enteteblock(f,prd,0);
        if (*prd_ch==true || prd==1)
        {
            strtoken1 = strtok_r(NULL, "$", &saveptr1); // recuperer le prochain enregistrement  
        }
        while ( strtoken1 != NULL && !(*trouv) && !stop) {
            strcpy(tmp,strtoken1);
            strcpy(enregistremet,strtoken1);//pour conserver le dernier enregistrement en cas ou il y a un chevauchement
            strtoken2=strtok_r(tmp, "#", &saveptr2);//# est le separateur d'atribut
            if (strcmp(strtoken2,c)==0 )//la cle se trouve dans le premier champs
            {
                (*trouv)=true;
            }
            else if (strcmp(strtoken2,c)>0)
            {
                (stop)=true;
                (*j)--;
            }
            strtoken2 = strtok_r(NULL, "#", &saveptr2); // recuperer le prochain attribut
                
            (*j)++;
            strtoken1 = strtok_r(NULL, "$", &saveptr1); // recuperer le prochain enregistrement
            chevauchement=enteteblock(f,*i,0);//le cas ou il y a un chevauchement dans le block 
            if ((*trouv)==true && strtoken1== NULL && *chevauchement==true && strtoken2==NULL)
            {
                (*trouv)=false;
            }
                       
        }
        prd=*i;
        (*i)++;

            
        chevauchement=enteteblock(f,(*i)-1,0);//le cas ou il y a un chevauchement dans le block
        if (strtoken1==NULL && (*trouv)==false && *chevauchement==true)
        {
            lireblock(f,*(i),buffer2);
            strtoken1=strtok_r(buffer2, "$",&saveptr1);//$ est le separateur d'enregistrement
            strcat(enregistremet,buffer2);
            strtoken2=strtok_r(enregistremet, "#",&saveptr2);
            if (strcmp(strtoken2,c)==0)
            {
                (*trouv)=true;
            }
            
        }   
            
    }
    (*i)--;//repositioner le numero de block
}
int main()
{
    fichier f;
    f.nb_block=0;
    f.taille_block=10;
    f.supp_logique=true;
    f.debut=NULL;
    f.fin=NULL;
    int i=allocblock(&f);
    ecrireblock(f,i,"1#$2#$9202");
    i=allocblock(&f);
    ecrireblock(f,i,"96#$3465#$");
    i=allocblock(&f);
    ecrireblock(f,i,"45678902#$");
    bool trouv=false;
    int j;
    recherche("920296",&trouv,&i,&j,f);
    if (trouv==true)
    {
        printf("la valeur ce trouve dans le block %d et l'enregistrement %d\n",i,j);
    }
    else
    {
        printf("la valeur ne se trouve pas dans le fichier le numero de block avant la valeur est %d et la position est %d",i,j);
    }
    
}