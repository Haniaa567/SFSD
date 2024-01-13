#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include <gtk/gtk.h>

#define block_width 250.0
#define block_height 180.0
#define field_width 30.0
#define field_height 20.0
#define CELL_SPACING 85.0
#define BLOCK_PAR_LIGNE 4
#define STARTX 100
#define STARTY 300


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
// Define global variables pour stocker GTK widgets et autre data
GtkWidget *window;
GtkWidget *drawing_area;
fichier f;

// Variables pour suivre la disposition actuelle
double x = STARTX;
double y = STARTY;
int highlighted_block = -1;
int highlighted_record = -1;
gboolean left_to_right = TRUE;
gboolean top_to_down = FALSE;
gboolean chvchmnt = FALSE;






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
//j'ai mis le retour un ppointeur pou pouvoir retourner un tableau(suppression)
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
//une autre fonction car ce n'est pas le meme type de retour
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
                x->nb_enr=occ;
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
                x->nb_enr=occ;
            }
            else if (occ>1 && prd->chevauchement==true && (x->chevauchement))
            {
                x->nb_enr=occ;
            }
            
            else if (occ==1 && prd->chevauchement==true && (x->chevauchement))
            {
                x->nb_enr=1;
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
char* extraireEtModifierenregi(const char *chaine, int i) {
    char *copieChaine = strdup(chaine);  // Duplication pour ne pas modifier la chaîne originale
    char *token;
    char *resultat = NULL;

    // Découper la chaîne en utilisant le caractère '$'
    token = strtok(copieChaine, "$");

    // Parcourir les parties de la chaîne jusqu'à la partie numéro i
    while (token != NULL && i > 0) {
        resultat = token;
        token = strtok(NULL, "$");
        i--;
    }

    if (resultat != NULL) {
        // Remplacer '#' par ' ' dans la partie sélectionnée
        for (int j = 0; j < strlen(resultat); j++) {
            if (resultat[j] == '#') {
                resultat[j] = ' ';
            }
        }

        // Dupliquer le résultat pour éviter les problèmes de mémoire
        resultat = strdup(resultat);
    }

    // Libérer la mémoire allouée pour la copie initiale
    free(copieChaine);

    return resultat;
}

void sauvegarderFichierEnTexte(const char* nomFichier, fichier maListe) {
    /*// Ouverture du fichier en mode écriture
    FILE* fichier = fopen(nomFichier, "w");

    // Vérification si l'ouverture a réussi
    if (fichier == NULL) {
        perror("Erreur lors de l'ouverture du fichier");
        return;
    }

    // Écriture des informations sur le fichier
    //fprintf(fichier, "%d %d %d\n", maListe->nb_block, maListe->taille_block, maListe->supp_logique);

    // Parcours de la liste et écriture de chaque bloc dans le fichier
    block* currentBlock = maListe.debut;
    int i=1;
    while (currentBlock != NULL) {
        fprintf(fichier, "%s \n", currentBlock->enregistrement);

        currentBlock = currentBlock->svt;
    }

    // Fermeture du fichier*/
     FILE* fichier = fopen(nomFichier, "w");

    // Vérification si l'ouverture a réussi
    if (fichier == NULL) {
        perror("Erreur lors de l'ouverture du fichier");
        return;
    }

    // Écriture des informations sur le fichier
    //fprintf(fichier, "%d %d %d\n", maListe.nb_block, maListe.taille_block, maListe.supp_logique);

    // Parcours de la liste et écriture de chaque bloc dans le fichier
    char tmp[500];
    char *affiche;
    block* currentBlock = maListe.debut;
    int j=1,nb=currentBlock->nb_enr,i=0;
    while (currentBlock != NULL) {
        while (i < nb)
        {
            strcpy(tmp,currentBlock->enregistrement);
            if (currentBlock->suppresion[i]==false)
            {
                affiche=extraireEtModifierenregi(tmp,i+1);
                fprintf(fichier,"%s",affiche);
                printf("en%d\n",i);
                if (currentBlock->chevauchement==false || i+1!=currentBlock->nb_enr)
                {
                fprintf(fichier,"\n");
                
                }
            }
            i++;
        }
        if(currentBlock->svt!=NULL){
        if (currentBlock->chevauchement && (currentBlock->nb_enr==-1 ||(currentBlock->nb_enr!=-1 && currentBlock->suppresion[currentBlock->nb_enr-1]==false)))
        {
            i=0;
            if (Enteteblock(f,j+1,3)!=-1)
            {
                nb=Enteteblock(f,j+1,3)+1;
            }
            else{
                nb=1;
            } 
            printf("1\n");
        }
        else if(currentBlock->chevauchement && (currentBlock->nb_enr==0 ||(currentBlock->nb_enr!=0 && currentBlock->suppresion[currentBlock->nb_enr-1])))
        {
            i=1;
            nb=Enteteblock(f,j+1,3);
            printf("2\n");
        }
        else if(!currentBlock->chevauchement)
        {
            i=0;
            nb=Enteteblock(f,j+1,3);
            printf("3 %d\n",nb);
            
        }
        }
        /*
        for (int i = 0; i < currentBlock->nb_enr; i++)
        {
            fprintf(fichier, "%d ",currentBlock->suppresion[i]);
        }
        fprintf(fichier,"\n");*/
                 

        currentBlock = currentBlock->svt;
        printf("blovk%d\n",j);
        j++;
       
    }

    // Fermeture du fichier
    fclose(fichier);
}

block* creerNouveauBlock() {
    block* nouveauBlock = (block*)malloc(sizeof(block));
    if (nouveauBlock != NULL) {
        // Initialiser les valeurs du nouveau bloc ici
        nouveauBlock->svt = NULL;
    }
    return nouveauBlock;
}

void ajouterBlock(fichier* maListe, block* nouveauBlock) {
    if (maListe->debut == NULL) {
        // La liste est vide
        maListe->debut = maListe->fin = nouveauBlock;
    } else {
        // Ajouter le bloc à la fin de la liste
        maListe->fin->svt = nouveauBlock;
        maListe->fin = nouveauBlock;
    }
}

void chargerListeDepuisFichierTexte(const char* nomFichier, fichier* maListe) {
    // Ouverture du fichier en mode lecture
    FILE* fichier = fopen(nomFichier, "r");

    // Vérification si l'ouverture a réussi
    if (fichier == NULL) {
        perror("Erreur lors de l'ouverture du fichier");
        return;
    }
    int supp,chvchmnt;
    // Lecture des informations sur le fichier
    fscanf(fichier, "%d %d %d\n", &maListe->nb_block, &maListe->taille_block, &supp);
    if(supp==1)
    {
        maListe->supp_logique=true;
    }
    else
    {
        maListe->supp_logique=false;
    }

    while (!feof(fichier)) {
        // Création d'un nouveau bloc
        block* nouveauBlock = creerNouveauBlock();

        // Lecture des propriétés du bloc depuis le fichier
        fscanf(fichier, "%s %d %d %d %d ", nouveauBlock->enregistrement, &nouveauBlock->nb_enr,
                &chvchmnt, &nouveauBlock->res, &nouveauBlock->ocup);
                if(chvchmnt==1)
                {
                    nouveauBlock->chevauchement=true;
                }
                else
                {
                    nouveauBlock->chevauchement=false;
                }
        for (int i = 0; i < nouveauBlock->nb_enr; i++)
        {
            fscanf(fichier,"%d ",&supp);
            if(supp==1)
            {
                nouveauBlock->suppresion[i]=true;
            }
            else
            {
                nouveauBlock->suppresion[i]=false;
            }
        }
        fscanf(fichier,"\n");
        
        // Ajout du bloc à la liste
        ajouterBlock(maListe, nouveauBlock);
    }

    // Fermeture du fichier
    fclose(fichier);
}


void sauvegarderFichierEnBinaire(const char* nomFichier, fichier* maListe) {
    // Ouverture du fichier en mode écriture binaire
    FILE* fichier = fopen(nomFichier, "wb");

    // Vérification si l'ouverture a réussi
    if (fichier == NULL) {
        perror("Erreur lors de l'ouverture du fichier");
        return;
    }

    // Écriture des informations sur le fichier
    fwrite(&maListe->nb_block, sizeof(int), 1, fichier);
    fwrite(&maListe->taille_block, sizeof(int), 1, fichier);
    fwrite(&maListe->supp_logique, sizeof(bool), 1, fichier);

    // Parcours de la liste et écriture de chaque bloc dans le fichier
    block* currentBlock = maListe->debut;
    while (currentBlock != NULL) {
        fwrite(currentBlock, sizeof(block), 1, fichier);
        currentBlock = currentBlock->svt;
    }

    // Fermeture du fichier
    fclose(fichier);
}

void chargerListeDepuisFichierBinaire(const char* nomFichier, fichier* maListe) {
    // Ouverture du fichier en mode lecture binaire
    FILE* fichier = fopen(nomFichier, "rb");

    // Vérification si l'ouverture a réussi
    if (fichier == NULL) {
        perror("Erreur lors de l'ouverture du fichier");
        return;
    }

    // Lecture des informations sur le fichier
    fread(&maListe->nb_block, sizeof(int), 1, fichier);
    fread(&maListe->taille_block, sizeof(int), 1, fichier);
    fread(&maListe->supp_logique, sizeof(bool), 1, fichier);

    while (1) {
        // Création d'un nouveau bloc
        block* nouveauBlock = (block*)malloc(sizeof(block));

        // Lecture du bloc depuis le fichier
        size_t bytesRead = fread(nouveauBlock, sizeof(block), 1, fichier);

        // Si la lecture n'a pas réussi, c'est probablement la fin du fichier
        if (bytesRead != 1) {
            free(nouveauBlock);
            break;
        }

        // Ajout du bloc à la liste
        nouveauBlock->svt = NULL;
        ajouterBlock(maListe, nouveauBlock);
    }

    // Fermeture du fichier
    fclose(fichier);
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
    bool *supp;
    supp=enteteblock(f,*i,1);
    if (*trouv)
    {
        if (*(supp+((*j)-1)*sizeof(bool))==true)
        {
            (*trouv)=false;
            (*j)--;
        }
    }
}
// dessiner des blocks

static gboolean draw_block(GtkWidget *widget, cairo_t *cr, gpointer data) {
    int block_index = GPOINTER_TO_INT(data);
    block *current_block = f.debut;
    for (int i = 1; i < block_index; i++) {
        if (current_block == NULL) {
            return FALSE;
        }
        current_block = current_block->svt;
    }

    if (current_block == NULL) {
        return FALSE;
    }

    cairo_rectangle(cr, x, y, block_width, block_height + current_block->nb_enr * field_width);
    cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
    cairo_fill_preserve(cr);
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_stroke(cr);

    cairo_move_to(cr, x + 10.0, y + 20.0);
    cairo_show_text(cr, "Block Headers :");
    cairo_move_to(cr, x + 10.0, y + 60.0);
    char tmp[300];
    bool *test;
    test=enteteblock(f, block_index, 0);
    if(*test)
        strcpy(tmp,"Chevauchement : Oui");
    else
        strcpy(tmp,"Chevauchement : Non");
    cairo_show_text(cr, tmp);
    cairo_move_to(cr, x + 10.0, y + 80.0);
    sprintf(tmp, "Nombre enregistrements : %i", Enteteblock(f, block_index, 3));
    cairo_show_text(cr, tmp);

    if (current_block->svt != NULL) {
        if(!top_to_down){
            double arrowStartX = left_to_right ? x + block_width : x;
            double arrowStartY = y + block_height / 2.0;
            double arrowEndX = left_to_right ? x + block_width + CELL_SPACING : x - CELL_SPACING;
            double arrowEndY = arrowStartY;

            cairo_move_to(cr, arrowStartX, arrowStartY);
            cairo_line_to(cr, arrowEndX, arrowEndY);
            cairo_stroke(cr);

            double arrowTipX = left_to_right ? arrowEndX - 10 :  arrowEndX + 10;
            double arrowTipY1 = arrowEndY - 5;
            double arrowTipY2 = arrowEndY + 5;

            cairo_move_to(cr, arrowTipX, arrowTipY1);
            cairo_line_to(cr, arrowEndX, arrowEndY);
            cairo_line_to(cr, arrowTipX, arrowTipY2);
            cairo_fill(cr);
        }else{
            double arrowStartX = x + block_width / 2.0;
            double arrowStartY = y + block_height + current_block->nb_enr * field_width;
            double arrowEndX = arrowStartX;
            double arrowEndY = y + block_height + CELL_SPACING + 50.0;

            cairo_move_to(cr, arrowStartX, arrowStartY);
            cairo_line_to(cr, arrowEndX, arrowEndY);
            cairo_stroke(cr);

            double arrowTipY = arrowEndY - 10;
            double arrowTipX1 = arrowEndX - 5;
            double arrowTipX2 = arrowEndX + 5;

            cairo_move_to(cr, arrowTipX1, arrowTipY);
            cairo_line_to(cr, arrowEndX, arrowEndY);
            cairo_line_to(cr, arrowTipX2, arrowTipY);
            cairo_fill(cr);
            top_to_down = FALSE;
        }
    }

    return FALSE;
}

static gboolean draw_enregistrement(GtkWidget *widget, cairo_t *cr, gpointer data) {
    int block_index = GPOINTER_TO_INT(data);

    block *current_block = f.debut;
    int cpt=1;
    block *prd=f.debut;
    for (int i = 1; i < block_index; i++) {
        if (current_block == NULL) {
            return FALSE;
        }
        prd=current_block;
        current_block = current_block->svt;
        cpt++;
    }

    if (current_block == NULL) {
        return FALSE;
    }
    char *saveptr1=NULL;
    
    char *enregistrement = strdup(current_block->enregistrement);
    char *token = strtok_r(enregistrement, "$", &saveptr1);
    int field_index = 1;
    if (prd->chevauchement==true && prd!=current_block)
    {
        field_index=0;
    }
    

    while (token != NULL) {
        char *saveptr2=NULL;
        double x_field = x + 120.0 + field_width;
        double y_field = y + 100.0 + field_index * field_width;
        char temp[30];
        sprintf(temp, "Enregistrement num %i :", field_index);
        cairo_move_to(cr, x + 10.0, y_field - 2.0);
        cairo_show_text(cr, temp);

        char *tmp = strdup(token);
        char *tkn = strtok_r(tmp, "#", &saveptr2);
        int i = 0;
        while(tkn){
            double xc = x_field + i * field_width + 0.2 * sizeof(tkn);
            cairo_rectangle(cr, xc, y_field, field_width + sizeof(tkn) / 2.0, field_height);
            cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
            cairo_fill_preserve(cr);
            cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
            cairo_stroke(cr);
            
            cairo_move_to(cr, xc + 5.0, y_field + field_height - 0.3 * field_height);
            bool *b = enteteblock(f, cpt, 1);
            if(field_index!=0 && !b[field_index-1])
                cairo_show_text(cr, tkn);
            else if (field_index==0 && prd->chevauchement==true && prd->suppresion[prd->nb_enr-1]==false)
            {
                cairo_show_text(cr, tkn);
            }
                
            tkn = strtok_r(NULL, "#", &saveptr2);
            i++;
        }
        free(tmp);
        token = strtok_r(NULL, "$", &saveptr1);
        field_index++;
    }

    free(enregistrement);
    return FALSE;
}

void update_gui() {
    gtk_widget_queue_draw(drawing_area);
}




void on_insert_button_clicked(GtkWidget *widget, gpointer data) {
 GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *entry;
    gint result;

   
    
    dialog = gtk_dialog_new_with_buttons("inserer un élément",
        GTK_WINDOW(gtk_widget_get_toplevel(widget)),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "OK", GTK_RESPONSE_OK,
        "Annuler", GTK_RESPONSE_CANCEL,
        NULL);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "entrer les donnees:");
    gtk_container_add(GTK_CONTAINER(content_area), entry);
    gtk_widget_show_all(dialog);
}


