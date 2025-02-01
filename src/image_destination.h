#ifndef LIGHTCUBE_IMAGE_DESTINATION_H
#define LIGHTCUBE_IMAGE_DESTINATION_H

#endif //LIGHTCUBE_IMAGE_DESTINATION_H

/**
 * Creates the current image destination with the full path and the image name with the current date time.
 *
 * @param[out] imageDestination the destination to the image destination string buffer
 * @param size the maximum size of the string buffer
 */
void CurrentImageDestination(char *imageDestination, long size);

/**
 * Tries to move the images from local storage to an usb device, if any is connected.
 */
void TryMovingImagesToUsb();
