#define NO_UI_COMPAT_MACROS
#include "toolbar.h"
#include "ui.h"
#include "render.h"
#include <stdio.h>

#define app_theme_color g_app_state.ui.app_theme_color
#define active_theme_idx g_app_state.ui.active_theme_idx
#define open_modal g_app_state.ui.open_modal
#define is_eraser g_app_state.draw.is_eraser
#define is_bucket g_app_state.draw.is_bucket
#define eraser_size g_app_state.draw.eraser_size
#define active_brush_size g_app_state.draw.active_brush_size
#define palette_colors g_app_state.draw.palette_colors
#define active_color_idx g_app_state.draw.active_color_idx
#define bg_pattern_idx g_app_state.draw.bg_pattern_idx

static void fillButtonBg(int x_start, int x_end, uint16_t color) {
    for (int y = 177; y < 191; y++) {
        for (int x = x_start + 1; x < x_end; x++) {
            canvas_buffer[y * 256 + x] = color;
        }
    }
}

void uiDrawToolbar(void) {
    // Fill toolbar background
    for (int y = 176; y < 192; y++) {
        for (int x = 0; x < 256; x++) {
            canvas_buffer[y * 256 + x] = RGB15(2, 2, 3);
        }
    }

    // Draw vertical separators
    for (int y = 176; y < 192; y++) {
        renderSetPixel(42, y, RGB15(6, 6, 8));
        renderSetPixel(84, y, RGB15(6, 6, 8));
        renderSetPixel(126, y, RGB15(6, 6, 8));
        renderSetPixel(168, y, RGB15(6, 6, 8));
        renderSetPixel(212, y, RGB15(6, 6, 8));
    }

    // Highlight active modal buttons
    uint16_t active_bg = blendRGB555_int(app_theme_color, RGB15(4, 4, 5), 8);
    if (open_modal == 0) fillButtonBg(0, 42, active_bg);
    if (open_modal == 1) fillButtonBg(42, 84, active_bg);
    if (open_modal == 2) fillButtonBg(84, 126, active_bg);
    if (open_modal == 3) fillButtonBg(126, 168, active_bg);

    // Highlight Menu (Button 4)
    if (open_modal == 5) {
        fillButtonBg(168, 212, active_bg);
    } else {
        fillButtonBg(168, 212, RGB15(3, 3, 4));
    }
    
    fillButtonBg(212, 256, RGB15(1, 5, 2));

    uint16_t default_text_color = RGB15(22, 22, 24);
    uint16_t active_text_color = RGB15(31, 31, 31);
    if (active_theme_idx == 0) { // Yellow theme text flip
        active_text_color = RGB15(0, 0, 0);
    }

    // Button 0: TOOL
    const char* tool_label = uiTxt(TXT_TOOLBAR_PINC);
    if (is_eraser) tool_label = uiTxt(TXT_TOOLBAR_BORR);
    else if (is_bucket) tool_label = "FILL";
    renderDrawText(tool_label, 9, 180, (open_modal == 0) ? active_text_color : default_text_color, 0);

    // Button 1: SIZE
    char size_label[8];
    if (is_eraser) sprintf(size_label, uiTxt(TXT_SIZE_ABBR_FORMAT), eraser_size);
    else sprintf(size_label, uiTxt(TXT_SIZE_ABBR_FORMAT), active_brush_size);
    renderDrawText(size_label, 50, 180, (open_modal == 1) ? active_text_color : default_text_color, 0);

    // Button 2: COLOR
    uint16_t current_color = palette_colors[active_color_idx];
    renderDrawText(uiTxt(TXT_COL_ABBR), 90, 180, (open_modal == 2) ? active_text_color : default_text_color, 0);
    for (int dy = 0; dy < 6; dy++) {
        for (int dx = 0; dx < 6; dx++) {
            renderSetPixel(112 + dx, 181 + dy, current_color);
        }
    }

    // Button 3: BG
    char bg_label[16];
    sprintf(bg_label, uiTxt(TXT_BG_ABBR_FORMAT), bg_pattern_idx);
    renderDrawText(bg_label, 128, 180, (open_modal == 3) ? active_text_color : default_text_color, 0);

    // Button 4: MENU
    renderDrawText("MENU", 178, 180, (open_modal == 5) ? active_text_color : default_text_color, 0);

    // Button 5: ENVIAR
    renderDrawText(uiTxt(TXT_TOOLBAR_ENVIAR), 216, 180, RGB15(20, 31, 20), 0);
}
