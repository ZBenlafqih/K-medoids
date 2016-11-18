#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <gtk/gtk.h>

int max_x = 750;
int max_y = 450;
int iter = 0; //nombre total de surface
int im = 0; //surface actuel
int edit = 0, del = 0;


typedef struct{
    int x;
    int y;
}point;

typedef struct{
    point p;
    int indice;
    struct input *nxt;
}input;

typedef struct{
    point p;
    int indice;
    struct cluster *nxt;
}cluster;

typedef struct{
    point medoide;
    cluster *cl;
    int indice;
    struct output *nxt;
}output;

static cairo_surface_t *surface[100] = {NULL};
const nbiter = 100;


input *in;
output *out;


/*****************

Fin des fonctions liées à K_medoides

**************************/



void show_warning(GtkWidget *widget, gpointer window, gchar *message, gchar *title)
{
  GtkWidget *dialog;
  dialog = gtk_message_dialog_new(GTK_WINDOW(window),
            GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_OK,
            message);
  gtk_window_set_title(GTK_WINDOW(dialog), title);
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
}

static void clear_surface (int i)
{
    cairo_t *cr;
    cr = cairo_create (surface[i]);
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);
    cairo_destroy (cr);
}


/* Draw a rectangle on the surface at the given position */
static void draw_brush (GtkWidget *widget, gdouble    x, gdouble    y, gdouble    mx, gdouble    my, double r, double g, double b)
{

    cairo_t *cr;

    /* Paint to the surface, where we store our state */
    cr = cairo_create (surface[iter]);

    cairo_set_source_rgb (cr,  r, g, b);

    cairo_rectangle (cr, x, y, 3, 3);

    cairo_fill (cr);

    cairo_set_line_width(cr, 0.7);
    cairo_move_to(cr,x+1.5,y+1.5);
    cairo_line_to(cr,mx+1.5,my+1.5);
    cairo_stroke(cr);

    cairo_destroy (cr);
}


void draw_out(gpointer data, output *out)
{
    clear_surface(iter);

    gtk_widget_queue_draw_area (data,0,0,max_x,max_y);

    srand(time(0));
    output *outC = out;
    cluster *clC;
    int x=0, y=0;
    int mx=0, my=0;
    double a=0, b=0 , c=0 ;
    while(outC!=NULL)
    {
        mx = outC->medoide.x;
        my = outC->medoide.y;
        clC = outC->cl;
        //generer une couleur aleatoire
        a = (rand()%255)/255., b = (rand()%255)/255., c = (rand()%255)/255.;

        while(clC!=NULL)
        {

            x=clC->p.x;
            y=clC->p.y;
            if(outC->indice==0)      draw_brush (data, x, y, mx, my,1,0,0);
            else if(outC->indice==1) draw_brush (data, x, y, mx, my,0,1,0);
            else if(outC->indice==2) draw_brush (data, x, y, mx, my,0,0,1);
            else if(outC->indice==3) draw_brush (data, x, y, mx, my, 0.3,0.3,0.3);
            else if(outC->indice==4) draw_brush (data, x, y, mx, my,0.84,0.84,0);
            else if(outC->indice==5) draw_brush (data, x, y, mx, my,0.81,0.12,0.56);
            else if(outC->indice==6) draw_brush (data, x, y, mx, my, 0.41,0.55,0.13);
            else if(outC->indice==7) draw_brush (data, x, y, mx, my,0.51,0.51,0.51);
            else                     draw_brush (data, x, y,mx,my,a,b,c);
            clC = clC->nxt;
        }

        outC=outC->nxt;
    }

}

/*
 * fonction de distance entre deux point (on l'utilise dans la fonction affecter)
 *
*/
float distance(point a, point b){
    return sqrt(pow((a.x-b.x),2)+pow((a.y-b.y),2));}


