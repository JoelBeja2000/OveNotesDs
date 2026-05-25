#ifndef UI_H
#define UI_H

#include <nds.h>
#include <stdbool.h>

extern PrintConsole subConsole;

extern int active_brush_size;
extern int eraser_size;
extern int active_color_idx;
extern int drawing_mode;
extern bool is_bucket;
extern int bg_pattern_idx;
extern bool is_eraser;
extern uint16_t palette_colors[5];

void uiDrawToolbar(void);
void uiDrawFormUI(int step, const char* input_text);
void uiDrawBottomForm(int step, const char* input_text);
char uiHandleKeyboardTouch(int tx, int ty, bool* shift_toggled, bool* caps_toggled, bool* enter_pressed, bool* backspace_pressed);

#endif // UI_H
