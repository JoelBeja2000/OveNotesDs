#ifndef UI_KEYBOARD_H
#define UI_KEYBOARD_H

#include <nds.h>
#include <stdbool.h>

extern bool shift_active;
extern bool caps_active;

void uiDrawRenameKeyboard(const char* input_text);
char uiHandleKeyboardTouch(int tx, int ty, bool* shift_toggled, bool* caps_toggled, bool* enter_pressed, bool* backspace_pressed);

#endif // UI_KEYBOARD_H