void on_delete_button_clicked(GtkWidget *widget, gpointer data) {
    
}
void calculate_block_position(int block_index, double *x_block, double *y_block){
    int j = 0;
    *x_block = STARTX;
    *y_block = STARTY;
    left_to_right = TRUE;
    top_to_down = FALSE;
    bool local_left_to_right = FALSE;
    for (int i = 1; i < block_index; i++) {
        if (j % BLOCK_PAR_LIGNE == BLOCK_PAR_LIGNE - 1)
            top_to_down = TRUE;

        if (left_to_right) {
            *x_block += block_width + CELL_SPACING;
            if (j % BLOCK_PAR_LIGNE == BLOCK_PAR_LIGNE - 1){
                *x_block -= block_width + CELL_SPACING;
                *y_block += block_height + CELL_SPACING + 50.0;
                left_to_right = FALSE;
            }
        } else {
            *x_block -= block_width + CELL_SPACING;
            if (j % BLOCK_PAR_LIGNE == BLOCK_PAR_LIGNE - 1) {
                *x_block = CELL_SPACING;
                *y_block += block_height + CELL_SPACING + 50.0;
                left_to_right = TRUE;
            }
        }
        j++;
    }
    left_to_right = TRUE;
    top_to_down = FALSE;
}

static void draw_highlighted_block(cairo_t *cr, int block_index) {

    block *current_block = f.debut;
    for (int i = 1; i < block_index; i++) {
        if (current_block == NULL) {
            return;
        }
        current_block = current_block->svt;
    }

    if (current_block == NULL) {
        return;
    }
    
    double x_block, y_block;

    calculate_block_position(block_index, &x_block, &y_block);

    cairo_rectangle(cr, x_block, y_block, block_width, block_height + current_block->nb_enr * field_width);
    cairo_set_source_rgb(cr, 0.35, 0.35, 0.5);
    cairo_fill_preserve(cr);
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_stroke(cr);

    cairo_move_to(cr, x_block + 10.0, y_block + 20.0);
    cairo_show_text(cr, "Block Headers :");
    cairo_move_to(cr, x_block + 10.0, y_block + 60.0);
    char tmp[30];
    bool *test;
    test=enteteblock(f, block_index, 0);
    if((*test))
    {
        strcpy(tmp,"Chevauchement :oui");
    }    
    else
    {
        strcpy(tmp,"Chevauchement : Non");
    }
        
    cairo_show_text(cr, tmp);
    cairo_move_to(cr, x_block + 10.0, y_block + 80.0);
    sprintf(tmp, "Nombre enregistrements : %i", Enteteblock(f, block_index, 3));
    cairo_show_text(cr, tmp);

    if (current_block->svt != NULL) {
        if(!top_to_down){
            double arrowStartX = left_to_right ? x + block_width : x;
            double arrowStartY = y + block_height / 2.0;
            double arrowEndX = left_to_right ? x + block_width + CELL_SPACING : x - CELL_SPACING;
            double arrowEndY = arrowStartY;

            cairo_move_to(cr, arrowStartX, arrowStartY);
            cairo_line_to(cr, arrowEndX, arrowEndY);
            cairo_stroke(cr);

            double arrowTipX = left_to_right ? arrowEndX - 10 :  arrowEndX + 10;
            double arrowTipY1 = arrowEndY - 5;
            double arrowTipY2 = arrowEndY + 5;

            cairo_move_to(cr, arrowTipX, arrowTipY1);
            cairo_line_to(cr, arrowEndX, arrowEndY);
            cairo_line_to(cr, arrowTipX, arrowTipY2);
            cairo_fill(cr);
        }else{
            double arrowStartX = x + block_width / 2.0;
            double arrowStartY = y + block_height + current_block->nb_enr * field_width;
            double arrowEndX = arrowStartX;
            double arrowEndY = y + block_height + CELL_SPACING + 50.0;

            cairo_move_to(cr, arrowStartX, arrowStartY);
            cairo_line_to(cr, arrowEndX, arrowEndY);
            cairo_stroke(cr);

            double arrowTipY = arrowEndY - 10;
            double arrowTipX1 = arrowEndX - 5;
            double arrowTipX2 = arrowEndX + 5;

            cairo_move_to(cr, arrowTipX1, arrowTipY);
            cairo_line_to(cr, arrowEndX, arrowEndY);
            cairo_line_to(cr, arrowTipX2, arrowTipY);
            cairo_fill(cr);
            top_to_down = FALSE;
        }
    }
    
}

