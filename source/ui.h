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

extern int open_modal;
extern uint16_t modal_backup[256 * 156];

extern int preset_page;
extern int custom_page;
extern int selected_custom_slot;
extern int color_modal_tab;
extern int bg_modal_tab;

extern int picker_x;
extern int picker_y;
extern int picker_h;
extern int picker_s;
extern int picker_v;

extern uint16_t preset_palettes[20][5];
extern uint16_t custom_palettes[50][5];

void uiDrawToolbar(void);
void uiOpenModal(int modal_idx);
void uiCloseModal(void);
void uiUpdateModalBackup(void);
uint16_t hsv_to_rgb15(int h, int s, int v);
void rgb15_to_hsv(uint16_t color, int* h, int* s, int* v);
void uiUpdatePickerPosFromActiveColor(void);
void uiUpdateColorPickerSelection(void);
void uiUpdateAngleWheelVisuals(void);
void uiDrawLayersOverlay(void);
void uiDrawFormUI(int step, const char* input_text);
void uiDrawBottomForm(int step, const char* input_text);
char uiHandleKeyboardTouch(int tx, int ty, bool* shift_toggled, bool* caps_toggled, bool* enter_pressed, bool* backspace_pressed);
void uiDrawRenameKeyboard(const char* input_text, uint8_t opacity);
void uiDrawUndoRedoButtons(void);

#endif // UI_H
