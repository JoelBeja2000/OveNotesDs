#define NO_UI_COMPAT_MACROS
#include "modals.h"
#include "ui.h"
#include "render.h"
#include "widget_buttons.h"
#include <stdio.h>

void uiDrawModalToolbox(const AppState* app, int y0, int y1) {
    renderDrawText(uiTxt(TXT_UTENSILIO), 16, y0 + 2, RGB15(31, 31, 31), 0);
    drawToolButtonWithIconAt(16, 86, 32, 52, uiTxt(TXT_PINCEL), 0, (!app->draw.is_eraser && !app->draw.is_bucket));
    drawToolButtonWithIconAt(92, 162, 32, 52, uiTxt(TXT_BORRADOR), 1, app->draw.is_eraser);
    drawToolButtonWithIconAt(168, 238, 32, 52, uiTxt(TXT_RELLENO), 2, app->draw.is_bucket);
    
    renderDrawText(uiTxt(TXT_TRAZO), 16, y0 + 36, RGB15(31, 31, 31), 0);
    drawToolButtonWithIconAt(16, 86, 66, 86, uiTxt(TXT_NORMAL), 3, (app->draw.drawing_mode == 0));
    drawToolButtonWithIconAt(92, 162, 66, 86, uiTxt(TXT_ROTUL), 4, (app->draw.drawing_mode == 1));
    
    renderDrawText(uiTxt(TXT_PATRON_BRUSH), 16, y0 + 70, RGB15(31, 31, 31), 0);
    for (int i = 0; i < 5; i++) {
        drawPatternBrushButtonAt(16 + i * 46, 16 + i * 46 + 40, 100, 120, 2 + i, (app->draw.drawing_mode == 2 + i));
    }
    
    renderDrawText(uiTxt(TXT_PLUMAS), 16, y0 + 104, RGB15(31, 31, 31), 0);
    drawPlumaButtonAt(16, 56, 134, 154, "PL1", 7, (app->draw.drawing_mode == 7));
    drawPlumaButtonAt(62, 102, 134, 154, "PL2", 8, (app->draw.drawing_mode == 8));
    drawPlumaButtonAt(108, 148, 134, 154, "PL3", 9, (app->draw.drawing_mode == 9));
    drawPlumaButtonAt(154, 194, 134, 154, "PL4", 10, (app->draw.drawing_mode == 10));
    char ang_lbl[16];
    sprintf(ang_lbl, "ANG:%d", app->draw.nib_angle);
    drawModalButtonAt(200, 240, 134, 154, ang_lbl, false);
}
