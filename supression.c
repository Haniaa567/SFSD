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

//en entrée la clé (c) à chercher et le fichier.
//en sortie le booleen Trouv, le numéro de bloc (i) contenant la clé et le déplacement (j)
void recherche(char c[],bool *trouv,int *i,int *j ,fichier f)
{
    //c est l'element recherche
    //i le num de block ou devrait se trouver c
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
    *i=1; //initialiser au 1er block
    *j=0; //offset=0
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
        if ((*prd_ch==true && prd!=*i) && !(*trouv))
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
    bool *supp;
    supp=enteteblock(f,*i,1);
    if (*trouv)
    {
        if (*(supp+((*j)-1)*sizeof(bool))==true)
        {
            (*trouv)=false;
        }
    }
        if (*trouv) {
        bool *supp = enteteblock(f, *i, 1);
        if (supp[*j - 1]) {
            *trouv = false; 
            // L'enregistrement a été supprimé logiquement
        }
}
}
block* ptr_block(fichier f,int position)
{
    block* x=Entete(f,1);
    int cpt=1;
    while(cpt!=position && x!=NULL)
    {
        cpt++;
        x=x->svt;
    } 
    return x;
    
}

void SuppressionLogique(fichier f,char cle[])
{
    bool trouv;
    int pos_block,pos_eng;
    size_t taille_eng;
    int nb_block = entete(f,0);

    //Recuperer les cordonnes de la cle recherchee
        recherche(cle, &trouv, &pos_block, &pos_eng, f);

        if (trouv) {
            //Pointer vers le bloc ou se trouve la cle
            block *x=ptr_block(f,pos_block);
        
            //Lire le bloc
            char buffer[200];
            lireblock(f,pos_block,buffer);
            char *saveptr;
            //il faut trouver la bonne position de l'enregistrement ici ca marche pour 1 mais pas pour le reste
            char *strtoken = strtok_r(buffer,"$",&saveptr);
            bool chevauchement=enteteblock(f,pos_block,0);
            taille_eng=strtoken-buffer;

            /*On va supprimer l'enregistrement logiquement*/
             //Maj des champs (supression, espace reste, espace occupe)
            //cas 1 : l'enregistrement se trouve dans le dernier bloc
            if (pos_block=nb_block){
                 x->nb_enr -= 1;
                x->suppresion[pos_eng-1] = true;
                x->ocup -= taille_eng;
                x->res += taille_eng;
            }
            //cas 2 : l'enregistrement n'est pas dans le dernier bloc
            else {
                int nb_eng=Enteteblock(f,pos_block,3);
                //2-1 : l'enregistrement n'est pas le dernier dans le bloc 
                if(pos_eng!=nb_eng)
                {
                    x->nb_enr -= 1;
                    x->suppresion[pos_eng-1] = true;
                    x->ocup -= taille_eng;
                    x->res += taille_eng;
                }
                //2-2 : l'enregistrement est le dernier dans le bloc mais ne chevauche pas 
                else if(pos_eng==nb_eng && chevauchement==false){
                    x->nb_enr -= 1;
                    x->suppresion[pos_eng-1] = true;
                    x->ocup -= taille_eng;
                    x->res += taille_eng;
                }
                //2-3 : l'enregistrement est le dernier dans le bloc et chevauche sur 1 ou plusieurs blocs
                else if(pos_eng==nb_eng && chevauchement==true )
                {
                    x->nb_enr -= 1;
                    x->suppresion[pos_eng-1] = true;
                    x->ocup -= taille_eng;
                    x->res += taille_eng;
                    pos_block++;
                    nb_eng=Enteteblock(f,pos_block,3);
                    //effectuer des changements sur les blocs que l'eng cherche prend carrement
                    while(pos_block<nb_block && nb_eng==-1)
                    {
                        x=ptr_block(f,pos_block);
                        x->suppresion[0]=true;
                        x->ocup=0;
                        x->res=200;
                        x->nb_enr=0;
                        x->chevauchement=false;
                        lireblock(f,pos_block,buffer);
                        strtoken = strtok_r(buffer,"$",&saveptr);
                        pos_block++;
                        nb_eng=Enteteblock(f,pos_block,3);
                    }
                    /*le cas du bloc ou se termine l'enregistrement/atteinte du dernier fichier : apart, car si on avance au prochain bloc et on trouve nb_eng!=-1
                    il faut verifier si il s'est termine au bloc precedent ou bien il y'a encore un morceau dans le bloc crt (*strtoke==NULL)<=>"$" non trouve*/
                    if( strtoken==NULL) 
                    {
                        lireblock(f,pos_block,buffer);
                        strtoken = strtok_r(buffer,"$",&saveptr);
                        taille_eng=strtoken-buffer;
                         x->nb_enr -= 1;
                         x->suppresion[0] = true;
                         x->ocup -= taille_eng;
                         x->res += taille_eng;
                         if(taille_eng==200){
                            x->chevauchement=false;
                         }
                    }

                }
            }
        }
        //Cle non trouve
        else{
            printf("error! not available\n");
        }
}

int main()
{
    fichier test;
    int i,j;
    bool trouv;
    char cle1[]="bellil",cle2[]="bellllil";
    int z=allocblock(&test);
    ecrireblock(test,z,"bellllil#nawel$");
    z=allocblock(&test);
    ecrireblock(test,z,"belaredj#amel$");
    z=allocblock(&test);
    ecrireblock(test,z,"bellil#napelnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn$");
    //sans chevauchement
    recherche(cle2,&trouv,&i,&j,test);
    if(trouv==true)
    {
        printf("val 2 trouvee\n");
    }
    else printf("error404, not found");

    SuppressionLogique(test,cle2);
        recherche(cle2,&trouv,&i,&j,test);
    if(trouv==true)
    {
        printf("val trouvee\n");
    }
    else printf("error404, not found\n");
    //chevauchement 
    recherche(cle1,&trouv,&i,&j,test);
    if(trouv==true)
    {
        printf("val 1 trouvee\n");
    }
    else printf("error404, not found\n");

    SuppressionLogique(test,cle1);
        recherche(cle1,&trouv,&i,&j,test);
    if(trouv==true)
    {
        printf("val trouvee\n");
    }
    else printf("error404, not found\n");
    


    


    return 0;
}