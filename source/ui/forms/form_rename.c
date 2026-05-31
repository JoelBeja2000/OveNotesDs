#define NO_UI_COMPAT_MACROS
#include "form_rename.h"
#include "keyboard.h"
#include "ui.h"
#include "render.h"
#include <stdio.h>

void uiDrawRenameKeyboard(const char* input_text) {
    uint16_t bg_color = RGB15(4, 4, 5);
    for (int y = 0; y < 192; y++) {
        for (int x = 0; x < 256; x++) {
            canvas_buffer[y * 256 + x] = bg_color;
        }
    }
    
    uint16_t grid_color = RGB15(6, 6, 8);
    for (int y = 0; y < 192; y += 16) {
        for (int x = 0; x < 256; x += 16) {
            renderSetPixel(x, y, grid_color);
        }
    }
    
    // Header banner: Localized
    uint16_t header_bg = blendRGB555_int(g_app_state.ui.app_theme_color, bg_color, 8);
    renderDrawRect(0, 0, 255, 12, header_bg);
    renderDrawRect(0, 12, 255, 12, g_app_state.ui.app_theme_color);
    renderDrawText(uiTxt(TXT_KEY_LAYER_PROP), 8, 2, RGB15(31, 31, 31), 0);
    
    // Text input box outline and fill
    for (int y = 18; y <= 36; y++) {
        for (int x = 10; x <= 246; x++) {
            canvas_buffer[y * 256 + x] = RGB15(2, 2, 3);
        }
    }
    renderDrawRectOutline(10, 18, 246, 36, g_app_state.ui.app_theme_color);
    
    char label[128];
    sprintf(label, "%s_", input_text);
    renderDrawText(label, 16, 23, RGB15(31, 31, 31), 0);
    
    // Cancel & Save Buttons (Localized)
    drawKey(10, 44, 90, 62, uiTxt(TXT_KEY_CANCEL), false);
    drawKey(166, 44, 246, 62, uiTxt(TXT_KEY_SAVE), false);
    
    drawKeyboard();
}
