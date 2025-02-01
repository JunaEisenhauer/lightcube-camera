#include "image_destination.h"

#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

typedef enum {
    CONNECTED, DISCONNECTED
} UsbConnectionState;

/**
 * Gets the current date time.
 *
 * @return the current date time
 */
struct tm *CurrentDateTime() {
    time_t now = time(&now);
    if (now == -1) {
        return NULL;
    }

    struct tm *datetime = gmtime(&now);
    return datetime;
}

/**
 * Checks the current connection state of the usb hub if any usb device is connected.
 *
 * @return the usb connection state
 */
UsbConnectionState ConnectionState() {
    struct stat sb;
    return (stat("/media/usb", &sb) == 0 && S_ISDIR(sb.st_mode)) ? CONNECTED : DISCONNECTED;
}

void CurrentImageDestination(char *imageDestination, long size) {
    struct tm *datetime = CurrentDateTime();
    if (ConnectionState() == CONNECTED) {
        strftime(imageDestination, size, "/media/usb/%F %T.jpg", datetime);
    } else {
        strftime(imageDestination, size, "/media/intern/%F %T.jpg", datetime);
    }
}

void TryMovingImagesToUsb() {
    if (ConnectionState() != CONNECTED) {
        return;
    }

    // TODO - move files from /media/intern to /media/usb
}