static void draw_highlighted_record(cairo_t *cr, int block_index, int record_index) {
    
    block *current_block = f.debut;
    int cpt=1;
    for (int i = 1; i < block_index; i++) {
        if (current_block == NULL) {
            return;
        }
        current_block = current_block->svt;
        cpt++;
    }

    if (current_block == NULL) {
        return;
    }


    double x_block, y_block;
    calculate_block_position(block_index, &x_block, &y_block);

    char *saveptr1=NULL;
    
    char *enregistrement = strdup(current_block->enregistrement);
    char *token = strtok_r(enregistrement, "$", &saveptr1);
    int field_index = 1;
    bool *test;
    if (block_index!=1)
    {
        test=enteteblock(f,block_index-1,0);
            if ( *test )
            {
                field_index--;
            }
    }
    
    bool suite=false;
    while (token != NULL) {
        char *saveptr2=NULL;
        double x_field = x_block + 120.0 + field_width;
        double y_field = y_block + 100.0 + field_index * field_width;
        char temp[30];
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        sprintf(temp, "Enregistrement num %i :", field_index);
        cairo_move_to(cr, x + 10.0, y_field - 2.0);
        cairo_show_text(cr, temp);

        char *tmp = strdup(token);
        char *tkn = strtok_r(tmp, "#", &saveptr2);
        int i = 0;
        int x=block_index;
        while(tkn){
            double xc = x_field + i * field_width + 0.2 * sizeof(tkn);
            
            if(field_index == record_index)
                cairo_set_source_rgb(cr, 1.0, 0, 0);
                /*test=enteteblock(f,block_index,0);
                if (*test && Enteteblock(f,block_index,3)==field_index)
                {
                    do
                    {
                        x++;
                        draw_highlighted_record(cr, x, 0);
                    } while (Enteteblock(f,x,3)==-1);
                    
                }*/
            else{
                cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
            }
            cairo_rectangle(cr, xc, y_field, field_width + sizeof(tkn) / 2.0, field_height);
            cairo_fill_preserve(cr);
            cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
            cairo_stroke(cr);
            
            cairo_move_to(cr, xc + 5.0, y_field + field_height - 0.3 * field_height);

            bool *b = enteteblock(f, cpt, 1);
            if(!b[field_index])
                cairo_show_text(cr, tkn);
            tkn = strtok_r(NULL, "#", &saveptr2);
            i++;
        }
        free(tmp);
        token = strtok_r(NULL, "$", &saveptr1);
        field_index++;
    }

    free(enregistrement);
}

