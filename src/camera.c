#include "camera.h"

#include <stdio.h>
#include <unistd.h>

#include <gst/gst.h>

#include "crosshair.h"
#include "image_camera.h"

#define DISPLAY_WIDTH 800
#define DISPLAY_HEIGHT 480

#define TEXT_DISAPPEAR_DELAY 5

// Brightness data is given by the camera. To get these values, use the command `v4l2-ctl -d 0 --all`
#define DEFAULT_BRIGHTNESS 8
#define BRIGHTNESS_STEP 1
#define MIN_BRIGHTNESS 0
#define MAX_BRIGHTNESS 15

typedef struct {
    GstElement *source;
    GstElement *capsfilter;
    GstElement *tee;
    GstElement *queue;
    GstElement *videoscale;
    GstElement *displayCapsfilter;
    GstElement *videoconvert;
    GstElement *gdkpixbufoverlay;
    GstElement *textoverlay;
    GstElement *sink;
    GstElement *fakesink;
} PipelineElements;

PipelineElements elements;
GstElement *pipeline;
GstBus *bus;

GstStructure *extraControlsStructure;
int brightness = DEFAULT_BRIGHTNESS;
float textDisplayTime = -1;

/**
 * Makes the pipeline elements.
 *
 * @return the status code of the element creation
 */
int MakeElements() {
    elements.source = gst_element_factory_make("v4l2src", "source");
    elements.capsfilter = gst_element_factory_make("capsfilter", "capsfilter");
    elements.tee = gst_element_factory_make("tee", "tee");
    elements.queue = gst_element_factory_make("queue", "queue");
    elements.videoscale = gst_element_factory_make("videoscale", "videoscale");
    elements.displayCapsfilter = gst_element_factory_make("capsfilter", "displayCapsfilter");
    elements.videoconvert = gst_element_factory_make("videoconvert", "videoconvert");
    elements.gdkpixbufoverlay = gst_element_factory_make("gdkpixbufoverlay", "gdkpixbufoverlay");
    elements.textoverlay = gst_element_factory_make("textoverlay", "textoverlay");
    elements.sink = gst_element_factory_make("fbdevsink", "sink");
    elements.fakesink = gst_element_factory_make("fakesink", "fakesink");

    if (!elements.source || !elements.capsfilter || !elements.tee || !elements.queue || !elements.videoscale
        || !elements.displayCapsfilter || !elements.videoconvert || !elements.gdkpixbufoverlay || !elements.textoverlay
        || !elements.sink || !elements.fakesink) {
        g_printerr("Could not create all elements.\n");
        return -1;
    }

    return 0;
}

/**
 * Adds the elements to the pipeline.
 */
void AddElementsToPipeline() {
    gst_bin_add_many(GST_BIN(pipeline), elements.source, elements.capsfilter, elements.tee, elements.queue,
                     elements.videoscale, elements.displayCapsfilter, elements.videoconvert, elements.gdkpixbufoverlay,
                     elements.textoverlay, elements.sink, elements.fakesink, NULL);
}

/**
 * Links the elements in the pipeline.
 *
 * @return the status code of the element linkage
 */
