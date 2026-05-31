#define NO_UI_COMPAT_MACROS
#include "modals.h"
#include "ui.h"
#include "render.h"
#include <stdio.h>

void uiDrawModalBrushSize(const AppState* app, int y0, int y1) {
    char label[32];
    if (app->draw.is_eraser) {
        sprintf(label, uiTxt(TXT_GROSOR_BORRADOR), app->draw.eraser_size);
    } else {
        sprintf(label, uiTxt(TXT_GROSOR_PINCEL), app->draw.active_brush_size);
    }
    renderDrawText(label, 16, 126, RGB15(31, 31, 31), 0);
    
    renderDrawRect(24, 144, 232, 148, RGB15(20, 20, 20));
    renderDrawRectOutline(24, 144, 232, 148, RGB15(0, 0, 0));
    
    int knob_x = 24;
    if (app->draw.is_eraser) {
        knob_x = 24 + (app->draw.eraser_size - 2) * 208 / 28;
    } else {
        knob_x = 24 + (app->draw.active_brush_size - 1) * 208 / 14;
    }
    if (knob_x < 24) knob_x = 24;
    if (knob_x > 232) knob_x = 232;
    
    renderDrawRect(knob_x - 4, 138, knob_x + 4, 154, RGB15(12, 12, 18));
    renderDrawRectOutline(knob_x - 4, 138, knob_x + 4, 154, RGB15(0, 0, 0));
}
