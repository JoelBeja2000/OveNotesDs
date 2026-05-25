#ifndef UI_H
#define UI_H

#include <nds.h>
#include <stdbool.h>

extern PrintConsole subConsole;
extern PrintConsole bottom_form_console;

extern int active_brush_size;
extern bool is_eraser;

void uiDrawToolbar(void);
void uiDrawFormUI(int step, const char* input_text);
void uiDrawBottomButtons(int active_step);

#endif // UI_H
