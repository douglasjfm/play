#include "play.h"

#define UIGLADE "/home/douglas/Área de Trabalho/TrackMP3/Main.glade"

int main (int argc, char *argv[])
{
    GError *error = NULL;

    /* Initialize GTK+ */
    gtk_init (&argc, &argv);

    g_log_set_handler ("Gtk", G_LOG_LEVEL_WARNING, (GLogFunc) gtk_false, NULL);
    g_log_set_handler ("Gtk", G_LOG_LEVEL_WARNING, g_log_default_handler, NULL);

    builder = gtk_builder_new();
    if (!gtk_builder_add_from_file(builder,UIGLADE,&error))
    {
        g_warning("main(): Erro: %s",error->message);
        g_free(error);
        return 0xE;
    }

    win = GTK_WIDGET(gtk_builder_get_object(builder,"janela"));
    gtk_builder_connect_signals(builder, builder);
    gtk_widget_show_all (win);
    gtk_main ();
    return 0;
}
