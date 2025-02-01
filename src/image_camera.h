#include <gst/gst.h>

#ifndef LIGHTCUBE_SRC_IMAGE_CAMERA_H_
#define LIGHTCUBE_SRC_IMAGE_CAMERA_H_

#endif //LIGHTCUBE_SRC_IMAGE_CAMERA_H_

/**
 * Sets up the image camera.
 *
 * @return the status code of the image camera setup
 */
int SetupImageCamera();

/**
 * Starts the image camera stream.
 *
 * @param sample the sample image to capture to file
 * @return the status code of the image camera stream start
 */
int StartImageCameraStream(GstSample *sample);

/**
 * Cleans up the image camera.
 */
void CleanupImageCamera();
