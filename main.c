#include <stdlib.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <glib/gi18n.h>
#include <pthread.h>

GtkWidget *drawingarea = NULL;

gulong video_area_xid = 0;

char *filename = NULL;
char stt = 0;

char campause = 0;

pthread_t tidmp3 = 0,tidcam = 0, tidrec = 0;
GtkWidget *label1 = NULL;


int mp3 (char *musica);
int pausepipe ();
int playpipe ();
int endpipe();

void playtrack ();
void* play();

static void helloWorld (GtkWidget *wid, GtkWidget *win)
{
    gint res = 0;
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

void playtrack ()
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

void camera ()
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

void recvideo ()
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

void camerastop()
{
    endcam();
    tidcam = 0;
    campause = 0;
}

static void video_area_realize_cb (GtkWidget * widget, gpointer data)
{
#if GTK_CHECK_VERSION(2,18,0)
  // This is here just for pedagogical purposes, GDK_WINDOW_XID will call
  // it as well in newer Gtk versions
  if (!gdk_window_ensure_native (widget->window))
    g_error ("Couldn't create native window needed for GstXOverlay!");
#endif

#ifdef GDK_WINDOWING_X11
    GtkWidget *win = (GtkWidget *) data;
    video_area_xid = GDK_WINDOW_XID (gtk_widget_get_window (win));
#endif
}

int main (int argc, char *argv[])
{
    GtkWidget *button = NULL;
    GtkWidget *win = NULL;
    GtkWidget *vbox = NULL;
    GtkWidget *video_area = NULL;

    /* Initialize GTK+ */
    g_log_set_handler ("Gtk", G_LOG_LEVEL_WARNING, (GLogFunc) gtk_false, NULL);
    gtk_init (&argc, &argv);
    g_log_set_handler ("Gtk", G_LOG_LEVEL_WARNING, g_log_default_handler, NULL);

    /* Create the main window */
    win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_container_set_border_width (GTK_CONTAINER (win), 8);
    gtk_window_set_title (GTK_WINDOW (win), "Player - douglasjfm");
    gtk_window_resize(GTK_WINDOW (win),250,130);
    gtk_window_set_position (GTK_WINDOW (win), GTK_WIN_POS_CENTER);
    gtk_widget_realize (win);
    g_signal_connect (win, "destroy", gtk_main_quit, NULL);

    /* Create a vertical box with buttons */
    vbox = gtk_vbox_new (TRUE, 6);
    gtk_container_add (GTK_CONTAINER (win), vbox);

    button = gtk_button_new_from_stock ("Escolher");
    g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (helloWorld), (gpointer) win);
    gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0);

    button = gtk_button_new_from_stock ("Tocar/Pausar");
    g_signal_connect (button, "clicked", G_CALLBACK(playtrack), NULL);
    gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0);

    button = gtk_button_new_from_stock ("Cam");
    g_signal_connect (button, "clicked", camera, NULL);
    gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0);

    button = gtk_button_new_from_stock ("Stop");
    g_signal_connect (button, "clicked", camerastop, NULL);
    gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0);

    button = gtk_button_new_from_stock ("Rec");
    g_signal_connect (button, "clicked", recvideo, NULL);
    gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0);

    label1 = gtk_label_new("...Nenhum arquivo");
    gtk_box_pack_start (GTK_BOX (vbox), label1, TRUE, TRUE, 0);

    button = gtk_button_new_from_stock ("Fechar");
    g_signal_connect (button, "clicked", gtk_main_quit, NULL);
    gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0);

    video_area = gtk_drawing_area_new();
    g_signal_connect(video_area, "realize",G_CALLBACK (video_area_realize_cb),win);
    gtk_widget_set_double_buffered(video_area,FALSE);
    gtk_container_add(GTK_CONTAINER(vbox),video_area);

    /* Enter the main loop */
    gtk_widget_show_all (win);
    gtk_main ();
    return 0;
}
