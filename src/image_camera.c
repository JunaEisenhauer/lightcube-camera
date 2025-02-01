#include "image_camera.h"

#include <stdio.h>

#include <gst/gst.h>

#include "image_destination.h"

typedef struct {
    GstElement *source;
    GstElement *sink;
    GstElement *jpegenc;
} ImagePipelineElements;

ImagePipelineElements imageElements;
GstElement *imagePipeline;
GstBus *imageBus;

/**
 * Makes the image pipeline elements.
 *
 * @return the status code of the image element creation
 */
int MakeImageElements() {
    imageElements.source = gst_element_factory_make("appsrc", "source");
    imageElements.sink = gst_element_factory_make("filesink", "sink");
    imageElements.jpegenc = gst_element_factory_make("jpegenc", "jpegenc");

    if (!imageElements.source || !imageElements.sink || !imageElements.jpegenc) {
        g_printerr("Could not create all image elements.\n");
        return -1;
    }

    return 0;
}

/**
 * Adds the image elements to the pipeline.
 */
void AddImageElementsToPipeline() {
    gst_bin_add_many(GST_BIN(imagePipeline), imageElements.source, imageElements.sink, imageElements.jpegenc, NULL);
}

/**
 * Links the image elements in the pipeline.
 *
 * @return the status code of the image element linkage
 */
int LinkImageElementsInPipeline() {
    if (!gst_element_link_many(imageElements.source, imageElements.jpegenc, imageElements.sink, NULL)) {
        g_printerr("Image elements could not be linked.\n");
        gst_object_unref(imagePipeline);
        return -1;
    }

    return 0;
}

/**
 * Sets up the image pipeline.
 *
 * @return the status code of the image pipeline setup
 */
int SetupImagePipeline() {
    imagePipeline = gst_pipeline_new("lightcube-image-pipeline");
    if (!imagePipeline) {
        g_printerr("Could not create image pipeline.\n");
        return -1;
    }

    AddImageElementsToPipeline();
    if (LinkImageElementsInPipeline() == -1) {
        return -1;
    }

    return 0;
}

/**
 * Modifies the image element properties.
 */
void ModifyImageElementProperties() {
    const int size = 256;
    char imageDestination[size];
    CurrentImageDestination(imageDestination, size);
    g_print("Saving image to %s.\n", imageDestination);
    g_object_set(imageElements.sink, "location", imageDestination, NULL);
}

int SetupImageCamera() {
    if (MakeImageElements() == -1) {
        return -1;
    }

    if (SetupImagePipeline() == -1) {
        return -1;
    }

    ModifyImageElementProperties();

    return 0;
}

int StartImageCameraStream(GstSample *sample) {
    GstStateChangeReturn state_change_return = gst_element_set_state(imagePipeline, GST_STATE_PLAYING);
    if (state_change_return == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set the pipeline to the playing state.\n");
        return -1;
    }

    GstFlowReturn *flowReturn;
    g_signal_emit_by_name(imageElements.source, "push-sample", sample, &flowReturn);
    if (flowReturn != GST_FLOW_OK) {
        g_printerr("Failed to push sample.\n");
        return -1;
    }

    g_signal_emit_by_name(imageElements.source, "end-of-stream", &flowReturn);
    if (flowReturn != GST_FLOW_OK) {
        g_printerr("Failed to send end-of-stream.\n");
        return -1;
    }

    imageBus = gst_element_get_bus(imagePipeline);
    GstMessage *msg = gst_bus_timed_pop_filtered(imageBus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

    if (msg != NULL) {
        GError *err;
        gchar *debug_info;

        switch (GST_MESSAGE_TYPE (msg)) {
            case GST_MESSAGE_ERROR:
                gst_message_parse_error(msg, &err, &debug_info);
                g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
                g_printerr("Debugging information: %s\n", debug_info ? debug_info : "none");
                g_clear_error(&err);
                g_free(debug_info);
                return -1;
            case GST_MESSAGE_EOS:
                g_print("End-Of-Stream reached.\n");
                break;
            default:
                // unreachable
                g_printerr("Unexpected message received.\n");
                return -1;
        }
        gst_message_unref(msg);
    }

    state_change_return = gst_element_set_state(imagePipeline, GST_STATE_PAUSED);
    if (state_change_return == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set the pipeline to the paused state.\n");
        return -1;
    }

    return 0;
}

void CleanupImageCamera() {
    gst_element_set_state(imagePipeline, GST_STATE_NULL);
    gst_object_unref(imagePipeline);
}