/*
 *
 * Fonction d'initialisation des points
 *
*/
input* initialiser_input(int nbr)
{
    in = (input*)malloc(sizeof(input));
    input *element;
    point p;
    srand(time(0));
    int i=0;

    in->p.x = 8+rand()%(max_x-18);
    in->p.y = 8+rand()%(max_y-18);
    in->indice = 0;
    in->nxt = NULL;
    input *inCopie = in;
    for(i=1;i<nbr;i++)
    {
        element = (input*)malloc(sizeof(input));
        element->p.x = 8+rand()%(max_x-18);
        element->p.y = 8+rand()%(max_y-18);
        element->indice = i;
        element->nxt = NULL;
        inCopie->nxt = element;
        inCopie = inCopie->nxt;
    }

    return in;
}

/*
 *
 * Fonction d'initialisation des medoids
 *
*/
output* initialiser_output(input *in, int nbr)
{
    output *out = (output*)malloc(sizeof(output));
    output *element;
    point p;

    int i=0;

    out->medoide.x = in->p.x;
    out->medoide.y = in->p.y;
    out->indice = 0;
    out->cl = NULL;
    out->nxt = NULL;
    output *outCopie = out;
    for(i=1;i<nbr;i++)
    {
        in = in->nxt;
        element = (output*)malloc(sizeof(output));
        element->medoide.x = in->p.x;
        element->medoide.y = in->p.y;
        element->cl= NULL;
        element->indice = i;
        element->nxt = NULL;
        outCopie->nxt = element;
        outCopie = outCopie->nxt;
    }

    return out;
}


/*
 * affecter chaque objet restant au medoide le plus proche
 *
*/
output* affecter(input *in, output *out){
	int min_indice; //medoide minimum
	float min_dis;
    output *outCopie;
    cluster *nvcl,*clC;
    //empty clusters
    output *outC = out;
    while(outC!=NULL)
    {
        outC->cl = NULL;
        outC = outC ->nxt;
    }
    while(in!=NULL)
    {
        nvcl = (cluster*)malloc(sizeof(cluster));
        nvcl->p = in->p;
        nvcl->nxt = NULL;
        outCopie = out;
        min_dis = distance(nvcl->p,outCopie->medoide);
        min_indice = 0;
        while(outCopie!=NULL)
        {
            if(distance(nvcl->p,outCopie->medoide)<min_dis)
            {
                min_dis = distance(nvcl->p,outCopie->medoide);
                min_indice = outCopie->indice;
            }
            outCopie = outCopie->nxt;
        }
        outCopie = out;
        while(outCopie->indice!=min_indice)
            outCopie = outCopie->nxt;

        if(outCopie->cl==NULL){
            outCopie->cl = nvcl;
        }
        else{
            clC = outCopie->cl;
            while(clC->nxt!=NULL)
                clC = clC->nxt;
            clC->nxt=nvcl;
        }

        in=in->nxt;
    }

	return out;
}

/*
 *
 * Fonction qui calcule le cout de changement d'un medoid par un point
 *
*/
int cout_change(output *out, input *in, point non_medoide, int indice) // indice du medoide courant
{
    float E1 = 0, E2 = 0;
    output *outC = out;
    cluster *clC;

    output *outClone=(output*)malloc(sizeof(output));
    outClone->indice=outC->indice;
    outClone->medoide=outC->medoide;
    outC=outC->nxt;
    output *outCloneC = outClone;
    output *nvout;
    while(outC)
    {
        nvout=(output*)malloc(sizeof(output));
        nvout->indice=outC->indice;
        nvout->medoide=outC->medoide;
        outC=outC->nxt;
        outCloneC->nxt = nvout;
        outCloneC = outCloneC->nxt;
    }
    outCloneC->nxt = NULL;

    outC = out;
    while(outC!=NULL)
    {
        clC = outC->cl;
        while(clC!=NULL){
            E1 += fabs(outC->medoide.x-clC->p.x)+fabs(outC->medoide.y-clC->p.y);
            clC = clC->nxt;
        }
        outC = outC->nxt;
    }

    outCloneC = outClone;
    while(outCloneC->indice!=indice)
        outCloneC = outCloneC->nxt;
    outCloneC->medoide = non_medoide;
    outClone = affecter(in, outClone);
    outCloneC = outClone;
    while(outCloneC!=NULL)
    {
        clC = outCloneC->cl;
        while(clC!=NULL)
        {
            E2 += fabs(outCloneC->medoide.x-clC->p.x)+fabs(outCloneC->medoide.y-clC->p.y);
            clC = clC->nxt;
        }
        outCloneC = outCloneC->nxt;
    }
    return E2-E1;
}