int LinkElementsInPipeline() {
    if (!gst_element_link_many(elements.source, elements.capsfilter, elements.tee, NULL)) {
        g_printerr("Elements could not be linked.\n");
        return -1;
    }

    if (!gst_element_link_many(elements.queue, elements.videoscale, elements.displayCapsfilter, elements.videoconvert,
                               elements.gdkpixbufoverlay, elements.textoverlay, elements.sink, NULL)) {
        g_printerr("Elements could not be linked.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    GstPad *teePad = gst_element_get_request_pad(elements.tee, "src_%u");
    GstPad *queuePad = gst_element_get_static_pad(elements.queue, "sink");
    GstPad *teeImagePad = gst_element_get_request_pad(elements.tee, "src_%u");
    GstPad *fakesinkPad = gst_element_get_static_pad(elements.fakesink, "sink");
    if (gst_pad_link(teePad, queuePad) != GST_PAD_LINK_OK
        || gst_pad_link(teeImagePad, fakesinkPad) != GST_PAD_LINK_OK) {
        g_printerr("Tee could not be linked.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    gst_object_unref(queuePad);
    gst_object_unref(fakesinkPad);

    return 0;
}

/**
 * Sets up the pipeline.
 *
 * @return the status code of the pipeline setup
 */
int SetupPipeline() {
    pipeline = gst_pipeline_new("lightcube-pipeline");
    if (!pipeline) {
        g_printerr("Could not create pipeline.\n");
        return -1;
    }

    AddElementsToPipeline();
    if (LinkElementsInPipeline() == -1) {
        return -1;
    }

    return 0;
}

/**
 * Copies the crosshair raw bytestream to file.
 *
 * @return the status code of the crosshair image creation
 */
int CreateCrosshairImage() {
    FILE *crosshairFile = fopen("crosshair.png", "w");
    if (crosshairFile == NULL) {
        g_printerr("Error creating crosshair.png file.\n");
        return -1;
    }

    int i;
    for (i = 0; i < crosshair_png_len; i++) {
        fwrite(crosshair_png, sizeof(char), crosshair_png_len, crosshairFile);
    }

    fclose(crosshairFile);

    return 0;
}

/**
 * Modifies the element properties.
 */
void ModifyElementProperties() {
    extraControlsStructure = gst_structure_new("v4l2", "brightness", G_TYPE_INT, brightness, NULL);
    g_object_set(elements.source, "extra-controls", extraControlsStructure, NULL);

    GstCaps *videoCaps = gst_caps_new_simple("video/x-raw", "framerate", GST_TYPE_FRACTION, 30, 1, NULL);
    g_object_set(elements.capsfilter, "caps", videoCaps, NULL);
    gst_caps_unref(videoCaps);

    GstCaps *displayCaps = gst_caps_new_simple("video/x-raw", "width", G_TYPE_INT, DISPLAY_WIDTH, "height", G_TYPE_INT,
                                               DISPLAY_HEIGHT, NULL);
    g_object_set(elements.displayCapsfilter, "caps", displayCaps, NULL);
    gst_caps_unref(displayCaps);

    if (CreateCrosshairImage() == -1) {
        g_printerr("Could not create crosshair image.\n");
    } else {
        g_object_set(elements.gdkpixbufoverlay, "location", "crosshair.png", NULL);
    }

    g_object_set(elements.textoverlay, "font-desc", "Sans, 24", NULL);
}

/**
 * Callback for received errors from the pipeline bus.
 */
void ErrorCallback() {
    g_printerr("ErrorCallback.\n");
}

/**
 * Callback for received end-of-stream message from the pipeline bus.
 */
void EosCallback() {
    g_printerr("EosCallback.\n");
}

/**
 * Sets up the pipeline bus callbacks.
 */
void SetupBusCallbacks() {
    gst_bus_add_signal_watch(bus);
    g_signal_connect(G_OBJECT(bus), "message::error", (GCallback) ErrorCallback, NULL);
    g_signal_connect(G_OBJECT(bus), "message::eos", (GCallback) EosCallback, NULL);
}

/**
 * Sets the text display of the camera.
 *
 * @param text the text to display
 */
void SetText(char *text) {
    textDisplayTime = 0.0f;
    g_object_set(elements.textoverlay, "text", text, NULL);
}

/**
 * Clears the text display of the camera.
 */
void ClearText() {
    textDisplayTime = -1;
    SetText("");
}

/**
 * Updates the brightness of the camera and displays the current brightness.
 */
void UpdateBrightness() {
    gst_structure_set(extraControlsStructure, "brightness", G_TYPE_INT, brightness, NULL);
    g_object_set(elements.source, "extra-controls", extraControlsStructure, NULL);

    char buffer[16];
    snprintf(buffer, sizeof(buffer), "Brightness: %d", brightness);
    SetText(buffer);
}

/**
 * Checks the text display and clears the text display when the text should disappear.
 *
 * @param deltaTime the delta time since the last call to this function
 */
void CheckTextDisplay(float deltaTime) {
    if (textDisplayTime < 0) {
        return;
    }

    textDisplayTime += deltaTime;
    if (textDisplayTime >= TEXT_DISAPPEAR_DELAY) {
        ClearText();
    }
}

int SetupCamera(int argc, char *argv[]) {
    // don't use v4l2 lib
    //    unsetenv("GST_V4L2_USE_LIBV4L2");
    putenv("GST_V4L2_USE_LIBV4L2=1");

    gst_init(&argc, &argv);

    if (MakeElements() == -1) {
        return -1;
    }

    if (SetupPipeline() == -1) {
        return -1;
    }

    ModifyElementProperties();

    return 0;
}

int StartCameraStream() {
    GstStateChangeReturn stateChangeReturn = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (stateChangeReturn == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set the pipeline to the playing state.\n");
        return -1;
    }

    bus = gst_element_get_bus(pipeline);
    SetupBusCallbacks();

    return 0;
}

void CleanupCamera() {
    if (bus != NULL) {
        gst_object_unref(bus);
    }

    if (extraControlsStructure != NULL) {
        gst_structure_free(extraControlsStructure);
    }

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
}

void UpdateCamera(float deltaTime) {
    CheckTextDisplay(deltaTime);
}

void IncreaseBrightness() {
    brightness += BRIGHTNESS_STEP;
    if (brightness > MAX_BRIGHTNESS) {
        brightness = MAX_BRIGHTNESS;
    }

    UpdateBrightness();
}

void DecreaseBrightness() {
    brightness -= BRIGHTNESS_STEP;
    if (brightness < MIN_BRIGHTNESS) {
        brightness = MIN_BRIGHTNESS;
    }

    UpdateBrightness();
}

int CaptureImage() {
    GstSample *lastsample;
    g_object_get(G_OBJECT(elements.fakesink), "last-sample", &lastsample, NULL);
    GstSample *image = gst_sample_copy(lastsample);
    gst_sample_unref(lastsample);

    if (SetupImageCamera() == -1) {
        CleanupCamera();
        return -1;
    }

    if (StartImageCameraStream(image) == -1) {
        return -1;
    }

    gst_sample_unref(image);
    CleanupImageCamera();

    return 0;
}
