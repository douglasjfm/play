#include <gst/gst.h>
#include <stdlib.h>
#include <string.h>
#include <gst/interfaces/xoverlay.h>

GstElement *pipeline_a = NULL;
extern gulong wave_area_xid;

static GstBusSyncReply bus_sync_handler2 (GstBus * bus, GstMessage * message, gpointer user_data)
{
// ignore anything but 'prepare-xwindow-id' element messages
    if (GST_MESSAGE_TYPE (message) != GST_MESSAGE_ELEMENT)
        return GST_BUS_PASS;
    if (!gst_structure_has_name (message->structure, "prepare-xwindow-id"))
        return GST_BUS_PASS;

    if (wave_area_xid != 0)
    {
        GstXOverlay *xoverlay;
        // GST_MESSAGE_SRC (message) will be the video sink element
        xoverlay = GST_X_OVERLAY (GST_MESSAGE_SRC (message));
        gst_x_overlay_set_window_handle (xoverlay, wave_area_xid);
    }
    else
    {
        g_warning ("Should have obtained video_window_xid by now!");
    }

    gst_message_unref (message);
    return GST_BUS_DROP;
}

int endpipe()//gera um erro para finalizar o pipeline
{
    if (pipeline_a){
        GstElement *src = gst_bin_get_by_name(GST_BIN(pipeline_a),"mp3src");
        GstElement *dest = gst_bin_get_by_name(GST_BIN(pipeline_a),"mp3dec");
        gst_element_unlink(src,dest);
    }
    return 0;
}

int pausepipe()
{
    gst_element_set_state(pipeline_a,GST_STATE_PAUSED);
    return 0;
}

int playpipe()
{
    gst_element_set_state(pipeline_a,GST_STATE_PLAYING);
    return 0;
}

int mp3 (char *musica)
{
    GstElement *source_a=0, *sink_a=0, *sinkrtcp=0, *sink3_a=0, *enc_a=0, *conv_a=0;
    GstElement *tee_a=0, *fila[2], *scope=0, *scpfil, *scpsink=0;
    GMainLoop *app_loop=0;
    GstElement *balaio = NULL;
    GstElement *pipescop = NULL;
    GstCaps *caps=0;

    GstBus *bus=0, *busscp=0;

    GstMessage *msg=0;
    GstStateChangeReturn ret;

    gboolean link_ok;

    //GMain Loop
    app_loop = g_main_loop_new (NULL, FALSE);

    /* Initialize GStreamer */
    gst_init(NULL, NULL);

    /* Create the elements */
    source_a = gst_element_factory_make("filesrc","mp3src");
    conv_a = gst_element_factory_make("audioconvert",NULL);
    enc_a = gst_element_factory_make("mad","mp3dec");
    tee_a = gst_element_factory_make("tee","t");
    fila[0] = gst_element_factory_make("queue",NULL);
    fila[1] = gst_element_factory_make("queue",NULL);
    scope = gst_element_factory_make("wavescope",NULL);
    scpfil = gst_element_factory_make("ffmpegcolorspace",NULL);
    scpsink = gst_element_factory_make("autovideosink",NULL);
    sink_a = gst_element_factory_make("autoaudiosink",NULL);
    /* Create the empty pipeline */
    pipeline_a = gst_pipeline_new("audio-pipeline");
    pipescop = gst_pipeline_new("pipe-scopio");
    balaio = gst_pipeline_new("balaio");

    g_object_set(source_a,"location",musica,NULL);
    g_object_set(scope,"shader",0,"style",3,NULL);

    if (!pipeline_a || !source_a || !sink_a || !enc_a || !conv_a)
    {
        g_printerr("Not all elements could be created.");
        return -1;
    }

    /* Build the pipeline */
    gst_bin_add(GST_BIN(pipeline_a), source_a);
    gst_bin_add(GST_BIN(pipeline_a), conv_a);
    gst_bin_add(GST_BIN(pipeline_a), enc_a);
    gst_bin_add(GST_BIN(pipeline_a), sink_a);
    gst_bin_add(GST_BIN(pipeline_a), fila[0]);

    gst_bin_add(GST_BIN(pipescop), fila[1]);
    gst_bin_add(GST_BIN(pipescop), scope);
    gst_bin_add(GST_BIN(pipescop), scpsink);
    gst_bin_add(GST_BIN(pipescop), scpfil);

    gst_bin_add(GST_BIN(balaio), pipescop);
    gst_bin_add(GST_BIN(balaio), tee_a);
    gst_bin_add(GST_BIN(balaio), pipeline_a);

    if (gst_element_link_many(source_a, enc_a, conv_a, tee_a, fila[0], sink_a, NULL) != TRUE)
    {
        g_printerr("Elements audio could not be linked.\n");
        gst_object_unref(pipeline_a);
        return -1;
    }

    if (gst_element_link_many(fila[1],scope,scpfil,scpsink, NULL) != TRUE)
    {
        g_printerr("Elements audio could not be linked.\n");
        gst_object_unref(pipeline_a);
        return -1;
    }
    else
    {
        gst_element_link(tee_a,fila[1]);
        printf("scope ok\n");
    }


    busscp = gst_element_get_bus(pipescop);
    bus = gst_element_get_bus(pipeline_a);
    gst_bus_set_sync_handler(bus,(GstBusSyncHandler) NULL,NULL);
    gst_bus_set_sync_handler(bus,(GstBusSyncHandler) bus_sync_handler2,NULL);

    /* Start playing */
    ret = gst_element_set_state(balaio, GST_STATE_PLAYING);

    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        g_printerr("Unable to set the pipeline to the playing state.\n");
        gst_object_unref(pipeline_a);
        return -1;
    }

    /* Wait until error or EOS */
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    g_main_loop_run (app_loop);

    /* Parse message */

    gst_message_unref(msg);


    /* Free resources */
    gst_object_unref(bus);
    gst_element_set_state(pipeline_a, GST_STATE_NULL);
    gst_object_unref(pipeline_a);
    g_main_loop_unref(app_loop);
    return 0;
}