/*
 * Tester si un point est un medoide
*/
int medoide(point p, output *out)
{
    output *outc = out;
	while(outc!=NULL)
    {
        if((p.x==outc->medoide.x)&&(p.y==outc->medoide.y))
        {
            return 1;
        }
        outc = outc->nxt;
    }
	return 0;
}

/*void afficher(output *out)
{
    output *outc = out;
    cluster *clC;
    while(outc!=NULL)
    {
        printf("\n(%d,%d) : ",outc->medoide.x,outc->medoide.y);
        clC = outc->cl;
        while(clC!=NULL)
        {
            printf("(%d,%d) ",clC->p.x,clC->p.y);
            clC = clC->nxt;
        }
        outc = outc->nxt;
    }
    printf("\n-----------\n");
}*/

/*
 *
 * Fonction principale k_medoide
 *
*/
output *k_medoide(input *in,output *out, gpointer data[])
{
    int change,CoutChangement;
    point non_medoide;
    input *inCopie = in;
    output *outCopie;
    input *Zero;
    Zero=(input*)malloc(sizeof(input));
    Zero->nxt = in;
    do{
        change = 0;
        out = affecter(in,out);
        iter++;
        draw_out(data[1], out);
        while(inCopie!=NULL)
        {
            if(medoide(inCopie->p, out)==0)
            {
                non_medoide = inCopie->p;
                outCopie = out;
                while(outCopie!=NULL)
                {
                    CoutChangement = cout_change(out,in,non_medoide,outCopie->indice);
                    if(CoutChangement<0)
                    {
                        outCopie->medoide = non_medoide;
                        out = affecter(in,out);
                        iter++;
                        draw_out(data[1], out);
                        change = 1;
                        inCopie = Zero;
                    }
                    outCopie = outCopie->nxt;
                }
            }
            inCopie = inCopie->nxt;
        }
    }while(change!=0);

    return out;
}

/*****************

Fin des fonctions liées à K_medoides

**************************/



/*****************

Debut des fonctions liées à GTK

**************************/

gboolean timeout(gpointer data)
{

    if(im<iter){
        gtk_widget_queue_draw_area (data,0,0,max_x,max_y);
        im++;
    }else{
        return FALSE;
    }
}

static gboolean configure_event_cb (GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{

    int i=0;
    for(i=0;i<nbiter;i++)
        surface[i] = cairo_image_surface_create_from_png("img/copyright.png");
    cairo_surface_t *img = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 100, 100);

    //Create the cairo context
    cairo_t *cr = cairo_create(img);

    //Initialize the image to black transparent
    cairo_set_source_rgba(cr, 0,0,0, 1);
    cairo_paint(cr);

    //Paint one image
    cairo_set_source_surface(cr, surface[0], 0, 0);
    cairo_paint(cr);
    return TRUE;
}

static gboolean draw_cb (GtkWidget *widget, cairo_t *cr, gpointer data)
{

    if(iter!=0){
        cairo_set_source_surface (cr, surface[im], 0, 0);
        cairo_paint (cr);
    }
    else{
        cairo_set_source_surface (cr, surface[0], 0, 0);
        cairo_paint (cr);
    }
    cairo_set_source_rgb(cr,0,0,0);
    cairo_rectangle(cr, 0, 0, max_x, max_y);
    cairo_stroke(cr);

  return TRUE;
}

/*
 *
 * bouton generer des points
 *
 */
