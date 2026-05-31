#define NO_UI_COMPAT_MACROS
#include "widget_buttons.h"
#include "ui.h"
#include "render.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define app_theme_color g_app_state.ui.app_theme_color
#define active_theme_idx g_app_state.ui.active_theme_idx

void drawModalButtonAt(int x0, int x1, int y0, int y1, const char* text, bool selected) {
    uint16_t bg = selected ? blendRGB555_int(app_theme_color, RGB15(4, 4, 5), 8) : RGB15(2, 2, 3);
    uint16_t outline = selected ? app_theme_color : blendRGB555_int(app_theme_color, RGB15(2, 2, 3), 8);
    uint16_t text_color = RGB15(31, 31, 31);
    if (selected && active_theme_idx == 0) {
        text_color = RGB15(0, 0, 0);
    }
    
    renderDrawRect(x0, y0, x1, y1, bg);
    renderDrawRectOutline(x0, y0, x1, y1, outline);
    
    int text_len = strlen(text);
    int text_w = text_len * 6 - 1;
    int tx = x0 + (x1 - x0 - text_w) / 2;
    int ty = y0 + (y1 - y0 - 8) / 2;
    renderDrawText(text, tx, ty, text_color, 0);
}

void drawModalButtonDisabledAt(int x0, int x1, int y0, int y1, const char* text) {
    uint16_t bg = RGB15(1, 1, 2);
    uint16_t outline = RGB15(4, 4, 5);
    uint16_t text_color = RGB15(8, 8, 8);
    
    renderDrawRect(x0, y0, x1, y1, bg);
    renderDrawRectOutline(x0, y0, x1, y1, outline);
    
    int text_len = strlen(text);
    int text_w = text_len * 6 - 1;
    int tx = x0 + (x1 - x0 - text_w) / 2;
    int ty = y0 + (y1 - y0 - 8) / 2;
    renderDrawText(text, tx, ty, text_color, 0);
}

void drawToolButtonWithIconAt(int x0, int x1, int y0, int y1, const char* text, int icon_type, bool selected) {
    uint16_t bg = selected ? blendRGB555_int(app_theme_color, RGB15(4, 4, 5), 8) : RGB15(2, 2, 3);
    uint16_t outline = selected ? app_theme_color : blendRGB555_int(app_theme_color, RGB15(2, 2, 3), 8);
    uint16_t text_color = RGB15(31, 31, 31);
    uint16_t icon_draw_color = RGB15(31, 31, 31);
    if (selected && active_theme_idx == 0) {
        text_color = RGB15(0, 0, 0);
        icon_draw_color = RGB15(0, 0, 0);
    } else if (!selected) {
        text_color = RGB15(22, 22, 24);
        icon_draw_color = RGB15(22, 22, 24);
    }
    
    renderDrawRect(x0, y0, x1, y1, bg);
    renderDrawRectOutline(x0, y0, x1, y1, outline);
    if (selected) {
        renderDrawRectOutline(x0 + 1, y0 + 1, x1 - 1, y1 - 1, outline);
    }
    
    int icon_x0 = x0 + 4;
    int icon_y0 = y0 + (y1 - y0 - 12) / 2;
    
    if (icon_type == 0) { // Pincel
        renderDrawRect(icon_x0 + 4, icon_y0 + 2, icon_x0 + 10, icon_y0 + 7, icon_draw_color);
        renderDrawRect(icon_x0 + 5, icon_y0 + 8, icon_x0 + 9, icon_y0 + 10, icon_draw_color);
        renderDrawRect(icon_x0 + 5, icon_y0 + 3, icon_x0 + 6, icon_y0 + 4, selected ? RGB15(0, 0, 0) : RGB15(31, 31, 31));
    } 
    else if (icon_type == 1) { // Borrador
        renderDrawRect(icon_x0 + 2, icon_y0 + 3, icon_x0 + 12, icon_y0 + 9, icon_draw_color);
        uint16_t div_c = selected ? RGB15(15, 15, 15) : RGB15(25, 25, 25);
        renderDrawRect(icon_x0 + 7, icon_y0 + 3, icon_x0 + 8, icon_y0 + 9, div_c);
    } 
    else if (icon_type == 2) { // Relleno
        renderDrawRect(icon_x0 + 4, icon_y0 + 4, icon_x0 + 10, icon_y0 + 10, icon_draw_color);
        renderDrawRect(icon_x0 + 5, icon_y0 + 2, icon_x0 + 9, icon_y0 + 3, icon_draw_color);
        renderDrawRect(icon_x0 + 11, icon_y0 + 8, icon_x0 + 12, icon_y0 + 10, icon_draw_color);
    } 
    else if (icon_type == 3) { // Trazo Normal
        renderDrawRect(icon_x0 + 1, icon_y0 + 5, icon_x0 + 13, icon_y0 + 6, icon_draw_color);
    } 
    else if (icon_type == 4) { // Rotulador
        renderDrawRectOutline(icon_x0 + 1, icon_y0 + 3, icon_x0 + 13, icon_y0 + 8, icon_draw_color);
        renderDrawRect(icon_x0 + 3, icon_y0 + 5, icon_x0 + 11, icon_y0 + 6, icon_draw_color);
    }
    
    int tx = x0 + 20 + (x1 - (x0 + 20) - (int)strlen(text) * 6) / 2;
    int ty = y0 + (y1 - y0 - 8) / 2;
    renderDrawText(text, tx, ty, text_color, 0);
}

