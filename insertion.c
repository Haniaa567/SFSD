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
    bloc->chevauchement=false;//le bloc sera vide donc les information auront des valeurs par defaut
    bloc->svt=NULL;
    bloc->res=f->taille_block;
    bloc->ocup=0;
    int cpt=1;
    //chercher la bonne position de bloc
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
    //trouver la bonne position
    while (cpt!=i && (n_block)!=NULL)
    {
        cpt++;
        n_block=n_block->svt;
    }
    //copier le contenu de block dans de buffer
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
    //trouver le bon block
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
        //initialiser les informations de block(chevauchement)
        printf("%d=%docc\n",i,occ);
        if (x->enregistrement[f.taille_block-1]=='$')
        {
            x->chevauchement=false;
        }
        else
        {
            x->chevauchement=true;
        }
        //nombre d'enregistrement
        if (prd !=x)
        {
            if (occ!=0 && prd->chevauchement==false && !(x->chevauchement))
            {
                x->nb_enr=occ+1;
            }
            else if (occ!=0 && prd->chevauchement==false && (x->chevauchement))
            {
                x->nb_enr=occ+1;
            }
            else if (x->chevauchement && occ==0 && prd->chevauchement==true)
            {
                x->nb_enr=-1;
                printf("?-1?\n");
            }
            else if (x->chevauchement && occ==0 && prd->chevauchement==false)
            {
                x->nb_enr=0;
            }
            else if (occ>1 && prd->chevauchement==true && !(x->chevauchement))
            {
                x->nb_enr=occ-1;
            }
            else if (occ>1 && prd->chevauchement==true && (x->chevauchement))
            {
                x->nb_enr=occ;
            }
            else if (occ==1 && prd->chevauchement==true && (x->chevauchement))
            {
                x->nb_enr=-1;
            }
            else if (occ==1 && prd->chevauchement==true && !(x->chevauchement))
            {
                x->nb_enr=-1;
            }
        }
        else
        {
            x->nb_enr=occ+1;
        }
        
        
        //suppression
        for (int i = 0; i < x->nb_enr; i++)
        {
            x->suppresion[i]=false;
        }
        printf("%d=%dnb\n",i,x->nb_enr);
        
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
    int k;
    while (!(*trouv) && !(stop) && *i<=nb_block)
    {
        lireblock(f,*i,buffer);   
        strtoken1=strtok_r(buffer, "$", &saveptr1);//$ est le separateur d'enregistrement
        *j=0;//la position de l'enregistrement
        prd_ch=enteteblock(f,prd,0);
        if ((*prd_ch==true && prd!=*i))
        {
            printf("!!!%d=i,%d=j\n",*i,*j);
            strtoken1 = strtok_r(NULL, "$", &saveptr1); // recuperer le prochain enregistrement  
        }
        while ( strtoken1 != NULL && !(*trouv) && !stop) {
            strcpy(tmp,strtoken1);//pour modifier tmp afin de trouver les attribut
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
            //le cas ou on a trouver la mauvaise cle a cause de chevauchement
            if ((*trouv)==true && strtoken1== NULL && *chevauchement==true && strtoken2==NULL)
            {
                (*trouv)=false;
            }

        }
        prd=*i;
        (*i)++;
        int x;
        x=prd;
        k=(*i);   
        chevauchement=enteteblock(f,k-1,0);//le cas ou il y a un chevauchement dans le block
        if (strtoken1==NULL && (*trouv)==false && *chevauchement==true)
        {    
            lireblock(f,k,buffer2);
            strtoken1=strtok_r(buffer2, "$",&saveptr1);//$ est le separateur d'enregistrement
            strcat(enregistremet,buffer2);//trouver l'enregitrement qui a etait couper
            while (k<nb_block && Enteteblock(f,k,3)==-1)
            {
                k++;
                lireblock(f,k,buffer2);
                strtoken1=strtok_r(buffer2, "$",&saveptr1);//$ est le separateur d'enregistrement
                strcat(enregistremet,buffer2);//trouver l'enregitrement qui a etait couper
            }
            strtoken2=strtok_r(enregistremet, "#",&saveptr2);
            if (strcmp(strtoken2,c)==0)
            {
                (*trouv)=true;
                *i=x+1;
            }
            
        }   
            
    }
    (*i)--;//repositioner le numero de block
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
           /*if ()
            {
                
            }*/
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
     ajout_entete(f,i,1);//rendre le champs supprime a faux (restorer)
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