static void button_n_clicked(GtkWidget *widget, gpointer data[])
{
    del = 0;
    int n = atoi(gtk_entry_get_text(data[0]));
    if(n<=0){
        show_warning(widget,GTK_WINDOW(data[3]),"Merci de donner une valeur numerique positive","Erreur : Nombre de point");
        return;
    }

    /*Initialiser les inputs*/
    in=initialiser_input(n);
    iter = 0;
    im = 0;
    free(out); out = NULL;
    clear_surface(0);
    gtk_widget_queue_draw_area (data[1],0,0,max_x,max_y);
    int i=0, x=0, y=0;

    input *inC=in;
    while(inC!=NULL)
    {
        x=inC->p.x;
        y=inC->p.y;
        draw_brush (data[1], x, y, x, y, 0,0,0);
        inC=inC->nxt;
    }

    for(i=5;i<9;i++)
        gtk_widget_set_sensitive(data[i],FALSE);
    gtk_widget_grab_focus(data[2]);
    edit =0;

  return TRUE;
}


/*
 *
 * bouton classifier les points
 *
 */
static void button_p_clicked(GtkWidget *widget, gpointer data[])
{
    del = 0;
    int n = 0;
    int p = atoi(gtk_entry_get_text(data[2]));
    //int n = atoi(gtk_entry_get_text(data[0]));
    input *inC = in;
    char str[70];
    if(in!=NULL)
        while(inC!=NULL){
            n = inC->indice;
            inC = inC->nxt;
        }
    sprintf(str,"Nombre de points doit etre inferieur a %d",n+1);
    if(in==NULL){
        show_warning(widget,GTK_WINDOW(data[3]),"Merci de generer les points a regrouper","Erreur");
        return;
    } else if(p<=0){
        show_warning(widget,GTK_WINDOW(data[3]),"Merci de donner une valeur numerique positive","Erreur : Nombre de groupe");
        return;
    } else if(p>n+1){
        show_warning(widget,GTK_WINDOW(data[3]),str,"Erreur");
        return;
    }

    //clear_surface();
    iter = 0;
    im = 0;

    out = initialiser_output(in,p);
    out = affecter(in,out);
    out = k_medoide(in, out, data);
    g_timeout_add(200, timeout, data[1]);
    edit = 0;
    gtk_widget_set_sensitive(data[4],TRUE);

    int i=0;
    for(i=5;i<9;i++)
        gtk_widget_set_sensitive(data[i],TRUE);
    return TRUE;
}

static void about_activate(GtkWidget *widget, gpointer data)
{
    del = 0;
    GtkBuilder *gtkBuilder;
    GtkWidget *about;
    gtkBuilder = gtk_builder_new();
    gtk_builder_add_from_file(gtkBuilder, "Kmedoid.glade", NULL);
    about = GTK_WIDGET(gtk_builder_get_object(gtkBuilder, "aboutdialog1"));

    //gtk_window_set_title(GTK_WINDOW(about), "A propos");
    gtk_dialog_run(GTK_DIALOG(about));
    gtk_widget_destroy(about);
}

static void Shortcuts_activate(GtkWidget *widget, gpointer data)
{
    del = 0;
    GtkBuilder *gtkBuilder;
    GtkWidget *about;
    gtkBuilder = gtk_builder_new();
    gtk_builder_add_from_file(gtkBuilder, "Kmedoid.glade", NULL);
    about = GTK_WIDGET(gtk_builder_get_object(gtkBuilder, "Shortcutsdialog"));

    //gtk_window_set_title(GTK_WINDOW(about), "A propos");
    gtk_dialog_run(GTK_DIALOG(about));
    gtk_widget_destroy(about);
}



static gchar* open_dialog(GtkWindow *window, GtkFileChooserAction action, gchar *file)
{
    gchar *str;
    GtkBuilder *gtkBuilder;
    GtkFileChooser *select;
    gint res;

    select = gtk_file_chooser_dialog_new ("Choisir votre chemin",
                                      window,
                                      action,
                                      "Annuler",
                                      GTK_RESPONSE_CANCEL,
                                      file,
                                      GTK_RESPONSE_ACCEPT,
                                      NULL);
    res = gtk_dialog_run(GTK_DIALOG(select));
    if(res == GTK_RESPONSE_ACCEPT){
        str = gtk_file_chooser_get_filename(select);
        gtk_widget_destroy(select);
        return str;
    }else{
        gtk_widget_destroy(select);
        return NULL;
    }

}

