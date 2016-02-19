#include <gst/gst.h>
#include <gst/interfaces/xoverlay.h>
#include <stdlib.h>
#include <string.h>

static void make_request_pad_and_link (GstElement *makePad,const gchar *pad_template, GstElement *linkToBefore, GstElement *linkToAfter, GstCaps *befCaps)
{
    GstPad * pad;
    gchar *name;
    gboolean ret;

    pad = gst_element_get_request_pad (makePad, pad_template);
    name = gst_pad_get_name (pad);
    //g_print ("A new pad %s was created\n", name);
    /* here, you would link the pad */
    if (linkToBefore)
    {
        GstPad *stcpad = gst_element_get_static_pad(linkToBefore,"src");
        if (!befCaps) ret = gst_pad_link(stcpad,pad);
        else
        {
            gst_pad_set_caps(pad,befCaps);
            ret = gst_pad_link(stcpad,pad);
        }
        //    gst_object_unref(stcpad);
    }
    if (linkToAfter)
    {
        GstPad *stcpad = gst_element_get_static_pad(linkToAfter,"sink");
        ret = gst_pad_link(pad,stcpad);
        //   gst_object_unref(stcpad);
    }

    if (ret)
        g_print ("  Pad req '%s' but link failed.\n", name);
    else
        g_print ("  Pad req Link succeeded (type '%s').\n", name);

    g_free (name);
    /* and, after doing that, free our reference */
    //gst_object_unref (GST_OBJECT (pad));
}

static void pad_added_handler(GstElement *src, GstPad *new_pad, GstElement *depay)
{
    GstPad *link = NULL;
    GstPadLinkReturn ret;
    GstCaps *new_pad_caps = NULL;
    GstStructure *new_pad_struct = NULL;
    gchar *name_pad = GST_PAD_NAME(new_pad);
    const gchar *new_pad_type = NULL;

    g_print ("-------->ouvir.c: Received new pad '%s' from '%s':\n", name_pad, GST_ELEMENT_NAME (src));
//	/* Check the new pad's type */
    if (g_str_has_prefix(name_pad,"recv_rtp_src"))
    {
        new_pad_caps = gst_pad_get_caps(new_pad);
        new_pad_struct = gst_caps_get_structure(new_pad_caps, 0);
        new_pad_type = gst_structure_get_name(new_pad_struct);
        name_pad = gst_pad_get_name(new_pad);
    }
    else return;

    /* Attempt the link */

    if (g_str_has_prefix(name_pad,"recv_rtp_src"))
    {
        g_print("->->recv_rtp_src detect\n");
        link = gst_element_get_pad(depay,"sink");

        if (gst_pad_is_linked(link))
        {
            gst_object_unref(link);
            goto exit;
        }
        ret = gst_pad_link(new_pad, link);
    }

    else goto exit;

    if (ret!=888 && GST_PAD_LINK_FAILED (ret))
    {
        g_print ("->->->Type is '%s' but link failed.\n", new_pad_type);
    }
    else
    {
        g_print ("->->->Link succeeded (type '%s').\n", new_pad_type);
    }

exit:
    /* Unreference the new pad's caps, if we got them */
    if (new_pad_caps != NULL)
        gst_caps_unref(new_pad_caps);
    if(link) gst_object_unref(link);
    /* Unreference the sink pad */
}

void *ouvirf(char *ippeer)
{
    GstElement *pipelisten, *rtpbin, *src_a, *rtpdepay, *dec_a, *conv_a, *sink_a, *sink_rtcp, *src_rtcp;

    GMainLoop *listen_loop = g_main_loop_new (NULL, FALSE);

    GstCaps *udpCaps = NULL;

    /* Initialize GStreamer */
    gst_init(NULL, NULL);

    pipelisten = gst_pipeline_new("pipelisten");

    rtpbin = gst_element_factory_make("gstrtpbin","rtpbin");
    src_a = gst_element_factory_make("udpsrc","src_a");
    src_rtcp = gst_element_factory_make("udpsrc","src_rtcp");
    rtpdepay = gst_element_factory_make("rtpopusdepay","rtpdepay");
    dec_a = gst_element_factory_make("opusdec","dec_a");
    conv_a = gst_element_factory_make("audioconvert","conv_a");
    sink_a = gst_element_factory_make("autoaudiosink","sink_a");
    sink_rtcp = gst_element_factory_make("udpsink","sink_rtcp");

    udpCaps = gst_caps_new_simple("application/x-rtp",
                                            "media",G_TYPE_STRING,"audio",
                                            "clock-rate",G_TYPE_INT,48000,
                                            "encoding-name",G_TYPE_STRING,"X-GST-OPUS-DRAFT-SPITTKA-00",NULL);

    g_object_set(src_a,"port",5002,NULL);
    g_object_set(src_rtcp,"port",5003,NULL);
    g_object_set(sink_rtcp,"port",5007,"host",ippeer,"sync",(gboolean) FALSE,"async",(gboolean) FALSE, NULL);

    if (!pipelisten || !rtpbin || !rtpdepay || !src_a || !dec_a || !sink_a || !conv_a || !sink_rtcp)
    {
        g_print("ouvir.c: Elementos nao puderam ser criados");
        return -1;
    }

    gst_bin_add_many(GST_BIN(pipelisten), rtpbin, src_a, rtpdepay, dec_a, conv_a, sink_a, sink_rtcp, NULL);

    if (gst_element_link_many(rtpdepay,dec_a,conv_a,sink_a,NULL) == FALSE)
    {
        g_print("ouvir.c: Elementos nao puderam ser linkados 1");
        return -1;
    }

    g_signal_connect(rtpbin, "pad-added", G_CALLBACK(pad_added_handler), rtpdepay);

    make_request_pad_and_link(rtpbin,"recv_rtp_sink_%d",src_a,NULL,udpCaps);
    make_request_pad_and_link(rtpbin,"send_rtcp_src_%d",NULL,sink_rtcp,NULL);

    g_main_loop_run(listen_loop);
    g_main_loop_unref(listen_loop);

    return 0;
}
