#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "buttons.h"
#include "camera.h"
#include "image_destination.h"

// The main loop is running every 50 milliseconds.
#define UPDATE_LOOP_DELAY 0.050f

#define HOLD_CYCLES 4

// Every 20th loop update, the images are tried to move to usb
#define IMAGE_MOVE_CYCLES 20

typedef enum {
    RUNNING, STOPPED
} RunningState;

RunningState runningState;
unsigned short imageMoveUpdate = 0;

// TODO - DEBUG remove
int holdCapture = 0;
int increaseHoldCycle = 0;
int decreaseHoldCycle = 0;

/**
 * Updates the camera to process button presses, to update the camera stream and to try moving images to usb.
 *
 * @return the status code of the update
 */
int Update() {
    ButtonState increaseState = ReadIncreaseButton(UPDATE_LOOP_DELAY);
    ButtonState decreaseState = ReadDecreaseButton(UPDATE_LOOP_DELAY);

    if ((increaseState == PRESSED || increaseState == HOLD_LONG) && decreaseState == RELEASED) {
        if (increaseHoldCycle % HOLD_CYCLES == 0) {
            IncreaseBrightness();
        }

        increaseHoldCycle++;
    }

    if (increaseState == RELEASED) {
        increaseHoldCycle = 0;
    }

    if (decreaseState == RELEASED) {
        decreaseHoldCycle = 0;
    }

    if ((decreaseState == PRESSED || decreaseState == HOLD_LONG) && increaseState == RELEASED) {
        if (decreaseHoldCycle % HOLD_CYCLES == 0) {
            DecreaseBrightness();
        }

        decreaseHoldCycle++;
    }

    ButtonState captureState = ReadCaptureButton(UPDATE_LOOP_DELAY);
    if (captureState == PRESSED) {
        int captureStatus = CaptureImage();
        if (captureStatus != 0) {
            return captureStatus;
        }
    }

    // TODO - DEBUG remove
    if (increaseState == HOLD && decreaseState == HOLD && holdCapture == 0) {
        holdCapture = 1;
        int captureStatus = CaptureImage();
        if (captureStatus != 0) {
            return captureStatus;
        }
    }
    if (increaseState == RELEASED || decreaseState == RELEASED) {
        holdCapture = 0;
    }
    // TODO - DEBUG remove end

    UpdateCamera(UPDATE_LOOP_DELAY);

    imageMoveUpdate++;
    if (imageMoveUpdate % IMAGE_MOVE_CYCLES == 0) {
        TryMovingImagesToUsb();
        imageMoveUpdate = 0;
    }

    return 0;
}

/**
 * Handles an incoming signal.
 *
 * @param signal the received signal
 */
void SignalHandler(int signal) {
    runningState = STOPPED;
}

/**
 * Entrypoint of the program.
 *
 * @param argc the count of program parameters
 * @param argv the program parameters
 * @return the status code of the program result
 */
int main(int argc, char *argv[]) {
    if (SetupCamera(argc, argv) == -1) {
        exit(1);
    }

    if (StartCameraStream() == -1) {
        CleanupCamera();
        exit(1);
    }

    signal(SIGINT, SignalHandler);

    SetupButtons();

    runningState = RUNNING;
    while (runningState == RUNNING) {
        usleep(UPDATE_LOOP_DELAY * 1000 * 1000);
        int updateStatus = Update();
        if (updateStatus != 0) {
            CleanupCamera();
            exit(1);
        }
    }

    CleanupCamera();
    return 0;
}