static void  nv_menu_activate(GtkWidget *widget, gpointer data[])
{
    gtk_widget_set_sensitive(data[4],TRUE);
    free(in); in = NULL;
    free(out); out = NULL;
    iter = 0;
    im = 0;
    gtk_widget_queue_draw_area (data[1],0,0,max_x,max_y);
    clear_surface(0);
    gtk_entry_set_text(data[2],"");
    gtk_entry_set_text(data[0],"");
    gtk_widget_grab_focus(data[0]);
    edit = 0;
    del = 0;
    int i=0;
    for(i=5;i<9;i++)
        gtk_widget_set_sensitive(data[i],FALSE);
}

static void undo_menu_activate(GtkWidget *widget, gpointer data[])
{
    del = 0;
    iter = 0;
    im = 0;
    free(out); out = NULL;
    clear_surface(0);
    gtk_widget_queue_draw_area (data[1],0,0,max_x,max_y);
    int i=0, x=0, y=0;

    input *inC=in;
    while(inC!=NULL)
    {
        x=inC->p.x;
        y=inC->p.y;
        draw_brush (data[1], x, y, x, y, 0,0,0);
        inC=inC->nxt;
    }
    gtk_widget_grab_focus(data[2]);
    edit = 0;
    gtk_widget_set_sensitive(data[4],TRUE);
    for(i=5;i<9;i++)
        gtk_widget_set_sensitive(data[i],FALSE);
}

static int save_menu_activate(GtkWidget *widget, gpointer data)
{
    del = 0;
    gchar *NOM_FICHIER = open_dialog(data,GTK_FILE_CHOOSER_ACTION_SAVE,"Enregistrer");
    if(NOM_FICHIER){
        FILE *P_FICHIER;
        input *tmp_in;
        //strcat(NOM_FICHIER,".txt");
        P_FICHIER = fopen(NOM_FICHIER, "w");
        tmp_in=in;
        while(tmp_in!=NULL){
            fprintf(P_FICHIER,"(%d,%d)\n",tmp_in->p.x,tmp_in->p.y);
            tmp_in=tmp_in->nxt;
        }
        free(tmp_in); tmp_in = NULL;
        fclose(P_FICHIER);
    }
}

static void open_menu_activate(GtkWidget *widget, gpointer data[])
{
    del = 0;
    gchar *NOM_FICHIER = open_dialog(data[3],GTK_FILE_CHOOSER_ACTION_OPEN,"Ouvrir");
    int caractereActuel = 0;
    int a,b;
    if(NOM_FICHIER){
        input* inC=(input*)malloc(sizeof(input));
        input *element;
        input *inCopie;
        int i = 0;
        FILE *P_FICHIER;
        P_FICHIER = fopen(NOM_FICHIER, "r");
        fscanf(P_FICHIER,"(%d,%d)",&a,&b);
        inC->p.x = a%max_x;
        inC->p.y = b%max_y;
        inC->nxt = NULL;
        inC->indice = i;
        i++;
        inCopie = inC;
        caractereActuel = fgetc(P_FICHIER);
        do
        {
            if(fgetc(P_FICHIER)!=EOF){
                    element = (input*)malloc(sizeof(input));
                    fscanf(P_FICHIER,"%d,%d)",&a,&b);
                    element->p.x = a%max_x;
                    element->p.y = b%max_y;
                    element->indice = i;
                    i++;
                    element->nxt = NULL;
                    inCopie->nxt = element;
                    inCopie = inCopie->nxt;
            }
            caractereActuel = fgetc(P_FICHIER);
        } while (caractereActuel != EOF);
        char a[5];
        itoa(i,a,10);
        gtk_entry_set_text(data[0],a);
        fclose(P_FICHIER);
        in = inC;

        iter = 0;
        im = 0;
        free(out); out = NULL;
        clear_surface(0);

        int x, y;

        inC=in;
        while(inC!=NULL)
        {
            x=inC->p.x;
            y=inC->p.y;
            draw_brush (data[1], x, y, x, y, 0,0,0);
            inC=inC->nxt;
        }
        gtk_widget_grab_focus(data[2]);
        gtk_widget_set_sensitive(data[4],FALSE);
        edit =0;
        for(i=5;i<9;i++)
            gtk_widget_set_sensitive(data[i],FALSE);
    }
}

