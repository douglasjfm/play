#include <stdlib.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <gtk/gtkwidget.h>
#include <gtk/gtkprogressbar.h>
#include <gdk/gdkx.h>
#include <gmodule.h>
#include <glib/gi18n.h>
#include <pthread.h>
#include "play.h"

#define UIGLADE "/home/douglas/Área de Trabalho/TrackMP3/Main.glade"

GtkWidget *win = NULL;

GtkWidget *drawingarea = NULL;
GtkProgressBar *barra;
GtkWidget *eventbox;

gulong video_area_xid = 0, wave_area_xid = 0;

char *filename = NULL;
char stt = 0;

char campause = 0;

pthread_t tidmp3 = 0,tidcam = 0, tidrec = 0;
GtkWidget *label1 = NULL;
GtkWidget *tracklabel = NULL;

void playtrack ();
void* play();

void setprogresso(guint pos, guint len, char *tmp)
{
    double f = ((double)pos/len);
    int i;
    gtk_progress_bar_set_fraction(barra,f);
    for (i=0;tmp[i]!='.';i++);
    tmp[i] = '\0';
    gtk_label_set(GTK_LABEL(tracklabel),tmp);
    fflush(stdout);
}

G_MODULE_EXPORT void set_track_pos(GtkWidget *widget, GdkEvent *ev, gpointer user_data)
{
    float newpos = ev->button.x/450;
    printf("main: %f\n",newpos);
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
    GtkStyle *sty = gtk_style_new();
    gtk_draw_flat_box(sty,GDK_WINDOW(widget),GTK_STATE_INSENSITIVE,GTK_SHADOW_ETCHED_IN,0,0,100,100);
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

    /* Initialize GTK+ */
    gtk_init (&argc, &argv);

    g_log_set_handler ("Gtk", G_LOG_LEVEL_WARNING, (GLogFunc) gtk_false, NULL);
    g_log_set_handler ("Gtk", G_LOG_LEVEL_WARNING, g_log_default_handler, NULL);

    builder = gtk_builder_new();
    if (!gtk_builder_add_from_file(builder,UIGLADE,&error))
    {
        g_warning("P1 Erro: %s",error->message);
        g_free(error);
        return 0xE;
    }

    win = GTK_WIDGET(gtk_builder_get_object(builder,"janela"));
    gtk_builder_connect_signals(builder, builder);
    gtk_widget_show_all (win);
    gtk_main ();

    return 0;
}
