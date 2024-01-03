#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include <gtk/gtk.h>

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
        if (*(supp+(*j-1)*sizeof(bool))==true)
        {
            (*trouv)=false;
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
            
            if(field_index == highlighted_record)
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
            //if(!b[field_index])
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


int main(int argc, char* argv[])
{
    fichier f;
    f.nb_block=0;
    f.taille_block=10;//sans le caractere de fin de chaine
    f.supp_logique=true;
    f.debut=NULL;
    f.fin=NULL;
    bool *test;
    int i=allocblock(&f);
    ecrireblock(f,i,"12#$24#$56\0");
    i=allocblock(&f);
    ecrireblock(f,i,"963#$737#$\0");
    i=allocblock(&f);
    ecrireblock(f,i,"825#$984#$\0");
    bool trouv=false;
    int j;
    recherche("24\0",&trouv,&i,&j,f);
    if (trouv==true)
    {
        printf("la valeur ce trouve dans le block %d et l'enregistrement %d\n",i,j);
    }
    else
    {
        printf("la valeur ne se trouve pas dans le fichier le numero de block avant la valeur est %d et la position est %d",i,j);
    }
    
}