static void edit_menu_activate(GtkWidget *widget, gpointer data[])
{
    undo_menu_activate(widget,data);
    gtk_widget_set_sensitive(data[4],FALSE);
    del = 0;
    edit = 1;
    int i;
    //"for" to disable precedent, next ...
    for(i=5;i<9;i++)
        gtk_widget_set_sensitive(data[i],FALSE);
}

static void delete_menu_activate(GtkWidget *widget, gpointer data[])
{
    undo_menu_activate(widget,data);
    //gtk_widget_set_sensitive(data[4],FALSE);
    del = 1;
    edit = 0;
    int i;
    //"for" to disable precedent, next ...
    for(i=5;i<9;i++)
        gtk_widget_set_sensitive(data[i],FALSE);
}

static void button_press_event(GtkWidget *widget,  GdkEventButton *event, gpointer data[])
{
    if(edit == 1){
        int a = 0;
        input *inC = in;
        char str[70];
        input *nv;
        nv = (input*)malloc(sizeof(input));
        gtk_widget_queue_draw_area(widget,0,0,max_x,max_y);
        draw_brush (widget, event->x, event->y, event->x,event->y,0,0,0);
        nv->p.x = event->x;
        nv->p.y = event->y;
        nv->nxt = NULL;
        if(in==NULL){
            nv->indice=0;
            a = -1 ;
            in = nv;
        }else{
            while(inC->nxt!=NULL){
                a = inC->indice + 1;
                inC = inC->nxt;
            }
            nv->indice = a+1;
            inC->nxt = nv;
        }

        itoa(a+2,str,10);
        gtk_entry_set_text(data[0],str);
        gtk_widget_grab_focus(data[2]);
    }

    else if(del == 1){

        input *inC = in, *inPrec;
        char str[70];
        while(inC!=NULL){
            if( inC->p.x-1 <= event->x && event->x <= inC->p.x + 4 && inC->p.y-1 <= event->y && event->y <= inC->p.y+4)
            {
                gtk_widget_queue_draw_area(data[1],0,0,max_x,max_y);
                draw_brush(data[1],inC->p.x,inC->p.y,inC->p.x,inC->p.y,1,1,1);
                if(inC->indice == 0){
                    if(inC->nxt==NULL)
                        in = NULL;
                    else{
                        in = inC->nxt;
                        in->indice = 0;}
                }
                else{
                    inPrec->nxt = inC->nxt;
                    inC=inPrec;
                }

            }
            inPrec = inC;
            inC = inC->nxt;
        }

        inC = in;
        int i = 0;
        while (inC!=NULL){
            inC->indice = i;
            i++;
            inC = inC->nxt;
        }
        itoa(i,str,10);

        gtk_entry_set_text(data[0],str);
        gtk_widget_grab_focus(data[2]);
        //del = 0;
    }
}

static void deb_tool_clicked(GtkWidget *widget, gpointer data[]){
    gtk_widget_queue_draw_area(data[1],0,0,max_x,max_y);
    im = 1;
}

static void prec_tool_clicked(GtkWidget *widget, gpointer data[]){

    gtk_widget_queue_draw_area(data[1],0,0,max_x,max_y);
    if(im>1) im--;

}

static void suiv_tool_clicked(GtkWidget *widget, gpointer data[]){
    gtk_widget_queue_draw_area(data[1],0,0,max_x,max_y);
    if(im<iter) im++;
}

