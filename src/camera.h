#ifndef LIGHTCUBE_SRC_CAMERA_H_
#define LIGHTCUBE_SRC_CAMERA_H_

#endif //LIGHTCUBE_SRC_CAMERA_H_

/**
 * Sets up the camera.
 *
 * @param argc the count of program parameters
 * @param argv the program parameters
 * @return the status code of the camera setup
 */
int SetupCamera(int argc, char *argv[]);

/**
 * Starts the camera stream.
 *
 * @return the status code of the camera stream start
 */
int StartCameraStream();

/**
 * Cleans up the camera.
 */
void CleanupCamera();

/**
 * Updates the camera displays.
 *
 * @param deltaTime the delta time since the last call to this function
 */
void UpdateCamera(float deltaTime);

/**
 * Increases the brightness of the camera.
 */
void IncreaseBrightness();

/**
 * Decreases the brightness of the camera.
 */
void DecreaseBrightness();

/**
 * Captures an image of the camera to file.
 *
 * @return the status code of the image capture.
 */
int CaptureImage();
