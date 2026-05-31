#define NO_UI_COMPAT_MACROS
#include "view_canvas.h"
#include "ui.h"
#include "render.h"
#include <stdio.h>
#include <string.h>

#define app_theme_color g_app_state.ui.app_theme_color

void uiDrawUndoRedoButtons(void) {
    if (g_app_state.ui.toolbar_hidden) return;
    
    uint16_t btn_bg = RGB15(4, 4, 5);
    uint16_t border_col = app_theme_color;
    
    // Undo
    renderDrawRect(4, 4, 20, 20, btn_bg);
    renderDrawRectOutline(4, 4, 20, 20, border_col);
    uint16_t undo_text_col = (undo_count > 0) ? RGB15(31, 31, 31) : RGB15(8, 8, 9);
    renderDrawText("<", 9, 8, undo_text_col, 0);
    
    // Redo
    renderDrawRect(24, 4, 40, 20, btn_bg);
    renderDrawRectOutline(24, 4, 40, 20, border_col);
    uint16_t redo_text_col = (redo_count > 0) ? RGB15(31, 31, 31) : RGB15(8, 8, 9);
    renderDrawText(">", 29, 8, redo_text_col, 0);
}

void uiDrawTopConsoleBox(const char* title) {
    if (preview_buffer == NULL) return;
    
    renderUpdatePreview();
    
    uint16_t border_color = app_theme_color;
    uint16_t bg_color = RGB15(3, 3, 4);
    
    for (int y = 44; y <= 147; y++) {
        for (int x = 20; x <= 235; x++) {
            if (y <= 46 || y >= 145 || x <= 22 || x >= 233) {
                preview_buffer[y * 256 + x] = border_color;
            } else {
                preview_buffer[y * 256 + x] = bg_color;
            }
        }
    }
    
    uint16_t header_bg = blendRGB555_int(app_theme_color, bg_color, 8);
    for (int y = 47; y <= 59; y++) {
        for (int x = 23; x <= 232; x++) {
            preview_buffer[y * 256 + x] = header_bg;
        }
    }
    
    int title_len = strlen(title);
    int text_width = title_len * 6;
    int tx = 23 + (210 - text_width) / 2;
    int ty = 47 + (13 - 8) / 2;
    if (tx < 23) tx = 23;
    renderDrawTextOnBuffer(preview_buffer, title, tx, ty, RGB15(31, 31, 31), 0);
}

void view_canvas_show(void) {
    renderComposeCanvas();
    renderUpdatePreview();
    if (!g_app_state.ui.toolbar_hidden) {
        uiDrawToolbar();
    }
}