static void fin_tool_clicked(GtkWidget *widget,  gpointer data[]){
    gtk_widget_queue_draw_area(data[1],0,0,max_x,max_y);
    im = iter;
}


/*****************

Fin des fonctions liées à GTK

**************************/



int main(int argc, char *argv[])
{
    GtkBuilder *gtkBuilder;
    GtkWidget *window_Principal;
    GtkWidget *entry_nbPt, *entry_nbGp, *vbox, *button_nbPt, *button_nbGp, *drawing_area;
    GtkWidget *about_menu, *shortcuts_menu, *quit_menu, *nv_menu, *undo_menu, *save_menu, *open_menu, *edit_menu, *delete_menu;
    GtkWidget *nv_tool, *open_tool, *save_tool, *edit_tool, *delete_tool, *undo_tool, *deb_tool, *prec_tool, *suiv_tool, *fin_tool;

    //initialisation gtk
    gtk_init(&argc, &argv);

    //instanciation des objet
    gtkBuilder = gtk_builder_new();
    gtk_builder_add_from_file(gtkBuilder, "Kmedoid.glade", NULL);

    //load controls from the glade
    window_Principal = GTK_WIDGET(gtk_builder_get_object(gtkBuilder, "window_Principal"));
    button_nbPt = GTK_WIDGET(gtk_builder_get_object(gtkBuilder, "button_nbPt"));
    button_nbGp = GTK_WIDGET(gtk_builder_get_object(gtkBuilder, "button_nbGp"));
    entry_nbPt = GTK_WIDGET(gtk_builder_get_object(gtkBuilder, "entry_nbPt"));
    entry_nbGp = GTK_WIDGET(gtk_builder_get_object(gtkBuilder, "entry_nbGp"));
    vbox = GTK_WIDGET(gtk_builder_get_object(gtkBuilder, "vbox1"));
    about_menu = GTK_WIDGET(gtk_builder_get_object(gtkBuilder, "menuitem_about"));
    shortcuts_menu = GTK_WIDGET(gtk_builder_get_object(gtkBuilder, "menuitem_shortcuts"));
    quit_menu = GTK_WIDGET(gtk_builder_get_object(gtkBuilder, "menuitem_quit"));
    nv_menu = GTK_WIDGET(gtk_builder_get_object(gtkBuilder, "menuitem_nv"));
    nv_tool = GTK_WIDGET(gtk_builder_get_object(gtkBuilder, "toolbutton_nv"));
    undo_menu = GTK_WIDGET(gtk_builder_get_object(gtkBuilder, "menuitem_undo"));
    undo_tool = GTK_WIDGET(gtk_builder_get_object(gtkBuilder, "toolbutton_undo"));
    save_menu = GTK_WIDGET(gtk_builder_get_object(gtkBuilder, "menuitem_save"));
    save_tool = GTK_WIDGET(gtk_builder_get_object(gtkBuilder, "toolbutton_save"));
    open_menu = GTK_WIDGET(gtk_builder_get_object(gtkBuilder, "menuitem_open"));
    open_tool = GTK_WIDGET(gtk_builder_get_object(gtkBuilder, "toolbutton_open"));
    edit_menu = GTK_WIDGET(gtk_builder_get_object(gtkBuilder, "menuitem_edit"));
    edit_tool = GTK_WIDGET(gtk_builder_get_object(gtkBuilder, "toolbutton_edit"));
    delete_menu = GTK_WIDGET(gtk_builder_get_object(gtkBuilder, "menuitem_delete"));
    delete_tool = GTK_WIDGET(gtk_builder_get_object(gtkBuilder, "toolbutton_delete"));
    deb_tool = GTK_WIDGET(gtk_builder_get_object(gtkBuilder, "toolbutton_deb"));
    prec_tool = GTK_WIDGET(gtk_builder_get_object(gtkBuilder, "toolbutton_prec"));
    suiv_tool = GTK_WIDGET(gtk_builder_get_object(gtkBuilder, "toolbutton_suiv"));
    fin_tool = GTK_WIDGET(gtk_builder_get_object(gtkBuilder, "toolbutton_fin"));


    drawing_area = gtk_drawing_area_new ();

    //gtk_container_set_border_width (GTK_CONTAINER (drawing_area), 10);
    gtk_widget_set_size_request(drawing_area, max_x, max_y);
    gtk_box_pack_end(GTK_BOX(vbox),drawing_area,0,0,0);
    gtk_widget_set_margin_left(drawing_area,10);
    gtk_widget_set_margin_right(drawing_area,10);
    gtk_widget_set_margin_bottom(drawing_area,10);


    gpointer data[9];
    data[0] = entry_nbPt;
    data[1] = drawing_area;
    data[2] = entry_nbGp;
    data[3] = window_Principal;
    data[4] = button_nbPt;
    data[5] = deb_tool;
    data[6] = prec_tool;
    data[7] = suiv_tool;
    data[8] = fin_tool;

    //declaration des evenements
    g_signal_connect (window_Principal,"destroy",G_CALLBACK(gtk_main_quit),data);
    g_signal_connect (button_nbPt,"clicked",G_CALLBACK(button_n_clicked), data);
    g_signal_connect (button_nbGp,"clicked",G_CALLBACK(button_p_clicked), data);
    g_signal_connect (drawing_area, "draw", G_CALLBACK (draw_cb), NULL);
    g_signal_connect (drawing_area, "configure-event", G_CALLBACK (configure_event_cb), NULL);
    g_signal_connect (entry_nbPt,"activate",G_CALLBACK (button_n_clicked), data);
    g_signal_connect (entry_nbGp,"activate",G_CALLBACK (button_p_clicked), data);
    g_signal_connect (about_menu,"activate",G_CALLBACK (about_activate), NULL);
    g_signal_connect (shortcuts_menu,"activate",G_CALLBACK (Shortcuts_activate), NULL);
    g_signal_connect (quit_menu,"activate",G_CALLBACK (gtk_main_quit), NULL);
    g_signal_connect (nv_menu,"activate",G_CALLBACK (nv_menu_activate), data);
    g_signal_connect (nv_tool,"clicked",G_CALLBACK (nv_menu_activate), data);
    g_signal_connect (undo_menu,"activate",G_CALLBACK (undo_menu_activate), data);
    g_signal_connect (undo_tool,"clicked",G_CALLBACK (undo_menu_activate), data);
    g_signal_connect (save_menu,"activate",G_CALLBACK (save_menu_activate), window_Principal);
    g_signal_connect (save_tool,"clicked",G_CALLBACK (save_menu_activate), window_Principal);
    g_signal_connect (open_menu,"activate",G_CALLBACK (open_menu_activate), data);
    g_signal_connect (open_tool,"clicked",G_CALLBACK (open_menu_activate), data);
    g_signal_connect (edit_menu,"activate",G_CALLBACK (edit_menu_activate), data);
    g_signal_connect (edit_tool,"clicked",G_CALLBACK (edit_menu_activate), data);
    g_signal_connect (delete_menu,"activate",G_CALLBACK (delete_menu_activate), data);
    g_signal_connect (delete_tool,"clicked",G_CALLBACK (delete_menu_activate), data);
    g_signal_connect (deb_tool,"clicked",G_CALLBACK (deb_tool_clicked), data);
    g_signal_connect (prec_tool,"clicked",G_CALLBACK (prec_tool_clicked), data);
    g_signal_connect (suiv_tool,"clicked",G_CALLBACK (suiv_tool_clicked), data);
    g_signal_connect (fin_tool,"clicked",G_CALLBACK (fin_tool_clicked), data);
    g_signal_connect (drawing_area, "button_press_event",G_CALLBACK(button_press_event), data);

    GdkEventMask event_mask;
    event_mask = GDK_BUTTON_PRESS_MASK;
    gtk_widget_set_events (GTK_WIDGET(data[1]), event_mask);

    gtk_widget_show_all(window_Principal);
    gtk_main();

    return 0;

}