void highlight_block_and_record(int block_index, int record_index) {
    
    gtk_widget_queue_draw(drawing_area);
    cairo_t *cr;
    cr = gdk_cairo_create(gtk_widget_get_window(drawing_area));
    //GdkWindow *window = gtk_widget_get_window(drawing_area);
    //GdkDrawingContext *context = gdk_window_begin_draw_frame(window, NULL);
    //cr = gdk_drawing_context_get_cairo_context(context);

    draw_highlighted_block(cr, block_index);
    draw_highlighted_record(cr, block_index, record_index);
    
    
    cairo_destroy(cr);
}

void on_refresh_button_clicked(GtkWidget *widget, gpointer data) {
    highlighted_block = -1;
    highlighted_record = -1;
    update_gui();
}

void on_search_button_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *entry;
    gint result;

    bool trouv = FALSE;
    
    dialog = gtk_dialog_new_with_buttons("Rechercher un élément",
        GTK_WINDOW(gtk_widget_get_toplevel(widget)),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "OK", GTK_RESPONSE_OK,
        "Annuler", GTK_RESPONSE_CANCEL,
        NULL);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Clef");
    gtk_container_add(GTK_CONTAINER(content_area), entry);
    gtk_widget_show_all(dialog);

    result = gtk_dialog_run(GTK_DIALOG(dialog));

    if (result == GTK_RESPONSE_OK) {
        
        const char *key = gtk_entry_get_text(GTK_ENTRY(entry));
        char *tmp = strdup(key);
        recherche(tmp, &trouv, &highlighted_block, &highlighted_record, f);
        g_print("%i",highlighted_record);
        free(tmp);
        bool *chevau;
        int k=highlighted_block;
        if (trouv) {
            highlight_block_and_record(highlighted_block, highlighted_record);
            int j = 1;
            chvchmnt = *enteteblock(f, highlighted_block, 0) && (highlighted_record == Enteteblock(f, highlighted_block, 3));
            if (chvchmnt)
            {
                do{
                    //chvchmnt = *enteteblock(f, highlighted_block + j, 0) && Enteteblock(f, highlighted_block + j, 3) == 1;
                    highlight_block_and_record(highlighted_block + j, 0);
                    j++;
                    k++;
                }while( Enteteblock(f,k,3)==-1 && k <= f.nb_block);
             
            }
            
               
        } else {
            on_refresh_button_clicked(NULL, NULL);
            GtkWidget *info_dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                             GTK_DIALOG_MODAL,
                                                             GTK_MESSAGE_INFO,
                                                             GTK_BUTTONS_OK,
                                                             "Enregistrement non trouve");
            gtk_dialog_run(GTK_DIALOG(info_dialog));
            gtk_widget_destroy(info_dialog);
        }
    }

    gtk_widget_destroy(dialog);
}

