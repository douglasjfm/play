#include <gst/gst.h>
#include <gst/video/videooverlay.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>
#include <X11/extensions/Xfixes.h>

extern GtkWidget *drawingarea;

GstElement *pipemaster = NULL;
GMainLoop *app_loop, *rec_loop;
GstCaps *caps;
int recfim = 0;

int endcam()//gera um erro para finalizar o pipeline
{
    if (pipemaster){
        //GstElement *src = gst_bin_get_by_name(GST_BIN(pipeline_v),"mp3src");
        //GstElement *dest = gst_bin_get_by_name(GST_BIN(pipeline_v),"mp3dec");
        //gst_element_unlink(src,dest);
        g_main_loop_quit(app_loop);
    }
    return 0;
}

int pausecam()
{
    gst_element_set_state(pipemaster,GST_STATE_PAUSED);
    return 0;
}

int playcam()
{
    gst_element_set_state(pipemaster,GST_STATE_PLAYING);
    return 0;
}

void stopgravar()
{
    g_main_loop_quit(rec_loop);
}

int fimtredrec()
{
    return (recfim == 1);
}

void link_rec()
{
    unsigned int ret;
    GstElement *t = gst_bin_get_by_name(pipemaster,"t1");
    GstElement *qq = gst_bin_get_by_name(GST_BIN(pipemaster),"fila_r");
    gst_element_set_state(pipemaster,GST_STATE_PAUSED);
    ret = gst_element_link(t,qq);
    ret <<= 8;
    ret <<= 8;
    ret += gst_element_set_state(pipemaster,GST_STATE_PLAYING);
    gst_bin_recalculate_latency(pipemaster);
    g_print("Gravando2...lnk: %d, state_r: %d, state_m: %d\n",ret>>16,(ret>>8)&0xff,ret&0xf);
}

void gravar()
{
    if (pipemaster) {
        GstPad *pad;
        g_print("Gravando...\n");
        recfim = 0;
        GstElement *conv2 = gst_element_factory_make("ffmpegcolorspace","conv2");
        GstElement *qq = gst_element_factory_make("queue","fila_r");
        GstElement *enc = gst_element_factory_make("theoraenc","encvideo");
        GstElement *mux = gst_element_factory_make("oggmux","muxvideo");
        GstElement *fsink = gst_element_factory_make("filesink","filevideo");
        g_object_set(fsink,"location","video.ogg",NULL);

        if (enc && mux && fsink) gst_bin_add_many(GST_BIN(pipemaster),qq,conv2,enc,mux,fsink,NULL);
        else exit(0x123);
        gst_element_link_filtered(qq,conv2,gst_caps_new_simple ("video/x-raw-yuv",
                                "width", G_TYPE_INT, 640,
                                "height", G_TYPE_INT, 480,
                                NULL));
        gst_element_link_many(conv2,enc,mux,fsink,NULL);
        link_rec();
    }
}

int cam ()
{
    GstElement *source_v, *sink_v, *conv_v, *tee_v, *qq_v;

    GstBus *bus;

    GstMessage *msg;
    GstStateChangeReturn ret;

    //GMain Loop
    app_loop = g_main_loop_new (NULL, FALSE);

    /* Initialize GStreamer */
    gst_init(NULL, NULL);

    caps = gst_caps_new_simple ("video/x-raw-yuv",
                                "width", G_TYPE_INT, 640,
                                "height", G_TYPE_INT, 480,
                                NULL);

    /* Create the elements */
    source_v = gst_element_factory_make("v4l2src","mp3src");
    conv_v = gst_element_factory_make("ffmpegcolorspace","mp3dec");
    tee_v = gst_element_factory_make("tee","t1");
    qq_v = gst_element_factory_make("queue","fila_v");
    sink_v = gst_element_factory_make ("autovideosink", "videosink");
    /* Create the empty pipeline */
    pipemaster = gst_pipeline_new("pipemaster");

    if (!pipemaster || !source_v || !tee_v || !conv_v || !sink_v || !qq_v)
    {
        g_printerr("Not all elements could be created.");
        return -1;
    }

    gst_bin_add(GST_BIN(pipemaster), sink_v);
    gst_bin_add(GST_BIN(pipemaster), qq_v);
    gst_bin_add(GST_BIN(pipemaster), conv_v);

    gst_bin_add(GST_BIN(pipemaster), tee_v);
    gst_bin_add(GST_BIN(pipemaster), source_v);

    //gst_bin_add(pipemaster,pipeline_v);

    if (gst_element_link(source_v,tee_v) != TRUE)
    {
        g_printerr("Elements Source and Tee could not be linked with caps.\n");
        gst_object_unref(pipemaster);
        return -1;
    }
    if (gst_element_link_filtered(tee_v,qq_v,caps) != TRUE)
    {
        g_printerr("Elements tee and conv could not be linked with caps1.\n");
        gst_object_unref(pipemaster);
        return -1;
    }
    if (gst_element_link_filtered(qq_v,conv_v,caps) != TRUE)
    {
        g_printerr("Elements qq and conv could not be linked with caps1.\n");
        gst_object_unref(pipemaster);
        return -1;
    }
    if (gst_element_link(conv_v,sink_v)!= TRUE)
    {
        g_printerr("Elements conv and sink could not be linked with caps.\n");
        gst_object_unref(pipemaster);
        return -1;
    }

    //g_signal_connect(source_v,"prepare-format",G_CALLBACK(videoiniciado),(gpointer)pipeline_v);

    /* Start playing */
    ret = gst_element_set_state(pipemaster, GST_STATE_PLAYING);
    //gravar();

    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        g_printerr("Unable to set the pipeline to the playing state.\n");
        gst_object_unref(pipemaster);
        return -1;
    }

    /* Wait until error or EOS */
    bus = gst_element_get_bus(pipemaster);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, (GstMessageType)(GST_MESSAGE_STATE_CHANGED | GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    g_main_loop_run(app_loop);

    /* Parse message */
    gst_message_unref(msg);

    /* Free resources */
    gst_object_unref(bus);
    gst_element_set_state(pipemaster, GST_STATE_NULL);
    gst_object_unref(pipemaster);
    pipemaster = NULL;
    g_main_loop_unref(app_loop);
    g_print("fim camera");
    return 0;
}
