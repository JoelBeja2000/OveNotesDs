#include "input.h"

void inputScan(void) {
    scanKeys();
}

uint32_t inputGetKeysHeld(void) {
    return keysHeld();
}

uint32_t inputGetKeysDown(void) {
    return keysDown();
}

bool inputGetTouch(touchPosition* touch) {
    if (keysHeld() & KEY_TOUCH) {
        touchRead(touch);
        return true;
    }
    return false;
}