/*void on_refresh_button_clicked(GtkWidget *widget, gpointer data) {
    highlighted_block = -1;
    highlighted_record = -1;
    update_gui();
}*/
static void on_size_allocate(GtkWidget *widget, GdkRectangle *allocation, gpointer data) {
    update_gui();
}

static void on_scrolled(GtkAdjustment *adjustment, gpointer data) {
    update_gui();
}

static gboolean on_focus_in_event(GtkWidget *widget, GdkEventFocus *event, gpointer user_data) {
    update_gui();
    return FALSE;
}

static gboolean on_focus_out_event(GtkWidget *widget, GdkEventFocus *event, gpointer user_data) {
    update_gui();
    return FALSE;
}

static double calculate_required_height(int num_blocks) {
    int rows = (num_blocks + BLOCK_PAR_LIGNE - 1) / BLOCK_PAR_LIGNE;
    return rows * (block_height + CELL_SPACING + 50) + STARTY;
}

static gboolean draw_file(GtkWidget *widget, cairo_t *cr, gpointer data) {

    double x_f = STARTX, y_f = 20.0;
    cairo_rectangle(cr, x_f, y_f, block_width, block_height);
    cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
    cairo_fill_preserve(cr);
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_stroke(cr);

    cairo_move_to(cr, x_f + 10.0, y_f + 20.0);
    cairo_show_text(cr, "Fichier Headers :");
    cairo_move_to(cr, x_f + 10.0, y_f + 60.0);
    char tmp[30];
    sprintf(tmp, "Nombre blocks : %i", entete(f, 0));
    cairo_show_text(cr, tmp);
    cairo_move_to(cr, x_f + 10.0, y_f + 80.0);
    sprintf(tmp, "Taille block : %i", entete(f, 2));
    cairo_show_text(cr, tmp);

    if (f.debut != NULL) {
        double arrowStartX = x_f + block_width / 2.0;
        double arrowStartY = y_f + block_height;
        double arrowEndX = arrowStartX;
        double arrowEndY = STARTY;

        cairo_move_to(cr, arrowStartX, arrowStartY);
        cairo_line_to(cr, arrowEndX, arrowEndY);
        cairo_stroke(cr);

        double arrowTipY = arrowEndY - 10;
        double arrowTipX1 = arrowEndX - 5;
        double arrowTipX2 = arrowEndX + 5;

        cairo_move_to(cr, arrowTipX1, arrowTipY);
        cairo_line_to(cr, arrowEndX, arrowEndY);
        cairo_line_to(cr, arrowTipX2, arrowTipY);
        cairo_fill(cr);
    }

    x = STARTX;
    y = STARTY;
    left_to_right = TRUE;
    top_to_down = FALSE;

    int num_blocks = f.nb_block;

    double required_height = calculate_required_height(num_blocks) + 100;

    double current_height;
    gtk_widget_get_size_request(drawing_area, NULL, &current_height);
    if (required_height > current_height) {
        gtk_widget_set_size_request(drawing_area, -1, required_height);
    }
    int j = 0;
    for (int i = 1; i <= num_blocks; i++) {
    
        if (j % BLOCK_PAR_LIGNE == BLOCK_PAR_LIGNE - 1)
            top_to_down = TRUE;
        if(i != highlighted_block && !chvchmnt){
            draw_block(widget, cr, GINT_TO_POINTER(i));
            draw_enregistrement(widget, cr, GINT_TO_POINTER(i));
        }else{
            if(!chvchmnt)
                highlight_block_and_record(i, highlighted_record);
            else
                highlight_block_and_record(i, 0);
            if(i == highlighted_block)
                chvchmnt = *enteteblock(f, i, 0) && (highlighted_record == Enteteblock(f, i, 3));
            else
                chvchmnt = *enteteblock(f, i, 0) && Enteteblock(f, i, 3) == 1;
        }
        if (left_to_right) {
            x += block_width + CELL_SPACING;
            if (j % BLOCK_PAR_LIGNE == BLOCK_PAR_LIGNE - 1){
                x -= block_width + CELL_SPACING;
                y += block_height + CELL_SPACING + 50.0;
                left_to_right = FALSE;
            }
        } else {
            x -= block_width + CELL_SPACING;
            if (j % BLOCK_PAR_LIGNE == BLOCK_PAR_LIGNE - 1) {
                x = CELL_SPACING;
                y += block_height + CELL_SPACING + 50.0;
                left_to_right = TRUE;
            }
        }
        j++;
    }

    return FALSE;
}

