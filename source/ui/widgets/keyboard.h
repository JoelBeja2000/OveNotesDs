#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <nds.h>
#include <stdbool.h>

extern bool shift_active;
extern bool caps_active;

void drawKey(int x0, int y0, int x1, int y1, const char* label, bool highlighted);
void drawKeyboard(void);
char uiHandleKeyboardTouch(int tx, int ty, bool* shift_toggled, bool* caps_toggled, bool* enter_pressed, bool* backspace_pressed);

#endif // KEYBOARD_H