void drawPatternBrushButtonAt(int x0, int x1, int y0, int y1, int pat_idx, bool selected) {
    uint16_t outline = selected ? app_theme_color : blendRGB555_int(app_theme_color, RGB15(2, 2, 3), 8);
    uint16_t p_color = selected ? blendRGB555_int(app_theme_color, RGB15(4, 4, 5), 8) : RGB15(2, 2, 3);
    uint16_t pat_color = RGB15(31, 31, 31);
    if (selected && active_theme_idx == 0) {
        pat_color = RGB15(0, 0, 0);
    } else if (!selected) {
        pat_color = RGB15(22, 22, 24);
    }
    
    renderDrawRect(x0, y0, x1, y1, p_color);
    renderDrawRectOutline(x0, y0, x1, y1, outline);
    if (selected) {
        renderDrawRectOutline(x0 + 1, y0 + 1, x1 - 1, y1 - 1, outline);
    }
    
    for (int y = y0 + 2; y <= y1 - 2; y++) {
        for (int x = x0 + 2; x <= x1 - 2; x++) {
            int rx = x - x0;
            int ry = y - y0;
            bool draw_pixel = false;
            
            if (pat_idx == 2) {
                if ((rx + ry) % 2 == 0) draw_pixel = true;
            } else if (pat_idx == 3) {
                if ((rx - ry) % 4 == 0) draw_pixel = true;
            } else if (pat_idx == 4) {
                if (ry % 4 == 0) draw_pixel = true;
            } else if (pat_idx == 5) {
                if (rx % 4 == 0) draw_pixel = true;
            } else if (pat_idx == 6) {
                if (rx % 4 == 0 || ry % 4 == 0) draw_pixel = true;
            }
            
            if (draw_pixel) {
                canvas_buffer[y * 256 + x] = pat_color;
            }
        }
    }
}

void drawPlumaButtonAt(int x0, int x1, int y0, int y1, const char* label, int pl_type, bool selected) {
    uint16_t bg = selected ? blendRGB555_int(app_theme_color, RGB15(4, 4, 5), 8) : RGB15(2, 2, 3);
    uint16_t outline = selected ? app_theme_color : blendRGB555_int(app_theme_color, RGB15(2, 2, 3), 8);
    uint16_t text_color = RGB15(31, 31, 31);
    uint16_t stroke_color = RGB15(31, 31, 31);
    if (selected && active_theme_idx == 0) {
        text_color = RGB15(0, 0, 0);
        stroke_color = RGB15(0, 0, 0);
    } else if (!selected) {
        text_color = RGB15(22, 22, 24);
        stroke_color = RGB15(22, 22, 24);
    }
    
    renderDrawRect(x0, y0, x1, y1, bg);
    renderDrawRectOutline(x0, y0, x1, y1, outline);
    if (selected) {
        renderDrawRectOutline(x0 + 1, y0 + 1, x1 - 1, y1 - 1, outline);
    }
    
    renderDrawText(label, x0 + 2, y0 + 2, text_color, 0);
    
    int cx = (x0 + x1) / 2;
    int cy = (y0 + y1) / 2 + 2;
    
    if (pl_type == 7) {
        for (int d = -4; d <= 4; d++) {
            renderDrawRect(cx + d - 1, cy - d - 1, cx + d + 1, cy - d + 1, stroke_color);
        }
    } 
    else if (pl_type == 8) {
        for (int d = -6; d <= 6; d++) {
            int r = 1;
            if (d >= -3 && d <= 3) r = 2;
            renderDrawRect(cx + d, cy - r, cx + d, cy + r, stroke_color);
        }
    } 
    else if (pl_type == 9) {
        for (int d = -5; d <= 0; d++) {
            renderDrawRect(cx + d, cy + 1, cx + d, cy + 2, stroke_color);
        }
        for (int d = 1; d <= 5; d++) {
            renderDrawRect(cx + d, cy - d + 1, cx + d, cy - d + 2, stroke_color);
        }
    } 
    else if (pl_type == 10) {
        renderDrawRect(cx - 6, cy, cx + 6, cy, stroke_color);
        renderDrawRect(cx, cy - 3, cx, cy - 3, stroke_color);
    }
}

void drawModalColorButtonAt(int x0, int x1, int y0, int y1, uint16_t color, bool selected) {
    uint16_t outline = selected ? app_theme_color : RGB15(0, 0, 0);
    int outline_width = selected ? 2 : 1;
    
    renderDrawRect(x0, y0, x1, y1, color);
    
    for (int w = 0; w < outline_width; w++) {
        renderDrawRectOutline(x0 + w, y0 + w, x1 - w, y1 - w, outline);
    }
}
