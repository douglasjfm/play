#include <gst/gst.h>
#include <stdlib.h>
#include <string.h>

GstElement *pipeline_a = NULL;

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
    GstElement *source_a, *sink_a, *sinkrtcp, *sink3_a, *enc_a, *conv_a;
    GMainLoop *app_loop;

    GstCaps *caps;

    GstBus *bus;

    GstMessage *msg;
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
    sink_a = gst_element_factory_make("autoaudiosink",NULL);
    /* Create the empty pipeline */
    pipeline_a = gst_pipeline_new("audio-pipeline");

    g_object_set(source_a,"location",musica,NULL);

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

    if (gst_element_link_many(source_a, enc_a, conv_a, sink_a, NULL) != TRUE)
    {
        g_printerr("Elements audio could not be linked.\n");
        gst_object_unref(pipeline_a);
        return -1;
    }

    //g_signal_connect(demux, "pad-added", G_CALLBACK(pad_added_handler), dec);
    //g_signal_connect(pipeline_a, "pad-added", G_CALLBACK(pad_added_handler), NULL);//tratar send_rtp_src tratar send_rtcp_src

    /* Start playing */
    ret = gst_element_set_state(pipeline_a, GST_STATE_PLAYING);

    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        g_printerr("Unable to set the pipeline to the playing state.\n");
        gst_object_unref(pipeline_a);
        return -1;
    }

    /* Wait until error or EOS */
    bus = gst_element_get_bus(pipeline_a);
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