int main(int argc, char* argv[])
{
    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "File Visualization");
    gtk_window_set_default_size(GTK_WINDOW(window), 1500, 1000);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);


    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, 1500, 1000);
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled_window), drawing_area);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

    g_signal_connect(drawing_area, "draw", G_CALLBACK(draw_file), NULL);
    g_signal_connect(drawing_area, "size-allocate", G_CALLBACK(on_size_allocate), NULL);
    GtkAdjustment *vadjustment;
    vadjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scrolled_window));
    g_signal_connect(vadjustment, "value-changed", G_CALLBACK(on_scrolled), NULL);
    g_signal_connect(window, "focus-in-event", G_CALLBACK(on_focus_in_event), NULL);
    g_signal_connect(window, "focus-out-event", G_CALLBACK(on_focus_out_event), NULL);


    GtkWidget *insert_button = gtk_button_new_with_label("Inserer");
    GtkWidget *delete_button = gtk_button_new_with_label("Supprimer");
    GtkWidget *search_button = gtk_button_new_with_label("Rechercher");
    GtkWidget *refresh_button = gtk_button_new_with_label("Refresh");

    g_signal_connect(insert_button, "clicked", G_CALLBACK(on_insert_button_clicked), NULL);
    g_signal_connect(delete_button, "clicked", G_CALLBACK(on_delete_button_clicked), NULL);
    g_signal_connect(search_button, "clicked", G_CALLBACK(on_search_button_clicked), NULL);
    g_signal_connect(refresh_button, "clicked", G_CALLBACK(on_refresh_button_clicked), NULL);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(hbox), insert_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), delete_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), search_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), refresh_button, TRUE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
    
    //chargerListeDepuisFichierTexte("testfichier",&f);
    f.nb_block=0;
    f.taille_block=10;
    f.supp_logique=true;
    f.debut=NULL;
    f.fin=NULL;
    bool *test;
    block *debut=f.debut;
    int i=allocblock(&f);
    ecrireblock(f,i,"12#$24#$56\0");
    
    test=enteteblock(f,i,0);
    if (*test==true)
    {
        printf("oui\n");
    }
    else
    {
        printf("non\n");
    }
    
    i=allocblock(&f);
    ecrireblock(f,i,"963#$737#$\0");
    test=enteteblock(f,i,0);
    if (*test==true)
    {
        printf("oui\n");
    }
    else
    {
        printf("non\n");
    }
    i=allocblock(&f);
    ecrireblock(f,i,"825#$984#$\0");
    test=enteteblock(f,i,0);
    if (*test==true)
    {
        printf("oui\n");
    }
    else
    {
        printf("non\n");
    }
    sauvegarderFichierEnTexte("testfichier",f);

    /*i=allocblock(&f);
    ecrireblock(f,i,"12#123#$24#$56");
    i=allocblock(&f);
    ecrireblock(f,i,"963#$737#$");
    i=allocblock(&f);
    ecrireblock(f,i,"825#$984#$");
    i=allocblock(&f);
    ecrireblock(f,i,"12#123#$24#$56");
    i=allocblock(&f);
    ecrireblock(f,i,"963#$737#$");
    i=allocblock(&f);
    ecrireblock(f,i,"825#$984#$");
    i=allocblock(&f);
    ecrireblock(f,i,"12#123#$24#$56");
    i=allocblock(&f);
    ecrireblock(f,i,"963#$737#$");
    i=allocblock(&f);
    ecrireblock(f,i,"825#$984#$");
    i=allocblock(&f);
    ecrireblock(f,i,"12#123#$24#$56");
    i=allocblock(&f);
    ecrireblock(f,i,"963#$737#$");
    i=allocblock(&f);
    ecrireblock(f,i,"825#$984#$");
    i=allocblock(&f);
    ecrireblock(f,i,"12#123#$24#$56");
    i=allocblock(&f);
    ecrireblock(f,i,"963#$737#$");
    i=allocblock(&f);
    ecrireblock(f,i,"825#$984#$");
    */
    
    update_gui();

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}