#include "buttons.h"

#include <stdio.h>

#include <wiringPi.h>

// The wiring pi pins for the buttons. Read pins and mappings with command `gpio readall`.
#define INCREASE_PIN 28 // wPi 28 -> BCM 20
#define DECREASE_PIN 29 // wPi 29 -> BCM 21
#define CAPTURE_PIN 27 // wPi 27 -> BCM 16

#define LONG_HOLD_TIME 0.5f

typedef struct {
    ButtonState state;
    float holdTime;
} ButtonData;

ButtonData increaseButtonData;
ButtonData decreaseButtonData;
ButtonData captureButtonData;

/**
 * Updates the button data for the given button.
 *
 * @param buttonData the button data to update
 * @param pin the wiring pi pin of the button
 * @param deltaTime the delta time since the last call to this buttons update
 * @return the updated button data
 */
ButtonData UpdateButton(ButtonData buttonData, int pin, float deltaTime) {
    //  pinState == 0 -> pressed, pinState == 1 -> released
    int pinState = digitalRead(pin);

    //  nothing (button still released)
    if (buttonData.state == RELEASED && pinState == 1) {
        return buttonData;
    }

    // press
    if (buttonData.state == RELEASED && pinState == 0) {
        buttonData.state = PRESSED;
        return buttonData;
    }

    // release
    if (buttonData.state != RELEASED && pinState == 1) {
        buttonData.holdTime = 0.0f;
        buttonData.state = RELEASED;
        return buttonData;
    }

    // hold
    if (buttonData.state != RELEASED && pinState == 0) {
        buttonData.holdTime += deltaTime;
        buttonData.state = buttonData.holdTime < LONG_HOLD_TIME ? HOLD : HOLD_LONG;
        return buttonData;
    }

    return buttonData;
}

int SetupButtons() {
    wiringPiSetup();

    pinMode(INCREASE_PIN, INPUT);
    pinMode(DECREASE_PIN, INPUT);
    pinMode(CAPTURE_PIN, INPUT);
    pullUpDnControl(INCREASE_PIN, PUD_UP);
    pullUpDnControl(DECREASE_PIN, PUD_UP);
    pullUpDnControl(CAPTURE_PIN, PUD_UP);

    return 0;
}

ButtonState ReadIncreaseButton(float deltaTime) {
    increaseButtonData = UpdateButton(increaseButtonData, INCREASE_PIN, deltaTime);
    return increaseButtonData.state;
}

ButtonState ReadDecreaseButton(float deltaTime) {
    decreaseButtonData = UpdateButton(decreaseButtonData, DECREASE_PIN, deltaTime);
    return decreaseButtonData.state;
}

ButtonState ReadCaptureButton(float deltaTime) {
    captureButtonData = UpdateButton(captureButtonData, CAPTURE_PIN, deltaTime);
    return captureButtonData.state;
}
