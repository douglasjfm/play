#include <stdlib.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gmodule.h>
#include <glib/gi18n.h>
#include <pthread.h>

GtkWidget *drawingarea = NULL;

gulong video_area_xid = 0, wave_area_xid = 0;

char *filename = NULL;
char stt = 0;

char campause = 0;

pthread_t tidmp3 = 0,tidcam = 0, tidrec = 0;
GtkWidget *label1 = NULL;


int mp3 (char *musica);
int pausepipe();
int playpipe();
int endpipe();

void playtrack ();
void* play();

G_MODULE_EXPORT void rotulo1(GtkObject *wid, gpointer data)
{
    label1 = GTK_WIDGET(wid);
    printf("Label ok\n");
}

G_MODULE_EXPORT void helloWorld (GtkObject *wid, gpointer data)
{
    gint res = 0;
    GtkWidget *win = gtk_widget_get_ancestor(wid,GTK_TYPE_WINDOW);
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
        GtkWidget *pane = gtk_widget_get_ancestor(wid,GTK_TYPE_BOX);
        GList * botoes = gtk_container_children(pane);
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
    return NULL;
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
  if (!gdk_window_ensure_native ((GTK_WIDGET(widget))->window))
    g_error ("Couldn't create native window needed for GstXOverlay!");
#endif

#ifdef GDK_WINDOWING_X11
    GtkWidget *win = (GtkWidget *) widget;
    video_area_xid = GDK_WINDOW_XID (gtk_widget_get_window (win));
    printf("xcamvid ok\n");
#endif
}

G_MODULE_EXPORT void wave_area_realize_cb (GtkObject * widget, gpointer data)
{
#if GTK_CHECK_VERSION(2,18,0)
  // This is here just for pedagogical purposes, GDK_WINDOW_XID will call
  // it as well in newer Gtk versions
  if (!gdk_window_ensure_native ((GTK_WIDGET(widget))->window))
    g_error ("Couldn't create native window needed for GstXOverlay!");
#endif

#ifdef GDK_WINDOWING_X11
    GtkWidget *win = (GtkWidget *) widget;
    wave_area_xid = GDK_WINDOW_XID (gtk_widget_get_window (win));
    printf("xwave ok\n");
#endif
}

int main (int argc, char *argv[])
{
    GtkBuilder *builder = NULL;
    GError *error = NULL;
    GtkWidget *win = NULL;

    /* Initialize GTK+ */
    g_log_set_handler ("Gtk", G_LOG_LEVEL_WARNING, (GLogFunc) gtk_false, NULL);
    gtk_init (&argc, &argv);
    g_log_set_handler ("Gtk", G_LOG_LEVEL_WARNING, g_log_default_handler, NULL);

    builder = gtk_builder_new();
    if (!gtk_builder_add_from_file(builder,"Main.glade",&error))
    {
        g_warning("P1 Erro: %s",error->message);
        g_free(error);
        return 0xE;
    }

    win = GTK_WIDGET(gtk_builder_get_object(builder,"janela"));
    gtk_builder_connect_signals(builder, builder);
    gtk_widget_show_all (win);
    gtk_main ();

    /* Create the main window */
//    win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
//    gtk_container_set_border_width (GTK_CONTAINER (win), 8);
//    gtk_window_set_title (GTK_WINDOW (win), "Player - douglasjfm");
//    gtk_window_resize(GTK_WINDOW (win),640,480);
//    gtk_window_set_position (GTK_WINDOW (win), GTK_WIN_POS_CENTER);
//    gtk_widget_realize (win);
//    g_signal_connect (win, "destroy", gtk_main_quit, NULL);
//
//    /* Create a vertical box with buttons */
//    vbox = gtk_vbox_new (TRUE, 6);
//
//    /*cria uma panned vertical */
//    panned = gtk_hpaned_new();
//    gtk_paned_add1(GTK_PANED(panned), vbox);
//
//    gtk_container_add (GTK_CONTAINER (win), panned);
//
//    button = gtk_button_new_from_stock ("Escolher");
//    g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (helloWorld), (gpointer) win);
//    gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0);
//
//    button = gtk_button_new_from_stock ("Tocar/Pausar");
//    g_signal_connect (button, "clicked", G_CALLBACK(playtrack), NULL);
//    gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0);
//
//    button = gtk_button_new_from_stock ("Cam");
//    g_signal_connect (button, "clicked", camera, NULL);
//    gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0);
//
//    button = gtk_button_new_from_stock ("Stop");
//    g_signal_connect (button, "clicked", camerastop, NULL);
//    gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0);
//
//    button = gtk_button_new_from_stock ("Rec");
//    g_signal_connect (button, "clicked", recvideo, NULL);
//    gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0);
//
//    label1 = gtk_label_new("...Nenhum arquivo");
//    gtk_box_pack_start (GTK_BOX (vbox), label1, TRUE, TRUE, 0);
//
//    button = gtk_button_new_from_stock ("Fechar");
//    g_signal_connect (button, "clicked", gtk_main_quit, NULL);
//    gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0);
//
//    /* configura a janela video */
//    video_area = gtk_drawing_area_new();
//    gtk_drawing_area_size(video_area,320,240);
//    g_signal_connect(video_area,"realize",G_CALLBACK (video_area_realize_cb),video_area);
//    gtk_widget_set_double_buffered(video_area,FALSE);
//    gtk_paned_add2(GTK_PANED(panned), video_area);

    return 0;
}
