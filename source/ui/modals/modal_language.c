#define NO_UI_COMPAT_MACROS
#include "modals.h"
#include "ui.h"
#include "render.h"
#include <stdio.h>

void uiDrawLanguageModal(const AppState* app) {
    uint16_t modal_bg = RGB15(4, 4, 5);
    renderDrawRect(32, 30, 224, 160, modal_bg);
    renderDrawRectOutline(32, 30, 224, 160, app->ui.app_theme_color);
    renderDrawRectOutline(33, 31, 223, 159, app->ui.app_theme_color);
    
    renderDrawText(uiTxt(TXT_SELECCIONAR_IDIOMA), 48, 38, RGB15(31, 31, 31), 0);
    
    uint16_t es_bg = (app->ui.current_lang == 0) ? blendRGB555_int(app->ui.app_theme_color, RGB15(4, 4, 5), 8) : RGB15(12, 12, 14);
    uint16_t es_border = (app->ui.current_lang == 0) ? app->ui.app_theme_color : RGB15(18, 18, 20);
    renderDrawRect(48, 52, 208, 74, es_bg);
    renderDrawRectOutline(48, 52, 208, 74, es_border);
    renderDrawText("ESPANOL", 98, 60, RGB15(31, 31, 31), 0);
    
    uint16_t en_bg = (app->ui.current_lang == 1) ? blendRGB555_int(app->ui.app_theme_color, RGB15(4, 4, 5), 8) : RGB15(12, 12, 14);
    uint16_t en_border = (app->ui.current_lang == 1) ? app->ui.app_theme_color : RGB15(18, 18, 20);
    renderDrawRect(48, 78, 208, 100, en_bg);
    renderDrawRectOutline(48, 78, 208, 100, en_border);
    renderDrawText("ENGLISH", 98, 86, RGB15(31, 31, 31), 0);

    uint16_t fr_bg = (app->ui.current_lang == 2) ? blendRGB555_int(app->ui.app_theme_color, RGB15(4, 4, 5), 8) : RGB15(12, 12, 14);
    uint16_t fr_border = (app->ui.current_lang == 2) ? app->ui.app_theme_color : RGB15(18, 18, 20);
    renderDrawRect(48, 104, 208, 126, fr_bg);
    renderDrawRectOutline(48, 104, 208, 126, fr_border);
    renderDrawText("FRANCAIS", 98, 112, RGB15(31, 31, 31), 0);
    
    renderDrawRect(48, 132, 208, 152, RGB15(24, 6, 6));
    renderDrawRectOutline(48, 132, 208, 152, RGB15(31, 0, 0));
    renderDrawText(uiTxt(TXT_CERRAR), 108, 138, RGB15(31, 31, 31), 0);
}
