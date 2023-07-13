#include "play.h"

GtkWidget *drawingarea = NULL;
GtkProgressBar *barra = NULL;
GtkWidget *eventbox = NULL;

gulong video_area_xid = 0, wave_area_xid = 0;

char *filename = NULL, meuip[25],ipcall[25];
char stt = 0;

char campause = 0;

pthread_t tidmp3 = 0,tidcam = 0, tidrec = 0,tidserv = 0,tidtx = 0;
GtkWidget *label1 = NULL;
GtkWidget *tracklabel = NULL;

void playtrack ();
void* play();

char curlogin[100];
static char telstt = 'f';

void setprogresso(guint pos, guint len, char *tmp)
{
    double f = ((double)pos/len);
    int i;
    gtk_progress_bar_set_fraction(barra,f);
    for (i=0;tmp[i]!='.';i++);
    tmp[i] = '\0';
    gtk_label_set_text(GTK_LABEL(tracklabel),tmp);
}

G_MODULE_EXPORT void set_track_pos(GtkWidget *widget, GdkEvent *ev, gpointer user_data)
{
    float newpos = ev->button.x/450;
    set_pos_track(newpos);
}

G_MODULE_EXPORT void progress_bar_realize_cb (GtkObject * widget, gpointer data)
{
    barra = GTK_PROGRESS_BAR(widget);
}

G_MODULE_EXPORT void rotulo1(GtkObject *wid, gpointer data)
{
    label1 = GTK_WIDGET(wid);
    printf("Label ok\n");
}

G_MODULE_EXPORT void rotulo2(GtkObject *wid, gpointer data)
{
    tracklabel = GTK_WIDGET(wid);
    printf("track label ok\n");
}

G_MODULE_EXPORT void helloWorld (GtkObject *wid, gpointer data)
{
    gint res = 0;
    GtkWidget *win = gtk_widget_get_ancestor(GTK_WIDGET(wid),GTK_TYPE_WINDOW);
    GtkWidget *winFile = gtk_file_chooser_dialog_new ("Abrir MP3",
                         GTK_WINDOW(win),
                         GTK_FILE_CHOOSER_ACTION_OPEN,
                         _("_Cancel"),
                         GTK_RESPONSE_CANCEL,
                         _("_Open"),
                         GTK_RESPONSE_ACCEPT,
                         NULL);

    res = gtk_dialog_run (GTK_DIALOG (winFile));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        char txt[100], *tkn, cp[200];
        GtkFileChooser *chooser = GTK_FILE_CHOOSER (winFile);
        filename = gtk_file_chooser_get_filename (chooser);
        strcpy(cp,filename);
        tkn = strtok(filename,"/");
        do
        {
            strcpy(txt,tkn);
            tkn = strtok(NULL,"/");
        }while(tkn);
        gtk_label_set_text(GTK_LABEL(label1),txt);
        strcpy(filename,cp);
        stt = 0x01;
        endpipe();
        play();
    }

    gtk_widget_destroy (winFile);
}

void *novatred()
{
    mp3(filename);
    return NULL;
}

void *callcam()
{
    cam();
    return NULL;
}

void* play()
{
    pthread_create(&tidmp3,NULL,novatred,NULL);
    return NULL;
}

G_MODULE_EXPORT void playtrack (GtkObject *btn, gpointer data)
{
    switch(stt)
    {
    case(0x01):
        stt = 0x00;
        pausepipe();
        break;
    case(0x00):
        stt = 0x01;
        playpipe();
        break;
    }
}

G_MODULE_EXPORT void camera (GtkObject *btn, gpointer data)
{
    if (!tidcam) pthread_create(&tidcam,NULL,&callcam,NULL);
    else
    {
        if (!campause++) pausecam();
        else
        {
            playcam();
            campause = 0;
        }
    }
}

G_MODULE_EXPORT void recvideo (GtkObject *btn, gpointer data)
{
    if (!tidrec)
    {
        gravar();
        tidrec = 1;
    }
    else
    {
        stopgravar();
        tidrec = 0;
    }
}

G_MODULE_EXPORT void camerastop(GtkObject *btn, gpointer data)
{
    endcam();
    tidcam = 0;
    campause = 0;
}

G_MODULE_EXPORT void video_area_realize_cb (GtkObject * widget, gpointer data)
{
#if GTK_CHECK_VERSION(2,18,0)
  // This is here just for pedagogical purposes, GDK_WINDOW_XID will call
  // it as well in newer Gtk versions
  if (!gdk_window_ensure_native (gtk_widget_get_window(GTK_WIDGET(widget))))
    g_error ("Couldn't create native window needed for GstXOverlay!");
#endif

    GtkWidget *win = (GtkWidget *) widget;
    GdkWindow *win_gdk = gtk_widget_get_window (win);
    video_area_xid = gdk_x11_window_get_xid(win_gdk);
}

G_MODULE_EXPORT void wave_area_realize_cb (GtkObject * widget, gpointer data)
{
    GtkStyle *sty = gtk_style_new();
    cairo_t *cctx = gdk_cairo_create(widget);
    gtk_paint_flat_box(sty,cctx,
                       GTK_STATE_INSENSITIVE,
                       GTK_SHADOW_ETCHED_IN,
                       NULL,
                       GTK_WIDGET(widget),
                       "style detail",
                       0,0,100,100);
#if GTK_CHECK_VERSION(2,18,0)
  // This is here just for pedagogical purposes, GDK_WINDOW_XID will call
  // it as well in newer Gtk versions
  if (!gdk_window_ensure_native (gtk_widget_get_window(GTK_WIDGET(widget))))
    g_error ("Couldn't create native window needed for GstXOverlay!");
#endif

    GtkWidget *win = (GtkWidget *) widget;
    GdkWindow *win_gdk = gtk_widget_get_window (win);
    wave_area_xid = gdk_x11_window_get_xid(win_gdk);
}

G_MODULE_EXPORT void fim()
{
    char url[200];
    strcpy(url,"deslogtel.php?login=");
    strcat(url,curlogin);
    if (telstt == 'o')
    {
        httpget("cidadelimpa.bugs3.com",url);
        telstt = 'f';
    }
    gtk_main_quit();
}

G_MODULE_EXPORT void logar_tele()
{
    char *extip, strnome[80],url[150];
    GtkEntry *nome = GTK_ENTRY(gtk_builder_get_object(builder,"entry1"));
    GtkLabel *logstt = GTK_LABEL(gtk_builder_get_object(builder,"label2"));
    strcpy(strnome,gtk_entry_get_text(nome));
    strcpy(curlogin,gtk_entry_get_text(nome));
    strcpy(url,"retip.php?login=");
    strcat(url,strnome);
    httpget("cidadelimpa.bugs3.com",url);
    strcpy(url,"online ");
    gtk_label_set_text(logstt,url);
    telstt = 'o';
    pthread_create(&tidserv,NULL,&initserv,NULL);
}

G_MODULE_EXPORT void call_user()
{
    char *extip, strnome[80],url[150];
    GtkEntry *nome = GTK_ENTRY(gtk_builder_get_object(builder,"entry1"));
    strcpy(ipcall,gtk_entry_get_text(nome));
//    strcpy(url,"getip.php?login=");
//    strcat(url,strnome);
//    extip = httpget("cidadelimpa.bugs3.com",url);
//    printf("usu ip : %s\n",extip);
    if (strlen(ipcall))
        pthread_create(&tidtx,NULL,(void*)&chamar,NULL);
}
