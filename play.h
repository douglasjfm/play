#ifndef PLAY_H_INCLUDED
#define PLAY_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <gtk/gtkwidget.h>
#include <gtk/gtkprogressbar.h>
#include <gdk/gdkx.h>
#include <gmodule.h>
#include <glib/gi18n.h>
#include <pthread.h>

GtkBuilder *builder;
GtkWidget *win;

typedef GObject GtkObject;


int endcam();
int pausecam();
int playcam();
void stopgravar();
void gravar();
int cam ();

void* initserv();
void* chamar (char *ip);
void getlocalip (char *ret);

void setprogresso(guint pos, guint len, char *tmp);
void set_pos_track(float newpos);

int mp3 (char *musica);
int pausepipe();
int playpipe();
int endpipe();

#endif // PLAY_H_INCLUDED
