#ifndef LIGHTCUBE_SRC_BUTTONS_H
#define LIGHTCUBE_SRC_BUTTONS_H

#endif //LIGHTCUBE_SRC_BUTTONS_H

typedef enum {
    RELEASED, PRESSED, HOLD, HOLD_LONG
} ButtonState;

/**
 * Sets up the buttons.
 *
 * @return the status code of the button setup
 */
int SetupButtons();

/**
 * Reads the increase button state.
 *
 * @param deltaTime the delta time since the last call to this function
 * @return the increase button state
 */
ButtonState ReadIncreaseButton(float deltaTime);

/**
 * Reads the decrease button state.
 *
 * @param deltaTime the delta time since the last call to this function
 * @return the decrease button state
 */
ButtonState ReadDecreaseButton(float deltaTime);

/**
 * Reads the capture button state.
 *
 * @param deltaTime the delta time since the last call to this function
 * @return the capture button state
 */
ButtonState ReadCaptureButton(float deltaTime);
