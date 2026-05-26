#include "ui.h"
#include "render.h"
#include "net.h"
#include "logo_data.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>

PrintConsole subConsole;

int active_brush_size = 1;
int eraser_size = 4;
int active_color_idx = 0;
int drawing_mode = 0;
bool is_bucket = false;
int bg_pattern_idx = 0;
bool is_eraser = false;

uint16_t palette_colors[5] = {
    RGB15(0, 0, 0),       // Black
    RGB15(0, 0, 28),      // Blue
    RGB15(28, 0, 0),      // Red
    RGB15(0, 20, 0),      // Green
    RGB15(30, 10, 20)     // Pink
};

uint16_t theme_colors[5] = {
    RGB15(31, 31, 0),   // Yellow / Banana
    RGB15(0, 31, 15),   // Mint Green
    RGB15(0, 15, 31),   // Sky Blue
    RGB15(31, 5, 5),    // Strawberry Red
    RGB15(20, 5, 31)    // Grape Purple
};
const char* theme_names[5] = {
    "BANANA (AMARILLO)",
    "MENTA (VERDE)",
    "CIELO (AZUL)",
    "FRESA (ROJO)",
    "UVA (MORADO)"
};
int active_theme_idx = 0;
uint16_t app_theme_color = RGB15(31, 31, 0);

int preset_page = 0;
int custom_page = 0;
int selected_custom_slot = 0;
int color_modal_tab = 0;
int bg_modal_tab = 0;

int picker_x = 128;
int picker_y = 84;
int picker_h = 0;
int picker_s = 31;
int picker_v = 31;

uint16_t preset_palettes[20][5] = {
    { RGB15(0, 0, 0), RGB15(0, 0, 28), RGB15(28, 0, 0), RGB15(0, 20, 0), RGB15(30, 10, 20) },
    { RGB15(28, 20, 20), RGB15(20, 24, 28), RGB15(28, 24, 20), RGB15(20, 28, 22), RGB15(28, 20, 28) },
    { RGB15(12, 10, 8), RGB15(8, 16, 16), RGB15(22, 12, 10), RGB15(24, 18, 12), RGB15(18, 20, 16) },
    { RGB15(8, 6, 4), RGB15(6, 14, 10), RGB15(18, 12, 6), RGB15(24, 22, 14), RGB15(12, 16, 8) },
    { RGB15(0, 0, 0), RGB15(0, 31, 31), RGB15(31, 0, 31), RGB15(31, 31, 0), RGB15(0, 31, 0) },
    { RGB15(0, 0, 0), RGB15(8, 8, 8), RGB15(16, 16, 16), RGB15(24, 24, 24), RGB15(31, 31, 31) },
    { RGB15(31, 0, 0), RGB15(31, 10, 0), RGB15(31, 20, 0), RGB15(31, 28, 10), RGB15(20, 0, 8) },
    { RGB15(0, 5, 15), RGB15(0, 12, 22), RGB15(0, 20, 28), RGB15(10, 28, 30), RGB15(20, 31, 31) },
    { RGB15(5, 2, 8), RGB15(31, 0, 20), RGB15(0, 28, 31), RGB15(24, 0, 31), RGB15(31, 28, 0) },
    { RGB15(6, 3, 1), RGB15(12, 6, 3), RGB15(18, 10, 5), RGB15(24, 16, 10), RGB15(28, 22, 18) },
    { RGB15(16, 4, 2), RGB15(24, 8, 4), RGB15(28, 16, 4), RGB15(20, 18, 6), RGB15(14, 12, 6) },
    { RGB15(31, 20, 22), RGB15(31, 15, 18), RGB15(28, 10, 14), RGB15(24, 6, 10), RGB15(18, 4, 6) },
    { RGB15(8, 4, 12), RGB15(18, 6, 15), RGB15(28, 10, 12), RGB15(31, 16, 8), RGB15(31, 24, 6) },
    { RGB15(10, 6, 16), RGB15(15, 10, 22), RGB15(20, 15, 28), RGB15(25, 20, 31), RGB15(28, 25, 31) },
    { RGB15(16, 24, 28), RGB15(20, 27, 30), RGB15(24, 29, 31), RGB15(28, 31, 31), RGB15(31, 31, 31) },
    { RGB15(10, 20, 16), RGB15(16, 25, 20), RGB15(22, 28, 24), RGB15(26, 31, 28), RGB15(30, 31, 30) },
    { RGB15(26, 20, 12), RGB15(28, 22, 15), RGB15(30, 25, 18), RGB15(31, 28, 22), RGB15(22, 16, 10) },
    { RGB15(6, 0, 10), RGB15(12, 2, 18), RGB15(18, 6, 24), RGB15(24, 12, 28), RGB15(28, 18, 31) },
    { RGB15(31, 16, 20), RGB15(16, 28, 31), RGB15(31, 28, 16), RGB15(20, 31, 20), RGB15(24, 16, 28) },
    { RGB15(2, 2, 3), RGB15(6, 6, 8), RGB15(12, 12, 14), RGB15(18, 18, 20), RGB15(26, 26, 28) }
};

uint16_t custom_palettes[50][5] = {
    [0 ... 49] = { RGB15(0, 0, 0), RGB15(0, 0, 28), RGB15(28, 0, 0), RGB15(0, 20, 0), RGB15(30, 10, 20) }
};

static void fillButtonBg(int x_start, int x_end, uint16_t color) {
    for (int y = 177; y < 191; y++) {
        for (int x = x_start + 1; x < x_end; x++) {
            canvas_buffer[y * 256 + x] = color;
        }
    }
}

uint16_t modal_backup[256 * 156];
int open_modal = -1;

static void getModalYRange(int modal, int* y0, int* y1) {
    if (modal == 0 || modal == 2 || modal == 3) {
        *y0 = 20;
        *y1 = 170;
    } else if (modal == 4) {
        *y0 = 90;
        *y1 = 170;
    } else if (modal == 5) {
        *y0 = 30;
        *y1 = 170;
    } else if (modal == 6) {
        *y0 = 50;
        *y1 = 170;
    } else { // modal == 1
        *y0 = 120;
        *y1 = 170;
    }
}

static void drawRect(int x0, int y0, int x1, int y1, uint16_t color) {
    if (x0 < 0) x0 = 0;
    if (x1 >= 256) x1 = 255;
    if (y0 < 0) y0 = 0;
    if (y1 >= 192) y1 = 191;
    for (int y = y0; y <= y1; y++) {
        for (int x = x0; x <= x1; x++) {
            canvas_buffer[y * 256 + x] = color;
        }
    }
}

static void drawRectOutline(int x0, int y0, int x1, int y1, uint16_t color) {
    if (x0 < 0) x0 = 0;
    if (x1 >= 256) x1 = 255;
    if (y0 < 0) y0 = 0;
    if (y1 >= 192) y1 = 191;
    for (int x = x0; x <= x1; x++) {
        canvas_buffer[y0 * 256 + x] = color;
        canvas_buffer[y1 * 256 + x] = color;
    }
    for (int y = y0; y <= y1; y++) {
        canvas_buffer[y * 256 + x0] = color;
        canvas_buffer[y * 256 + x1] = color;
    }
}

static void drawLine(int x0, int y0, int x1, int y1, uint16_t color) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;
    while (1) {
        if (x0 >= 0 && x0 < 256 && y0 >= 0 && y0 < 192) {
            canvas_buffer[y0 * 256 + x0] = color;
        }
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

static void drawCircleOutline(int xc, int yc, int r, uint16_t color) {
    int x = 0;
    int y = r;
    int d = 3 - 2 * r;
    while (y >= x) {
        #define SET_P(px, py) if ((px) >= 0 && (px) < 256 && (py) >= 0 && (py) < 192) canvas_buffer[(py)*256 + (px)] = color
        SET_P(xc + x, yc + y); SET_P(xc - x, yc + y);
        SET_P(xc + x, yc - y); SET_P(xc - x, yc - y);
        SET_P(xc + y, yc + x); SET_P(xc - y, yc + x);
        SET_P(xc + y, yc - x); SET_P(xc - y, yc - x);
        #undef SET_P
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
    }
}

static void drawModalButtonAt(int x0, int x1, int y0, int y1, const char* text, bool selected) {
    uint16_t bg = selected ? blendRGB555_int(app_theme_color, RGB15(4, 4, 5), 8) : RGB15(2, 2, 3);
    uint16_t outline = selected ? app_theme_color : blendRGB555_int(app_theme_color, RGB15(2, 2, 3), 8);
    uint16_t text_color = RGB15(31, 31, 31);
    if (selected && active_theme_idx == 0) {
        text_color = RGB15(0, 0, 0);
    }
    
    drawRect(x0, y0, x1, y1, bg);
    drawRectOutline(x0, y0, x1, y1, outline);
    
    int text_len = strlen(text);
    int text_w = text_len * 6 - 1;
    int tx = x0 + (x1 - x0 - text_w) / 2;
    int ty = y0 + (y1 - y0 - 8) / 2;
    renderDrawText(text, tx, ty, text_color, 0);
}

static void drawModalButtonDisabledAt(int x0, int x1, int y0, int y1, const char* text) {
    uint16_t bg = RGB15(1, 1, 2);
    uint16_t outline = RGB15(4, 4, 5);
    uint16_t text_color = RGB15(8, 8, 8);
    
    drawRect(x0, y0, x1, y1, bg);
    drawRectOutline(x0, y0, x1, y1, outline);
    
    int text_len = strlen(text);
    int text_w = text_len * 6 - 1;
    int tx = x0 + (x1 - x0 - text_w) / 2;
    int ty = y0 + (y1 - y0 - 8) / 2;
    renderDrawText(text, tx, ty, text_color, 0);
}

static void drawToolButtonWithIconAt(int x0, int x1, int y0, int y1, const char* text, int icon_type, bool selected) {
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
    
    drawRect(x0, y0, x1, y1, bg);
    drawRectOutline(x0, y0, x1, y1, outline);
    if (selected) {
        drawRectOutline(x0 + 1, y0 + 1, x1 - 1, y1 - 1, outline);
    }
    
    int icon_x0 = x0 + 4;
    int icon_y0 = y0 + (y1 - y0 - 12) / 2;
    
    if (icon_type == 0) { // Pincel
        drawRect(icon_x0 + 4, icon_y0 + 2, icon_x0 + 10, icon_y0 + 7, icon_draw_color);
        drawRect(icon_x0 + 5, icon_y0 + 8, icon_x0 + 9, icon_y0 + 10, icon_draw_color);
        drawRect(icon_x0 + 5, icon_y0 + 3, icon_x0 + 6, icon_y0 + 4, selected ? RGB15(0, 0, 0) : RGB15(31, 31, 31));
    } 
    else if (icon_type == 1) { // Borrador
        drawRect(icon_x0 + 2, icon_y0 + 3, icon_x0 + 12, icon_y0 + 9, icon_draw_color);
        uint16_t div_c = selected ? RGB15(15, 15, 15) : RGB15(25, 25, 25);
        drawRect(icon_x0 + 7, icon_y0 + 3, icon_x0 + 8, icon_y0 + 9, div_c);
    } 
    else if (icon_type == 2) { // Relleno
        drawRect(icon_x0 + 4, icon_y0 + 4, icon_x0 + 10, icon_y0 + 10, icon_draw_color);
        drawRect(icon_x0 + 5, icon_y0 + 2, icon_x0 + 9, icon_y0 + 3, icon_draw_color);
        drawRect(icon_x0 + 11, icon_y0 + 8, icon_x0 + 12, icon_y0 + 10, icon_draw_color);
    } 
    else if (icon_type == 3) { // Trazo Normal
        drawRect(icon_x0 + 1, icon_y0 + 5, icon_x0 + 13, icon_y0 + 6, icon_draw_color);
    } 
    else if (icon_type == 4) { // Rotulador
        drawRectOutline(icon_x0 + 1, icon_y0 + 3, icon_x0 + 13, icon_y0 + 8, icon_draw_color);
        drawRect(icon_x0 + 3, icon_y0 + 5, icon_x0 + 11, icon_y0 + 6, icon_draw_color);
    }
    
    int tx = x0 + 20 + (x1 - (x0 + 20) - (int)strlen(text) * 6) / 2;
    int ty = y0 + (y1 - y0 - 8) / 2;
    renderDrawText(text, tx, ty, text_color, 0);
}

static void drawPatternBrushButtonAt(int x0, int x1, int y0, int y1, int pat_idx, bool selected) {
    uint16_t outline = selected ? app_theme_color : blendRGB555_int(app_theme_color, RGB15(2, 2, 3), 8);
    uint16_t p_color = selected ? blendRGB555_int(app_theme_color, RGB15(4, 4, 5), 8) : RGB15(2, 2, 3);
    uint16_t pat_color = RGB15(31, 31, 31);
    if (selected && active_theme_idx == 0) {
        pat_color = RGB15(0, 0, 0);
    } else if (!selected) {
        pat_color = RGB15(22, 22, 24);
    }
    
    drawRect(x0, y0, x1, y1, p_color);
    drawRectOutline(x0, y0, x1, y1, outline);
    if (selected) {
        drawRectOutline(x0 + 1, y0 + 1, x1 - 1, y1 - 1, outline);
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

static void drawPlumaButtonAt(int x0, int x1, int y0, int y1, const char* label, int pl_type, bool selected) {
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
    
    drawRect(x0, y0, x1, y1, bg);
    drawRectOutline(x0, y0, x1, y1, outline);
    if (selected) {
        drawRectOutline(x0 + 1, y0 + 1, x1 - 1, y1 - 1, outline);
    }
    
    renderDrawText(label, x0 + 2, y0 + 2, text_color, 0);
    
    int cx = (x0 + x1) / 2;
    int cy = (y0 + y1) / 2 + 2;
    
    if (pl_type == 7) {
        for (int d = -4; d <= 4; d++) {
            drawRect(cx + d - 1, cy - d - 1, cx + d + 1, cy - d + 1, stroke_color);
        }
    } 
    else if (pl_type == 8) {
        for (int d = -6; d <= 6; d++) {
            int r = 1;
            if (d >= -3 && d <= 3) r = 2;
            drawRect(cx + d, cy - r, cx + d, cy + r, stroke_color);
        }
    } 
    else if (pl_type == 9) {
        for (int d = -5; d <= 0; d++) {
            drawRect(cx + d, cy + 1, cx + d, cy + 2, stroke_color);
        }
        for (int d = 1; d <= 5; d++) {
            drawRect(cx + d, cy - d + 1, cx + d, cy - d + 2, stroke_color);
        }
    } 
    else if (pl_type == 10) {
        drawRect(cx - 6, cy, cx + 6, cy, stroke_color);
        drawRect(cx, cy - 3, cx, cy - 3, stroke_color);
    }
}

static void drawModalColorButtonAt(int x0, int x1, int y0, int y1, uint16_t color, bool selected) {
    uint16_t outline = selected ? app_theme_color : RGB15(0, 0, 0);
    int outline_width = selected ? 2 : 1;
    
    drawRect(x0, y0, x1, y1, color);
    
    for (int w = 0; w < outline_width; w++) {
        drawRectOutline(x0 + w, y0 + w, x1 - w, y1 - w, outline);
    }
}

static void drawPatternPreview(int x0, int y0, int x1, int y1, int pat_idx, bool selected) {
    uint16_t p_color = bg_primary_palette[bg_color_p_idx];
    uint16_t s_color = bg_secondary_palette[bg_color_s_idx];
    uint16_t red_margin = RGB15(30, 8, 8);
    uint16_t outline = selected ? app_theme_color : RGB15(0, 0, 0);
    
    drawRect(x0, y0, x1, y1, p_color);
    
    for (int y = y0 + 1; y < y1; y++) {
        for (int x = x0 + 1; x < x1; x++) {
            int rx = x - x0;
            int ry = y - y0;
            
            bool draw_s = false;
            bool draw_red = false;
            
            if (pat_idx == 1) {
                if (ry % 8 == 0 && rx % 8 == 0) draw_s = true;
            } else if (pat_idx == 2) {
                if (ry % 8 == 0) draw_s = true;
            } else if (pat_idx == 3) {
                if (ry % 8 == 0 || rx % 8 == 0) draw_s = true;
            } else if (pat_idx == 4) {
                if (rx % 8 == 0) draw_s = true;
            } else if (pat_idx == 5) {
                if ((rx + ry * 2) % 16 == 0 || (rx - ry * 2) % 16 == 0) draw_s = true;
            } else if (pat_idx == 6) {
                if (ry % 12 == 0) draw_s = true;
            } else if (pat_idx == 7) {
                if (ry % 8 == 0) draw_s = true;
                if (rx == 8) draw_red = true;
            }
            
            if (draw_red) {
                canvas_buffer[y * 256 + x] = red_margin;
            } else if (draw_s) {
                canvas_buffer[y * 256 + x] = s_color;
            }
        }
    }
    
    drawRectOutline(x0, y0, x1, y1, outline);
    if (selected) {
        drawRectOutline(x0 + 1, y0 + 1, x1 - 1, y1 - 1, outline);
    }
}

uint16_t hsv_to_rgb15(int h, int s, int v) {
    int r = 0, g = 0, b = 0;
    if (s == 0) {
        r = g = b = v;
    } else {
        int base = ((31 - s) * v) >> 5;
        int color_range = v - base;
        int phase = (h / 60) % 6;
        int f = h % 60;
        int descending = (color_range * (60 - f)) / 60;
        int ascending = (color_range * f) / 60;
        
        switch (phase) {
            case 0: r = v; g = base + ascending; b = base; break;
            case 1: r = base + descending; g = v; b = base; break;
            case 2: r = base; g = v; b = base + ascending; break;
            case 3: r = base; g = base + descending; b = v; break;
            case 4: r = base + ascending; g = base; b = v; break;
            case 5: r = v; g = base; b = base + descending; break;
        }
    }
    return RGB15(r, g, b);
}

void rgb15_to_hsv(uint16_t color, int* h, int* s, int* v) {
    int r = color & 0x1F;
    int g = (color >> 5) & 0x1F;
    int b = (color >> 10) & 0x1F;
    
    int max_val = r;
    if (g > max_val) max_val = g;
    if (b > max_val) max_val = b;
    
    int min_val = r;
    if (g < min_val) min_val = g;
    if (b < min_val) min_val = b;
    
    *v = max_val;
    
    int delta = max_val - min_val;
    if (max_val == 0) {
        *s = 0;
    } else {
        *s = (delta * 31) / max_val;
    }
    
    if (delta == 0) {
        *h = 0;
    } else {
        if (max_val == r) {
            *h = (60 * (g - b)) / delta;
        } else if (max_val == g) {
            *h = 120 + (60 * (b - r)) / delta;
        } else {
            *h = 240 + (60 * (r - g)) / delta;
        }
        if (*h < 0) *h += 360;
    }
}



void uiUpdateColorPickerSelection(void) {
    if (open_modal != 2) return;
    
    // 1. Redraw the top 5 color swatches
    for (int i = 0; i < 5; i++) {
        drawModalColorButtonAt(16 + i * 46, 16 + i * 46 + 40, 36, 48, palette_colors[i], (active_color_idx == i));
    }
    
    // 2. Redraw the 2D Hue-Saturation Map: x = 16..136 (outline), inside: x = 17..135 (width 119), y = 57..115 (height 59)
    for (int dy = 0; dy < 59; dy++) {
        int s = 31 - (dy * 31) / 59;
        for (int dx = 0; dx < 119; dx++) {
            int h = (dx * 360) / 119;
            canvas_buffer[(57 + dy) * 256 + (17 + dx)] = hsv_to_rgb15(h, s, 31);
        }
    }
    
    // Draw Hue-Saturation reticle
    int reticle_x = 17 + (picker_h * 119) / 360;
    int reticle_y = 57 + ((31 - picker_s) * 59) / 31;
    if (reticle_x < 17) reticle_x = 17;
    if (reticle_x > 135) reticle_x = 135;
    if (reticle_y < 57) reticle_y = 57;
    if (reticle_y > 115) reticle_y = 115;
    uint16_t bg_col = hsv_to_rgb15(picker_h, picker_s, 31);
    int r = bg_col & 31, g = (bg_col >> 5) & 31, b = (bg_col >> 10) & 31;
    uint16_t reticle_color = (r + g + b > 45) ? RGB15(0, 0, 0) : RGB15(31, 31, 31);
    drawCircleOutline(reticle_x, reticle_y, 3, reticle_color);

    // 3. Draw the vertical Preview Swatch: x = 152..176 (outline), inside: x = 153..175, y = 57..115
    uint16_t preview_color = palette_colors[active_color_idx];
    drawRect(153, 57, 175, 115, preview_color);

    // 4. Draw the vertical Value (brightness) slider: x = 192..216 (outline), inside: x = 193..215 (width 23), y = 57..115 (height 59)
    for (int dy = 0; dy < 59; dy++) {
        int v = 31 - (dy * 31) / 59;
        uint16_t slider_color = hsv_to_rgb15(picker_h, picker_s, v);
        for (int dx = 0; dx < 23; dx++) {
            canvas_buffer[(57 + dy) * 256 + (193 + dx)] = slider_color;
        }
    }
    
    // Clear left and right sides of Value indicator line to grey background
    drawRect(190, 57, 191, 115, RGB15(28, 28, 28));
    drawRect(217, 57, 218, 115, RGB15(28, 28, 28));
    // Restore vertical borders of Value slider
    for (int y = 57; y <= 115; y++) {
        canvas_buffer[y * 256 + 192] = RGB15(0, 0, 0);
        canvas_buffer[y * 256 + 216] = RGB15(0, 0, 0);
    }
    
    // Draw Value indicator line
    int v_indicator_y = 57 + ((31 - picker_v) * 59) / 31;
    if (v_indicator_y < 57) v_indicator_y = 57;
    if (v_indicator_y > 115) v_indicator_y = 115;
    drawRect(190, v_indicator_y - 1, 218, v_indicator_y + 1, RGB15(0, 0, 0));
    drawRect(191, v_indicator_y, 217, v_indicator_y, RGB15(31, 31, 31));
}

void uiUpdatePickerPosFromActiveColor(void) {
    rgb15_to_hsv(palette_colors[active_color_idx], &picker_h, &picker_s, &picker_v);
}

static void drawLockIcon(int x, int y, bool locked) {
    uint16_t metal = RGB15(12, 12, 12);
    uint16_t body = RGB15(26, 15, 0); // Golden body
    if (locked) {
        body = RGB15(28, 5, 5); // Red body when locked
    }
    // Draw body: 8x6 rectangle
    for (int dy = 4; dy <= 9; dy++) {
        for (int dx = 1; dx <= 8; dx++) {
            if (x + dx >= 0 && x + dx < 256 && y + dy >= 0 && y + dy < 192) {
                canvas_buffer[(y + dy) * 256 + (x + dx)] = body;
            }
        }
    }
    // Draw shackle
    if (locked) {
        // Closed shackle: loop connecting both sides
        for (int i = 0; i < 4; i++) {
            if (x + 3 >= 0 && x + 3 < 256 && y + i >= 0 && y + i < 192) {
                canvas_buffer[(y + i) * 256 + (x + 3)] = metal;
            }
            if (x + 6 >= 0 && x + 6 < 256 && y + i >= 0 && y + i < 192) {
                canvas_buffer[(y + i) * 256 + (x + 6)] = metal;
            }
        }
        for (int dx = 4; dx <= 5; dx++) {
            if (x + dx >= 0 && x + dx < 256 && y >= 0 && y < 192) {
                canvas_buffer[y * 256 + (x + dx)] = metal;
            }
        }
    } else {
        // Open shackle: loop is open/turned on one side
        for (int i = 0; i < 4; i++) {
            if (x + 2 >= 0 && x + 2 < 256 && y + i >= 0 && y + i < 192) {
                canvas_buffer[(y + i) * 256 + (x + 2)] = metal;
            }
        }
        for (int dx = 3; dx <= 4; dx++) {
            if (x + dx >= 0 && x + dx < 256 && y >= 0 && y < 192) {
                canvas_buffer[y * 256 + (x + dx)] = metal;
            }
        }
        if (x + 4 >= 0 && x + 4 < 256 && y + 1 >= 0 && y + 1 < 192) {
            canvas_buffer[(y + 1) * 256 + (x + 4)] = metal;
        }
    }
}

void uiOpenModal(int modal_idx) {
    if (modal_idx == 2) {
        uiUpdatePickerPosFromActiveColor();
    }
    bool already_open = (open_modal == modal_idx);
    if (open_modal != -1 && !already_open) {
        uiCloseModal();
    }
    
    int y0, y1;
    getModalYRange(modal_idx, &y0, &y1);
    
    if (already_open) {
        for (int y = y0; y < 176; y++) {
            for (int x = 0; x < 256; x++) {
                canvas_buffer[y * 256 + x] = modal_backup[(y - y0) * 256 + x];
            }
        }
    } else {
        open_modal = modal_idx;
        for (int y = y0; y < 176; y++) {
            for (int x = 0; x < 256; x++) {
                modal_backup[(y - y0) * 256 + x] = canvas_buffer[y * 256 + x];
            }
        }
    }
    
    uint16_t modal_bg = RGB15(4, 4, 5);
    drawRect(8, y0, 247, y1, modal_bg);
    drawRectOutline(8, y0, 247, y1, app_theme_color);
    
    if (open_modal == 0) {
        renderDrawText("UTENSILIO", 16, y0 + 2, RGB15(31, 31, 31), 0);
        drawToolButtonWithIconAt(16, 86, 32, 52, "PINCEL", 0, (!is_eraser && !is_bucket));
        drawToolButtonWithIconAt(92, 162, 32, 52, "BORRADOR", 1, is_eraser);
        drawToolButtonWithIconAt(168, 238, 32, 52, "RELLENO", 2, is_bucket);
        
        renderDrawText("TRAZO", 16, y0 + 36, RGB15(31, 31, 31), 0);
        drawToolButtonWithIconAt(16, 86, 66, 86, "NORMAL", 3, (drawing_mode == 0));
        drawToolButtonWithIconAt(92, 162, 66, 86, "ROTUL.", 4, (drawing_mode == 1));
        
        renderDrawText("PATRON BRUSH", 16, y0 + 70, RGB15(31, 31, 31), 0);
        for (int i = 0; i < 5; i++) {
            drawPatternBrushButtonAt(16 + i * 46, 16 + i * 46 + 40, 100, 120, 2 + i, (drawing_mode == 2 + i));
        }
        
        renderDrawText("PLUMAS", 16, y0 + 104, RGB15(31, 31, 31), 0);
        drawPlumaButtonAt(16, 56, 134, 154, "PL1", 7, (drawing_mode == 7));
        drawPlumaButtonAt(62, 102, 134, 154, "PL2", 8, (drawing_mode == 8));
        drawPlumaButtonAt(108, 148, 134, 154, "PL3", 9, (drawing_mode == 9));
        drawPlumaButtonAt(154, 194, 134, 154, "PL4", 10, (drawing_mode == 10));
        char ang_lbl[16];
        sprintf(ang_lbl, "ANG:%d", nib_angle);
        drawModalButtonAt(200, 240, 134, 154, ang_lbl, false);
    } 
    else if (open_modal == 1) {
        char label[32];
        if (is_eraser) {
            sprintf(label, "GROSOR BORRADOR: %d px", eraser_size);
        } else {
            sprintf(label, "GROSOR PINCEL: %d px", active_brush_size);
        }
        renderDrawText(label, 16, 126, RGB15(31, 31, 31), 0);
        
        drawRect(24, 144, 232, 148, RGB15(20, 20, 20));
        drawRectOutline(24, 144, 232, 148, RGB15(0, 0, 0));
        
        int knob_x = 24;
        if (is_eraser) {
            knob_x = 24 + (eraser_size - 2) * 208 / 28;
        } else {
            knob_x = 24 + (active_brush_size - 1) * 208 / 14;
        }
        if (knob_x < 24) knob_x = 24;
        if (knob_x > 232) knob_x = 232;
        
        drawRect(knob_x - 4, 138, knob_x + 4, 154, RGB15(12, 12, 18));
        drawRectOutline(knob_x - 4, 138, knob_x + 4, 154, RGB15(0, 0, 0));
    } 
    else if (open_modal == 2) {
        uint16_t tab_active_bg = blendRGB555_int(app_theme_color, RGB15(4, 4, 5), 6);
        uint16_t tab_inactive_bg = RGB15(2, 2, 3);
        uint16_t tab_border = app_theme_color;
        
        drawRect(8, 20, 127, 32, (color_modal_tab == 0) ? tab_active_bg : tab_inactive_bg);
        drawRect(128, 20, 247, 32, (color_modal_tab == 1) ? tab_active_bg : tab_inactive_bg);
        
        drawRectOutline(8, 20, 127, 32, (color_modal_tab == 0) ? tab_border : blendRGB555_int(app_theme_color, RGB15(2,2,3), 8));
        drawRectOutline(128, 20, 247, 32, (color_modal_tab == 1) ? tab_border : blendRGB555_int(app_theme_color, RGB15(2,2,3), 8));
        
        renderDrawText("PRESETS", 44, 23, (color_modal_tab == 0) ? RGB15(31, 31, 31) : RGB15(15, 15, 15), 0);
        renderDrawText("MIS PALETAS", 152, 23, (color_modal_tab == 1) ? RGB15(31, 31, 31) : RGB15(15, 15, 15), 0);
        
        for (int i = 0; i < 5; i++) {
            drawModalColorButtonAt(16 + i * 46, 16 + i * 46 + 40, 36, 48, palette_colors[i], (active_color_idx == i));
        }
        
        // 1. Draw the 2D Hue-Saturation Map: x = 16..136 (outline), inside: x = 17..135 (width 119), y = 57..115 (height 59)
        for (int dy = 0; dy < 59; dy++) {
            int s = 31 - (dy * 31) / 59;
            for (int dx = 0; dx < 119; dx++) {
                int h = (dx * 360) / 119;
                canvas_buffer[(57 + dy) * 256 + (17 + dx)] = hsv_to_rgb15(h, s, 31);
            }
        }
        drawRectOutline(16, 56, 136, 116, RGB15(0, 0, 0));
        
        // Draw Hue-Saturation reticle
        int reticle_x = 17 + (picker_h * 119) / 360;
        int reticle_y = 57 + ((31 - picker_s) * 59) / 31;
        if (reticle_x < 17) reticle_x = 17;
        if (reticle_x > 135) reticle_x = 135;
        if (reticle_y < 57) reticle_y = 57;
        if (reticle_y > 115) reticle_y = 115;
        uint16_t bg_col = hsv_to_rgb15(picker_h, picker_s, 31);
        int r = bg_col & 31, g = (bg_col >> 5) & 31, b = (bg_col >> 10) & 31;
        uint16_t reticle_color = (r + g + b > 45) ? RGB15(0, 0, 0) : RGB15(31, 31, 31);
        drawCircleOutline(reticle_x, reticle_y, 3, reticle_color);
 
        // 2. Draw the vertical Preview Swatch: x = 152..176 (outline), inside: x = 153..175, y = 57..115
        uint16_t preview_color = palette_colors[active_color_idx];
        drawRect(153, 57, 175, 115, preview_color);
        drawRectOutline(152, 56, 176, 116, RGB15(0, 0, 0));
 
        // 3. Draw the vertical Value (brightness) slider: x = 192..216 (outline), inside: x = 193..215 (width 23), y = 57..115 (height 59)
        for (int dy = 0; dy < 59; dy++) {
            int v = 31 - (dy * 31) / 59;
            uint16_t slider_color = hsv_to_rgb15(picker_h, picker_s, v);
            for (int dx = 0; dx < 23; dx++) {
                canvas_buffer[(57 + dy) * 256 + (193 + dx)] = slider_color;
            }
        }
        drawRectOutline(192, 56, 216, 116, RGB15(0, 0, 0));
        
        // Draw Value indicator line
        int v_indicator_y = 57 + ((31 - picker_v) * 59) / 31;
        if (v_indicator_y < 57) v_indicator_y = 57;
        if (v_indicator_y > 115) v_indicator_y = 115;
        drawRect(190, v_indicator_y - 1, 218, v_indicator_y + 1, RGB15(0, 0, 0));
        drawRect(191, v_indicator_y, 217, v_indicator_y, RGB15(31, 31, 31));
        
        if (color_modal_tab == 0) {
            char page_lbl[16];
            sprintf(page_lbl, "%d/4", preset_page + 1);
            
            for (int i = 0; i < 5; i++) {
                int preset_idx = preset_page * 5 + i;
                int x0 = 12 + i * 48;
                int x1 = x0 + 38;
                int y0 = 134, y1 = 152;
                
                drawRectOutline(x0, y0, x1, y1, RGB15(0, 0, 0));
                int pad = 1;
                int x0_inner = x0 + pad;
                int x1_inner = x1 - pad;
                int W = x1_inner - x0_inner + 1;
                for (int c = 0; c < 5; c++) {
                    int cx0 = x0_inner + (c * W) / 5;
                    int cx1 = x0_inner + ((c + 1) * W) / 5 - 1;
                    drawRect(cx0, y0 + pad, cx1, y1 - pad, preset_palettes[preset_idx][c]);
                }
            }
            
            drawModalButtonAt(74, 114, 154, 170, "<-", false);
            renderDrawText(page_lbl, 122, 158, RGB15(31, 31, 31), 0);
            drawModalButtonAt(142, 182, 154, 170, "->", false);
        } else {
            char page_lbl[16];
            sprintf(page_lbl, "%d/10", custom_page + 1);
            
            for (int i = 0; i < 5; i++) {
                int global_idx = custom_page * 5 + i;
                int x0 = 12 + i * 48;
                int x1 = x0 + 38;
                int y0 = 134, y1 = 152;
                
                bool is_sel = (selected_custom_slot == i);
                uint16_t outline = is_sel ? app_theme_color : RGB15(0, 0, 0);
                drawRectOutline(x0, y0, x1, y1, outline);
                if (is_sel) {
                    drawRectOutline(x0 + 1, y0 + 1, x1 - 1, y1 - 1, outline);
                }
                
                int pad = is_sel ? 2 : 1;
                int x0_inner = x0 + pad;
                int x1_inner = x1 - pad;
                int W = x1_inner - x0_inner + 1;
                for (int c = 0; c < 5; c++) {
                    int cx0 = x0_inner + (c * W) / 5;
                    int cx1 = x0_inner + ((c + 1) * W) / 5 - 1;
                    drawRect(cx0, y0 + pad, cx1, y1 - pad, custom_palettes[global_idx][c]);
                }
            }
            
            drawModalButtonAt(12, 82, 154, 170, "GUARDAR", false);
            drawModalButtonAt(114, 144, 154, 170, "<-", false);
            drawModalButtonAt(174, 204, 154, 170, "->", false);
            renderDrawText(page_lbl, 210, 158, RGB15(31, 31, 31), 0);
        }
    }
    else if (open_modal == 3) {
        uint16_t tab_active_bg = blendRGB555_int(app_theme_color, RGB15(4, 4, 5), 6);
        uint16_t tab_inactive_bg = RGB15(2, 2, 3);
        uint16_t tab_border = app_theme_color;
        
        drawRect(8, 20, 127, 32, (bg_modal_tab == 0) ? tab_active_bg : tab_inactive_bg);
        drawRect(128, 20, 247, 32, (bg_modal_tab == 1) ? tab_active_bg : tab_inactive_bg);
        
        drawRectOutline(8, 20, 127, 32, (bg_modal_tab == 0) ? tab_border : blendRGB555_int(app_theme_color, RGB15(2,2,3), 8));
        drawRectOutline(128, 20, 247, 32, (bg_modal_tab == 1) ? tab_border : blendRGB555_int(app_theme_color, RGB15(2,2,3), 8));
        
        renderDrawText("PATRONES", 44, 23, (bg_modal_tab == 0) ? RGB15(31, 31, 31) : RGB15(15, 15, 15), 0);
        renderDrawText("PERSPECTIVA", 152, 23, (bg_modal_tab == 1) ? RGB15(31, 31, 31) : RGB15(15, 15, 15), 0);
        
        if (bg_modal_tab == 0) {
            for (int i = 0; i < 4; i++) {
                drawPatternPreview(12 + i * 58, 38, 12 + i * 58 + 52, 78, i, (bg_pattern_idx == i));
            }
            for (int i = 0; i < 4; i++) {
                drawPatternPreview(12 + i * 58, 84, 12 + i * 58 + 52, 124, 4 + i, (bg_pattern_idx == 4 + i));
            }
            
            char color_p_lbl[16];
            sprintf(color_p_lbl, "COL P:%d", bg_color_p_idx);
            drawModalButtonAt(12, 70, 132, 150, color_p_lbl, false);
            
            char color_s_lbl[16];
            sprintf(color_s_lbl, "COL S:%d", bg_color_s_idx);
            drawModalButtonAt(74, 132, 132, 150, color_s_lbl, false);
            
            drawModalButtonAt(136, 194, 132, 150, bg_modifiable ? "MOD:SI" : "MOD:NO", bg_modifiable);
            
            char rot_lbl[16];
            sprintf(rot_lbl, "ROT:%d", bg_angle);
            drawModalButtonAt(198, 244, 132, 150, rot_lbl, false);
        } else {
            renderDrawText("MODO DE PERSPECTIVA", 16, 36, RGB15(31, 31, 31), 0);
            
            drawModalButtonAt(12, 53, 46, 62, "OFF", (perspective_mode == 0));
            drawModalButtonAt(57, 101, 46, 62, "1 VP", (perspective_mode == 1));
            drawModalButtonAt(105, 149, 46, 62, "2 VP", (perspective_mode == 2));
            drawModalButtonAt(153, 197, 46, 62, "3 VP", (perspective_mode == 3));
            drawModalButtonAt(201, 244, 46, 62, "4 VP", (perspective_mode == 4));
            
            if (perspective_mode > 0) {
                drawModalButtonAt(12, 244, 68, 84, "REUBICAR TODOS LOS PUNTOS", false);
            } else {
                drawModalButtonAt(12, 244, 68, 84, "(SIN PERSPECTIVA)", false);
            }
            
            // Individual point editing buttons
            if (perspective_mode >= 1) {
                drawModalButtonAt(12, 65, 90, 106, "PUNTO 1", false);
            } else {
                drawModalButtonDisabledAt(12, 65, 90, 106, "PUNTO 1");
            }
            
            if (perspective_mode >= 2) {
                drawModalButtonAt(71, 124, 90, 106, "PUNTO 2", false);
            } else {
                drawModalButtonDisabledAt(71, 124, 90, 106, "PUNTO 2");
            }
            
            if (perspective_mode >= 3) {
                drawModalButtonAt(130, 183, 90, 106, "PUNTO 3", false);
            } else {
                drawModalButtonDisabledAt(130, 183, 90, 106, "PUNTO 3");
            }
            
            if (perspective_mode >= 4) {
                drawModalButtonAt(189, 244, 90, 106, "PUNTO 4", false);
            } else {
                drawModalButtonDisabledAt(189, 244, 90, 106, "PUNTO 4");
            }
            
            char dens_lbl[32];
            sprintf(dens_lbl, "DENSIDAD DE REJILLA: %d px", perspective_step);
            drawModalButtonAt(12, 244, 112, 128, dens_lbl, false);
        }
    } 
    else if (open_modal == 4) {
        char label[32];
        if (angle_target == 1) {
            sprintf(label, "ROTACION FONDO: %d", bg_angle);
        } else {
            sprintf(label, "ANGULO DE LA PLUMA: %d", nib_angle);
        }
        renderDrawText(label, 16, 96, RGB15(31, 31, 31), 0);
        
        int cx = 128;
        int cy = 132;
        int r = 28;
        drawCircleOutline(cx, cy, r, app_theme_color);
        
        drawRect(cx - 2, cy, cx + 2, cy, RGB15(15, 15, 15));
        drawRect(cx, cy - 2, cx, cy + 2, RGB15(15, 15, 15));
        
        int current_angle = (angle_target == 1) ? bg_angle : nib_angle;
        float rad = current_angle * 3.14159265f / 180.0f;
        int end_x = cx + (int)(cosf(rad) * 26.0f);
        int end_y = cy + (int)(sinf(rad) * 26.0f);
        drawLine(cx, cy, end_x, end_y, RGB15(31, 0, 0));
    }
    else if (open_modal == 5) {
        // MENU DE LA NOTA
        renderDrawText("MENU DE LA NOTA", 80, y0 + 6, RGB15(31, 31, 31), 0);
        
        // Draw options buttons
        drawModalButtonAt(24, 232, y0 + 22, y0 + 44, "GUARDAR NOTA (SD)", false);
        drawModalButtonAt(24, 232, y0 + 50, y0 + 72, "WIFI / CONEXION", false);
        drawModalButtonAt(24, 232, y0 + 78, y0 + 100, "VOLVER AL MENU INICIO", false);
        drawModalButtonAt(24, 232, y0 + 106, y0 + 128, "CANCELAR", false);
    }
    else if (open_modal == 6) {
        // ¿GUARDAR ANTES DE SALIR?
        renderDrawText("¿GUARDAR ANTES DE SALIR?", 56, y0 + 8, RGB15(31, 5, 5), 0);
        
        // Draw confirmation buttons
        drawModalButtonAt(24, 232, y0 + 26, y0 + 48, "SI, GUARDAR Y SALIR", false);
        drawModalButtonAt(24, 232, y0 + 54, y0 + 76, "NO, SALIR SIN GUARDAR", false);
        drawModalButtonAt(24, 232, y0 + 82, y0 + 104, "CANCELAR", false);
    }
}

static void drawFilledCircle(int xm, int ym, int r, uint16_t color) {
    int x = -r, y = 0, err = 2-2*r;
    do {
        for (int i = xm + x; i <= xm - x; i++) {
            if (i >= 0 && i < 256 && ym + y >= 0 && ym + y < 192) {
                canvas_buffer[(ym + y) * 256 + i] = color;
            }
            if (i >= 0 && i < 256 && ym - y >= 0 && ym - y < 192) {
                canvas_buffer[(ym - y) * 256 + i] = color;
            }
        }
        r = err;
        if (r <= y) err += ++y*2+1;
        if (r > x || err > y) err += ++x*2+1;
    } while (x < 0);
}

void uiUpdateAngleWheelVisuals(void) {
    if (open_modal != 4) return;
    
    uint16_t light_grey = RGB15(28, 28, 28);
    
    // Clear old text area and redraw it
    drawRect(16, 96, 220, 104, light_grey);
    char label[32];
    if (angle_target == 1) {
        sprintf(label, "ROTACION FONDO: %d", bg_angle);
    } else {
        sprintf(label, "ANGULO DE LA PLUMA: %d", nib_angle);
    }
    renderDrawText(label, 16, 96, RGB15(0, 0, 0), 0);
    
    // Clear circle interior only (radius 27)
    int cx = 128;
    int cy = 132;
    drawFilledCircle(cx, cy, 27, light_grey);
    
    // Redraw crosshair inside circle
    drawRect(cx - 2, cy, cx + 2, cy, RGB15(15, 15, 15));
    drawRect(cx, cy - 2, cx, cy + 2, RGB15(15, 15, 15));
    
    // Draw the new angle line
    int current_angle = (angle_target == 1) ? bg_angle : nib_angle;
    float rad = current_angle * 3.14159265f / 180.0f;
    int end_x = cx + (int)(cosf(rad) * 26.0f);
    int end_y = cy + (int)(sinf(rad) * 26.0f);
    drawLine(cx, cy, end_x, end_y, RGB15(31, 0, 0));
}

void uiCloseModal(void) {
    if (open_modal == -1) return;
    int y0, y1;
    getModalYRange(open_modal, &y0, &y1);
    for (int y = y0; y < 176; y++) {
        for (int x = 0; x < 256; x++) {
            canvas_buffer[y * 256 + x] = modal_backup[(y - y0) * 256 + x];
        }
    }
    open_modal = -1;
}

void uiUpdateModalBackup(void) {
    if (open_modal == -1) return;
    int y0, y1;
    getModalYRange(open_modal, &y0, &y1);
    
    renderComposeCanvas();
    
    for (int y = y0; y < 176; y++) {
        for (int x = 0; x < 256; x++) {
            modal_backup[(y - y0) * 256 + x] = canvas_buffer[y * 256 + x];
        }
    }
    
    uiOpenModal(open_modal);
}

void uiDrawToolbar(void) {
    // Fill toolbar background (dark blue-gray)
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

    // Highlight the active open modal button
    uint16_t active_bg = blendRGB555_int(app_theme_color, RGB15(4, 4, 5), 8);
    if (open_modal == 0) fillButtonBg(0, 42, active_bg);
    if (open_modal == 1) fillButtonBg(42, 84, active_bg);
    if (open_modal == 2) fillButtonBg(84, 126, active_bg);
    if (open_modal == 3) fillButtonBg(126, 168, active_bg);

    // Highlight Config (Button 4) and Publicar (Button 5)
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
    const char* tool_label = "PINC";
    if (is_eraser) tool_label = "BORR";
    else if (is_bucket) tool_label = "FILL";
    renderDrawText(tool_label, 9, 180, (open_modal == 0) ? active_text_color : default_text_color, 0);

    // Button 1: SIZE
    char size_label[6];
    if (is_eraser) sprintf(size_label, "S:%d", eraser_size);
    else sprintf(size_label, "S:%d", active_brush_size);
    renderDrawText(size_label, 50, 180, (open_modal == 1) ? active_text_color : default_text_color, 0);

    // Button 2: COLOR
    uint16_t current_color = palette_colors[active_color_idx];
    renderDrawText("COL", 90, 180, (open_modal == 2) ? active_text_color : default_text_color, 0);
    for (int dy = 0; dy < 6; dy++) {
        for (int dx = 0; dx < 6; dx++) {
            renderSetPixel(112 + dx, 181 + dy, current_color);
        }
    }

    // Button 3: BG
    char bg_label[6];
    sprintf(bg_label, "BG%d", bg_pattern_idx);
    renderDrawText(bg_label, 138, 180, (open_modal == 3) ? active_text_color : default_text_color, 0);

    // Button 4: MENU
    renderDrawText("MENU", 178, 180, (open_modal == 5) ? active_text_color : default_text_color, 0);

    // Button 5: ENVIAR
    renderDrawText("ENVIAR", 216, 180, RGB15(20, 31, 20), 0);
}

#include "font8x8.h"

static void drawSubPixel(int x, int y, uint16_t color) {
    if (x >= 0 && x < 256 && y >= 0 && y < 192 && wizard_buffer != NULL) {
        wizard_buffer[y * 256 + x] = color;
    }
}

static void drawSubRect(int x0, int y0, int x1, int y1, uint16_t color) {
    for (int y = y0; y <= y1; y++) {
        for (int x = x0; x <= x1; x++) {
            drawSubPixel(x, y, color);
        }
    }
}

static void drawSubRectOutline(int x0, int y0, int x1, int y1, uint16_t color) {
    for (int x = x0; x <= x1; x++) {
        drawSubPixel(x, y0, color);
        drawSubPixel(x, y1, color);
    }
    for (int y = y0; y <= y1; y++) {
        drawSubPixel(x0, y, color);
        drawSubPixel(x1, y, color);
    }
}

static void drawSubLine(int x0, int y0, int x1, int y1, uint16_t color) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        drawSubPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

static void drawSubChar(char c, int x, int y, uint16_t color, uint16_t bg_color) {
    if ((unsigned char)c < 32) return;
    const unsigned char *glyph = &font5x7[((unsigned char)c) * 5];
    for (int dx = 0; dx < 5; dx++) {
        unsigned char col_data = glyph[dx];
        for (int dy = 0; dy < 8; dy++) {
            if (col_data & (1 << dy)) {
                drawSubPixel(x + dx, y + dy, color);
            } else if (bg_color != 0) {
                drawSubPixel(x + dx, y + dy, bg_color);
            }
        }
    }
}

static void drawSubText(const char* text, int x, int y, uint16_t color, uint16_t bg_color) {
    while (*text) {
        drawSubChar(*text, x, y, color, bg_color);
        x += 6;
        text++;
    }
}

static void drawBlueArrow(int x, int y) {
    uint16_t color = RGB15(0, 16, 30);
    drawSubPixel(x, y + 3, color);
    drawSubPixel(x + 1, y + 2, color);
    drawSubPixel(x + 1, y + 3, color);
    drawSubPixel(x + 1, y + 4, color);
    drawSubPixel(x + 2, y + 1, color);
    drawSubPixel(x + 2, y + 2, color);
    drawSubPixel(x + 2, y + 3, color);
    drawSubPixel(x + 2, y + 4, color);
    drawSubPixel(x + 2, y + 5, color);
    drawSubPixel(x + 3, y, color);
    drawSubPixel(x + 3, y + 1, color);
    drawSubPixel(x + 3, y + 2, color);
    drawSubPixel(x + 3, y + 3, color);
    drawSubPixel(x + 3, y + 4, color);
    drawSubPixel(x + 3, y + 5, color);
    drawSubPixel(x + 3, y + 6, color);
}



static void drawSubGridBackground(void) {
    drawSubRect(0, 0, 255, 191, RGB15(4, 4, 5));
    uint16_t grid_color = RGB15(6, 6, 8);
    for (int y = 0; y < 192; y += 16) {
        for (int x = 0; x < 256; x += 16) {
            drawSubPixel(x, y, grid_color);
        }
    }
}

void uiDrawFormUI(int step, const char* input_text) {
    if (wizard_buffer == NULL) return;

    drawSubGridBackground();

    uint16_t text_col = RGB15(31, 31, 31);
    uint16_t panel_bg = RGB15(2, 2, 3);
    uint16_t header_bg = blendRGB555_int(app_theme_color, RGB15(2, 2, 3), 6);
    uint16_t border_col = app_theme_color;
    uint16_t inactive_text = RGB15(15, 15, 15);
    uint16_t active_col = app_theme_color;
    uint16_t header_text_col = text_col;
    
    if (active_theme_idx == 0) {
        header_text_col = RGB15(0, 0, 0);
    }

    // 1. Header (Connection Settings)
    drawSubRect(0, 0, 255, 15, header_bg);
    drawSubLine(0, 16, 255, 16, border_col);
    drawSubText("Connection Settings", 8, 4, header_text_col, 0);

    // 2. Footer (Cancel / Save)
    drawSubRect(0, 176, 255, 191, header_bg);
    drawSubLine(0, 175, 255, 175, border_col);

    // [B] Cancel
    drawSubRectOutline(8, 180, 20, 188, border_col);
    drawSubText("B", 11, 181, header_text_col, 0);
    drawSubText("Cancel", 24, 181, header_text_col, 0);

    // [A] Save
    drawSubRectOutline(204, 180, 216, 188, border_col);
    drawSubText("A", 207, 181, header_text_col, 0);
    drawSubText("Save", 220, 181, header_text_col, 0);

    // 3. Left Panel (Connection 1 / Connection 2) - Shrunk height to 110
    drawSubRect(6, 22, 86, 110, panel_bg);
    drawSubRectOutline(6, 22, 86, 110, border_col);

    drawSubText("Conn. 1", 10, 26, text_col, 0);

    // Connection 1 Box (HTTP selected)
    drawSubRectOutline(8, 36, 84, 52, active_col);
    drawSubRectOutline(9, 37, 83, 51, active_col);
    drawBlueArrow(14, 41); // We'll let it stay as drawBlueArrow, the sprite itself might be blue but that's fine for now
    drawSubText("HTTP", 24, 40, active_col, 0);

    // Dotted separator
    for (int x = 8; x < 84; x += 2) {
        drawSubPixel(x, 58, inactive_text);
    }

    drawSubText("Conn. 2", 10, 64, text_col, 0);
    drawSubText("Not Config", 10, 74, inactive_text, 0);

    // Center help description text on the grid background below the panels
    drawSubText("Configure your HTTP server.", 47, 136, text_col, 0);

    // 4. Right Panel (HTTP Configuration Fields) - Shrunk height to 110
    drawSubRect(92, 22, 250, 110, panel_bg);
    drawSubRectOutline(92, 22, 250, 110, border_col);

    // Inner Header (HTTP Configuration)
    drawSubRect(93, 23, 249, 36, blendRGB555_int(app_theme_color, RGB15(4, 4, 5), 8));
    drawSubLine(92, 37, 250, 37, border_col);
    drawSubText("HTTP Configuration", 96, 26, header_text_col, 0);

    // Field 0: IP (y = 44 to 58)
    drawSubText("IP", 96, 48, text_col, 0);
    drawSubRectOutline(136, 44, 244, 58, (step == 0) ? active_col : RGB15(15, 15, 15));
    if (step == 0) {
        drawSubRectOutline(137, 45, 243, 57, active_col);
        drawSubText(input_text, 140, 47, text_col, 0);
    } else {
        drawSubText(http_ip, 140, 47, text_col, 0);
    }

    // Field 1: Port (y = 66 to 80)
    drawSubText("Port", 96, 70, text_col, 0);
    drawSubRectOutline(136, 66, 190, 80, (step == 1) ? active_col : RGB15(15, 15, 15));
    if (step == 1) {
        drawSubRectOutline(137, 67, 189, 79, active_col);
        drawSubText(input_text, 140, 69, text_col, 0);
    } else {
        drawSubText(http_port_str, 140, 69, text_col, 0);
    }

    // Field 2: SSID (y = 88 to 102)
    drawSubText("SSID", 96, 92, text_col, 0);
    drawSubRectOutline(136, 88, 244, 102, (step == 2) ? active_col : RGB15(15, 15, 15));
    if (step == 2) {
        drawSubRectOutline(137, 89, 243, 101, active_col);
        drawSubText(input_text, 140, 91, text_col, 0);
    } else {
        drawSubText(wifi_ssid, 140, 91, text_col, 0);
    }
}

static bool shift_active = false;
static bool caps_active = false;

static void drawKey(int x0, int y0, int x1, int y1, const char* label, bool highlighted) {
    uint16_t bg_color = highlighted ? blendRGB555_int(app_theme_color, RGB15(4, 4, 5), 6) : RGB15(2, 2, 3);
    uint16_t border_color = highlighted ? app_theme_color : blendRGB555_int(app_theme_color, RGB15(2, 2, 3), 8);
    uint16_t text_color = RGB15(31, 31, 31);
    if (highlighted && active_theme_idx == 0) {
        text_color = RGB15(0, 0, 0);
    }
    
    // Draw background
    for (int y = y0; y <= y1; y++) {
        for (int x = x0; x <= x1; x++) {
            canvas_buffer[y * 256 + x] = bg_color;
        }
    }
    // Draw border
    for (int x = x0; x <= x1; x++) {
        canvas_buffer[y0 * 256 + x] = border_color;
        canvas_buffer[y1 * 256 + x] = border_color;
    }
    for (int y = y0; y <= y1; y++) {
        canvas_buffer[y * 256 + x0] = border_color;
        canvas_buffer[y * 256 + x1] = border_color;
    }
    
    // Center label
    int len = strlen(label);
    int text_w = len * 6 - 1; // 5px per char + 1px spacing (excluding last character's spacing)
    if (text_w < 0) text_w = 0;
    int text_h = 7;
    int tx = x0 + (x1 - x0 + 1 - text_w) / 2;
    int ty = y0 + (y1 - y0 + 1 - text_h) / 2;
    renderDrawText(label, tx, ty, text_color, 0);
}

static void drawKeyboard(void) {
    bool upper = shift_active || caps_active;
    
    // Row 1: 1 to = and Bksp
    const char* r1_labels[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "="};
    for (int i = 0; i < 12; i++) {
        int x0 = 4 + i * 18;
        drawKey(x0, 96, x0 + 16, 116, r1_labels[i], false);
    }
    drawKey(220, 96, 252, 116, "<-", false);
    
    // Row 2: q to backslash
    const char* r2_labels_low[] = {"q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]", "\\"};
    const char* r2_labels_up[]  = {"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "{", "}", "|"};
    for (int i = 0; i < 13; i++) {
        int x0 = 4 + i * 19;
        drawKey(x0, 118, x0 + 17, 138, upper ? r2_labels_up[i] : r2_labels_low[i], false);
    }
    
    // Row 3: Caps, a to ' and Rtrn
    drawKey(4, 140, 29, 160, "CPS", caps_active);
    const char* r3_labels_low[] = {"a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "'"};
    const char* r3_labels_up[]  = {"A", "S", "D", "F", "G", "H", "J", "K", "L", ":", "\""};
    for (int i = 0; i < 11; i++) {
        int x0 = 31 + i * 18;
        drawKey(x0, 140, x0 + 16, 160, upper ? r3_labels_up[i] : r3_labels_low[i], false);
    }
    drawKey(229, 140, 253, 160, "Ent", false);
    
    // Row 4: Shift, z to /, Space
    drawKey(4, 162, 29, 182, "SFT", shift_active);
    const char* r4_labels_low[] = {"z", "x", "c", "v", "b", "n", "m", ",", ".", "/"};
    const char* r4_labels_up[]  = {"Z", "X", "C", "V", "B", "N", "M", "<", ">", "?"};
    for (int i = 0; i < 10; i++) {
        int x0 = 31 + i * 18;
        drawKey(x0, 162, x0 + 16, 182, upper ? r4_labels_up[i] : r4_labels_low[i], false);
    }
    drawKey(211, 162, 254, 182, "Space", false);
}

void uiDrawBottomForm(int step, const char* input_text) {
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
    
    for (int y = 8; y <= 36; y++) {
        for (int x = 10; x <= 246; x++) {
            canvas_buffer[y * 256 + x] = RGB15(2, 2, 3);
        }
    }
    drawRectOutline(10, 8, 246, 36, app_theme_color);
    
    char label[128];
    if (step == 0)      sprintf(label, "IP: %s_", input_text);
    else if (step == 1) sprintf(label, "PORT: %s_", input_text);
    else if (step == 2) sprintf(label, "SSID: %s_", input_text);
    renderDrawText(label, 16, 17, RGB15(31, 31, 31), 0);
    
    drawKey(10, 42, 70, 62, "IP", (step == 0));
    drawKey(76, 42, 136, 62, "PORT", (step == 1));
    drawKey(142, 42, 202, 62, "SSID", (step == 2));
    
    drawKeyboard();
}

char uiHandleKeyboardTouch(int tx, int ty, bool* shift_toggled, bool* caps_toggled, bool* enter_pressed, bool* backspace_pressed) {
    *shift_toggled = false;
    *caps_toggled = false;
    *enter_pressed = false;
    *backspace_pressed = false;
    
    bool upper = shift_active || caps_active;
    
    if (ty >= 96 && ty <= 116) {
        for (int i = 0; i < 12; i++) {
            int x0 = 4 + i * 18;
            if (tx >= x0 && tx <= x0 + 16) {
                const char* r1_labels[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "="};
                return r1_labels[i][0];
            }
        }
        if (tx >= 220 && tx <= 252) {
            *backspace_pressed = true;
            return 0;
        }
    }
    
    if (ty >= 118 && ty <= 138) {
        for (int i = 0; i < 13; i++) {
            int x0 = 4 + i * 19;
            if (tx >= x0 && tx <= x0 + 17) {
                const char* r2_labels_low[] = {"q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]", "\\"};
                const char* r2_labels_up[]  = {"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "{", "}", "|"};
                return upper ? r2_labels_up[i][0] : r2_labels_low[i][0];
            }
        }
    }
    
    if (ty >= 140 && ty <= 160) {
        if (tx >= 4 && tx <= 29) {
            *caps_toggled = true;
            caps_active = !caps_active;
            return 0;
        }
        for (int i = 0; i < 11; i++) {
            int x0 = 31 + i * 18;
            if (tx >= x0 && tx <= x0 + 16) {
                const char* r3_labels_low[] = {"a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "'"};
                const char* r3_labels_up[]  = {"A", "S", "D", "F", "G", "H", "J", "K", "L", ":", "\""};
                return upper ? r3_labels_up[i][0] : r3_labels_low[i][0];
            }
        }
        if (tx >= 229 && tx <= 253) {
            *enter_pressed = true;
            return 0;
        }
    }
    
    if (ty >= 162 && ty <= 182) {
        if (tx >= 4 && tx <= 29) {
            *shift_toggled = true;
            shift_active = !shift_active;
            return 0;
        }
        for (int i = 0; i < 10; i++) {
            int x0 = 31 + i * 18;
            if (tx >= x0 && tx <= x0 + 16) {
                const char* r4_labels_low[] = {"z", "x", "c", "v", "b", "n", "m", ",", ".", "/"};
                const char* r4_labels_up[]  = {"Z", "X", "C", "V", "B", "N", "M", "<", ">", "?"};
                char c = upper ? r4_labels_up[i][0] : r4_labels_low[i][0];
                if (shift_active) {
                    shift_active = false;
                    *shift_toggled = true;
                }
                return c;
            }
        }
        if (tx >= 211 && tx <= 254) {
            if (shift_active) {
                shift_active = false;
                *shift_toggled = true;
            }
            return ' ';
        }
    }
    
    return 0;
}

static void uiDrawLayersSidebar(void) {
    uint16_t panel_bg = RGB15(4, 4, 5);
    uint16_t border_col = app_theme_color;
    uint16_t text_col = RGB15(31, 31, 31);
    
    // Fill sidebar region: x = 144..255, y = 0..176
    for (int y = 0; y < 176; y++) {
        for (int x = 144; x < 256; x++) {
            canvas_buffer[y * 256 + x] = panel_bg;
        }
        canvas_buffer[y * 256 + 143] = border_col;
    }
    
    // Header title "CAPAS"
    renderDrawText("CAPAS", 148, 2, text_col, 0);
    
    // Close button "X" (red on dark red) at x = 236..252, y = 1..11
    drawRect(236, 1, 252, 11, RGB15(6, 2, 2));
    drawRectOutline(236, 1, 252, 11, border_col);
    renderDrawText("X", 241, 2, RGB15(31, 10, 10), 0);
    
    // "+ CAPA" button at x = 148..252, y = 14..24
    if (layers_count < MAX_LAYERS) {
        uint16_t plus_capa_bg = blendRGB555_int(app_theme_color, panel_bg, 6);
        drawRect(148, 14, 252, 24, plus_capa_bg);
        drawRectOutline(148, 14, 252, 24, border_col);
        renderDrawText("+ CAPA", 182, 16, text_col, 0);
    } else {
        drawRect(148, 14, 252, 24, RGB15(2, 2, 3));
        drawRectOutline(148, 14, 252, 24, RGB15(6, 6, 8));
        renderDrawText("LLENO", 188, 16, RGB15(10, 10, 10), 0);
    }
    
    // Draw layer items from layers_count-1 down to 0
    for (int i = layers_count - 1; i >= 0; i--) {
        int idx_from_top = layers_count - 1 - i;
        int y_pos = 27 + idx_from_top * 14;
        
        // Drag handle: circle at cx = 151, cy = y_pos + 6
        uint16_t circle_col = (dragging_layer_idx == i) ? app_theme_color : RGB15(8, 8, 10);
        // Draw filled circle (radius 2)
        for (int dy = -2; dy <= 2; dy++) {
            for (int dx = -2; dx <= 2; dx++) {
                if (dx*dx + dy*dy <= 5) {
                    int px = 151 + dx;
                    int py = y_pos + 6 + dy;
                    if (px >= 144 && px < 256 && py >= 0 && py < 192) {
                        canvas_buffer[py * 256 + px] = circle_col;
                    }
                }
            }
        }
        // Circle border
        for (int dy = -3; dy <= 3; dy++) {
            for (int dx = -3; dx <= 3; dx++) {
                if (dx*dx + dy*dy > 5 && dx*dx + dy*dy <= 10) {
                    int px = 151 + dx;
                    int py = y_pos + 6 + dy;
                    if (px >= 144 && px < 256 && py >= 0 && py < 192) {
                        canvas_buffer[py * 256 + px] = RGB15(0, 0, 0);
                    }
                }
            }
        }
        
        // Active indicator / selection button: x = 158..212, y = y_pos..y_pos+12
        bool is_active = (active_layer_idx == i);
        uint16_t btn_bg = is_active ? blendRGB555_int(app_theme_color, panel_bg, 8) : RGB15(2, 2, 3);
        uint16_t btn_border = is_active ? app_theme_color : RGB15(6, 6, 8);
        uint16_t btn_txt = is_active ? (active_theme_idx == 0 ? RGB15(0, 0, 0) : RGB15(31, 31, 31)) : RGB15(20, 20, 22);
        
        drawRect(158, y_pos, 212, y_pos + 12, btn_bg);
        drawRectOutline(158, y_pos, 212, y_pos + 12, btn_border);
        
        char disp_name[7];
        strncpy(disp_name, layer_names[i], 6);
        disp_name[6] = '\0';
        renderDrawText(disp_name, 162, y_pos + 2, btn_txt, 0);
        
        // Visibility toggle button: x = 216..232, y = y_pos..y_pos+12
        bool is_visible = layers_visible[i];
        uint16_t vis_bg = is_visible ? blendRGB555_int(app_theme_color, panel_bg, 6) : RGB15(2, 2, 3);
        uint16_t vis_border = is_visible ? app_theme_color : RGB15(6, 6, 8);
        uint16_t vis_txt = is_visible ? (is_active && active_theme_idx == 0 ? RGB15(0, 0, 0) : RGB15(31, 31, 31)) : RGB15(10, 10, 10);
        
        drawRect(216, y_pos, 232, y_pos + 12, vis_bg);
        drawRectOutline(216, y_pos, 232, y_pos + 12, vis_border);
        renderDrawText(is_visible ? "V" : "H", 220, y_pos + 2, vis_txt, 0);
        
        // Delete button: x = 236..252, y = y_pos..y_pos+12
        if (i > 0) {
            drawRect(236, y_pos, 252, y_pos + 12, RGB15(12, 3, 3));
            drawRectOutline(236, y_pos, 252, y_pos + 12, RGB15(6, 2, 2));
            renderDrawText("X", 241, y_pos + 2, RGB15(31, 10, 10), 0);
        } else {
            drawRect(236, y_pos, 252, y_pos + 12, RGB15(2, 2, 3));
            drawRectOutline(236, y_pos, 252, y_pos + 12, RGB15(6, 6, 8));
            renderDrawText("-", 241, y_pos + 2, RGB15(10, 10, 10), 0);
        }
    }
    
    // Draw "FONDO" item at bg_y = 27 + layers_count * 14
    int bg_y = 27 + layers_count * 14;
    drawRect(148, bg_y, 212, bg_y + 12, RGB15(2, 2, 3));
    drawRectOutline(148, bg_y, 212, bg_y + 12, RGB15(6, 6, 8));
    renderDrawText("FONDO", 158, bg_y + 2, text_col, 0);
    
    // Lock toggle: x = 216..252, y = bg_y..bg_y+12
    drawRect(216, bg_y, 252, bg_y + 12, bg_modifiable ? blendRGB555_int(app_theme_color, panel_bg, 6) : RGB15(12, 3, 3));
    drawRectOutline(216, bg_y, 252, bg_y + 12, bg_modifiable ? border_col : RGB15(6, 2, 2));
    renderDrawText(bg_modifiable ? " MOD" : " LOK", 218, bg_y + 2, bg_modifiable ? text_col : RGB15(31, 10, 10), 0);
    drawLockIcon(244, bg_y + 1, !bg_modifiable);
    
    // Draw "COMBINAR" button at y = 157..169
    bool can_merge = (active_layer_idx > 0);
    uint16_t merge_bg = can_merge ? blendRGB555_int(app_theme_color, panel_bg, 8) : RGB15(2, 2, 3);
    uint16_t merge_border = can_merge ? app_theme_color : RGB15(6, 6, 8);
    uint16_t merge_txt = can_merge ? (active_theme_idx == 0 ? RGB15(0, 0, 0) : RGB15(31, 31, 31)) : RGB15(10, 10, 10);
    
    drawRect(148, 157, 252, 169, merge_bg);
    drawRectOutline(148, 157, 252, 169, merge_border);
    renderDrawText("COMBINAR", 180, 160, merge_txt, 0);
}

void uiDrawLayersOverlay(void) {
    if (layers_panel_open) {
        uiDrawLayersSidebar();
    } else {
        // Draw a small floating tab button on the right edge: x = 244..255, y = 70..106
        uint16_t tab_bg = RGB15(2, 2, 3);
        uint16_t tab_border = app_theme_color;
        
        drawRect(244, 70, 255, 106, tab_bg);
        drawRectOutline(244, 70, 255, 106, tab_border);
        
        // Stack of three sheets
        uint16_t sheet_col = blendRGB555_int(app_theme_color, RGB15(31, 31, 31), 16);
        drawRect(247, 76, 252, 78, sheet_col);
        drawRect(247, 82, 252, 84, sheet_col);
        drawRect(247, 88, 252, 90, sheet_col);
        
        renderDrawText("<", 247, 96, RGB15(31, 31, 31), 0);
    }
}

void uiDrawRenameKeyboard(const char* input_text, uint8_t opacity) {
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
    
    // Header banner: "PROPIEDADES DE CAPA"
    uint16_t header_bg = blendRGB555_int(app_theme_color, bg_color, 8);
    drawRect(0, 0, 255, 12, header_bg);
    drawRect(0, 12, 255, 12, app_theme_color);
    renderDrawText("PROPIEDADES DE CAPA", 8, 2, RGB15(31, 31, 31), 0);
    
    // Text input box outline and fill
    for (int y = 18; y <= 36; y++) {
        for (int x = 10; x <= 246; x++) {
            canvas_buffer[y * 256 + x] = RGB15(2, 2, 3);
        }
    }
    drawRectOutline(10, 18, 246, 36, app_theme_color);
    
    char label[128];
    sprintf(label, "%s_", input_text);
    renderDrawText(label, 16, 23, RGB15(31, 31, 31), 0);
    
    // Cancel & Save Buttons:
    // Cancel (B) at x = 10..90, y = 44..62
    drawKey(10, 44, 90, 62, "CANCEL (B)", false);
    
    // Save (A) at x = 166..246, y = 44..62
    drawKey(166, 44, 246, 62, "SAVE (A)", false);
    
    // Opacity Selector:
    renderDrawText("OPACIDAD:", 10, 73, RGB15(31, 31, 31), 0);
    drawKey(100, 68, 124, 86, "-", false);
    
    char op_val_str[16];
    sprintf(op_val_str, "%d%%", opacity);
    renderDrawText(op_val_str, 138, 73, RGB15(31, 31, 31), 0);
    
    drawKey(176, 68, 200, 86, "+", false);
    
    drawKeyboard();
}

void uiDrawUndoRedoButtons(void) {
    uint16_t btn_bg = RGB15(4, 4, 5);
    uint16_t border_col = app_theme_color;
    
    // Undo
    drawRect(4, 4, 20, 20, btn_bg);
    drawRectOutline(4, 4, 20, 20, border_col);
    uint16_t undo_text_col = (undo_count > 0) ? RGB15(31, 31, 31) : RGB15(8, 8, 9);
    renderDrawText("<", 9, 8, undo_text_col, 0);
    
    // Redo
    drawRect(24, 4, 40, 20, btn_bg);
    drawRectOutline(24, 4, 40, 20, border_col);
    uint16_t redo_text_col = (redo_count > 0) ? RGB15(31, 31, 31) : RGB15(8, 8, 9);
    renderDrawText(">", 29, 8, redo_text_col, 0);
}

void uiDrawLogo(void) {
    if (wizard_buffer == NULL) return;
    uint16_t bg_color = RGB15(4, 4, 5); // dark charcoal background
    for (int y = 0; y < 192; y++) {
        for (int x = 0; x < 256; x++) {
            uint8_t alpha = logo_data[y * 256 + x]; // 0 to 255
            // Convert alpha 0-255 to 0-32
            int alpha_32 = alpha >> 3;
            uint16_t blended_pixel = blendRGB555_int(app_theme_color, bg_color, alpha_32);
            wizard_buffer[y * 256 + x] = blended_pixel;
        }
    }
    // Clean remaining lines of 256x256
    for (int y = 192; y < 256; y++) {
        for (int x = 0; x < 256; x++) {
            wizard_buffer[y * 256 + x] = bg_color;
        }
    }
}

void uiDrawStartMenu(void) {
    // Fill bottom screen with dark charcoal
    uint16_t bg_color = RGB15(4, 4, 5);
    for (int y = 0; y < 192; y++) {
        for (int x = 0; x < 256; x++) {
            canvas_buffer[y * 256 + x] = bg_color;
        }
    }
    
    // Draw grid
    uint16_t grid_color = RGB15(8, 8, 10);
    for (int y = 0; y < 192; y += 16) {
        for (int x = 0; x < 256; x += 16) {
            renderSetPixel(x, y, grid_color);
        }
    }
    
    // Draw welcome title
    renderDrawText("BIENVENIDO A OVENOTES DS", 32, 24, app_theme_color, 0);
    
    // Button 1: Crear Nueva Nota
    // x = 32..224, y = 56..80
    drawRect(32, 56, 224, 80, RGB15(10, 10, 12));
    drawRectOutline(32, 56, 224, 80, app_theme_color);
    renderDrawText("CREAR NUEVA NOTA", 64, 64, RGB15(31, 31, 31), 0);
    
    // Button 2: Ver Notas Creadas
    // x = 32..224, y = 92..116
    drawRect(32, 92, 224, 116, RGB15(10, 10, 12));
    drawRectOutline(32, 92, 224, 116, app_theme_color);
    renderDrawText("VER NOTAS CREADAS", 60, 100, RGB15(31, 31, 31), 0);
    
    // Button 3: Cambiar Tema
    // x = 32..224, y = 128..152
    drawRect(32, 128, 224, 152, RGB15(10, 10, 12));
    drawRectOutline(32, 128, 224, 152, app_theme_color);
    char theme_lbl[48];
    sprintf(theme_lbl, "TEMA: %s", theme_names[active_theme_idx]);
    int pad = (24 - strlen(theme_lbl)) * 4;
    if (pad < 0) pad = 0;
    renderDrawText(theme_lbl, 44 + pad, 136, RGB15(31, 31, 31), 0);
    
    // Bottom instructions
    renderDrawText("Toca una opcion para comenzar", 20, 172, RGB15(20, 20, 22), 0);
    
    uiDrawLogo();
}

void uiDrawNotesGallery(int selected_idx, int total_count, const char filenames[][32]) {
    // Fill bottom screen with dark charcoal background
    uint16_t bg_color = RGB15(4, 4, 5);
    for (int y = 0; y < 192; y++) {
        for (int x = 0; x < 256; x++) {
            canvas_buffer[y * 256 + x] = bg_color;
        }
    }
    
    // Draw screen grid
    uint16_t grid_color = RGB15(8, 8, 10);
    for (int y = 0; y < 192; y += 16) {
        for (int x = 0; x < 256; x += 16) {
            renderSetPixel(x, y, grid_color);
        }
    }
    
    // Header
    drawRect(0, 0, 255, 14, RGB15(12, 12, 12));
    renderDrawText("VER NOTAS CREADAS", 8, 3, app_theme_color, 0);
    
    // Back button
    // x = 180..250, y = 1..13
    drawRect(180, 1, 250, 13, RGB15(24, 6, 6));
    drawRectOutline(180, 1, 250, 13, RGB15(31, 0, 0));
    renderDrawText("VOLVER(B)", 186, 3, RGB15(31, 31, 31), 0);
    
    // If no notes are present
    if (total_count == 0) {
        renderDrawText("No se encontraron notas.", 32, 80, RGB15(20, 20, 22), 0);
        renderDrawText("Crea una nota en el menu de inicio.", 16, 96, RGB15(16, 16, 18), 0);
        
        // Clean top screen preview to indicate empty
        if (wizard_buffer != NULL) {
            for (int i = 0; i < 256 * 256; i++) {
                wizard_buffer[i] = RGB15(0, 0, 0);
            }
            for (int y = 0; y < 192; y++) {
                for (int x = 0; x < 256; x++) {
                    if (x == 0 || x == 255 || y == 0 || y == 191) {
                        wizard_buffer[y * 256 + x] = app_theme_color;
                    }
                }
            }
        }
        return;
    }
    
    // Draw list of notes (max 5 visible at a time)
    int start_visible = (selected_idx / 5) * 5;
    int end_visible = start_visible + 5;
    if (end_visible > total_count) end_visible = total_count;
    
    int row_y = 24;
    for (int i = start_visible; i < end_visible; i++) {
        bool is_selected = (i == selected_idx);
        uint16_t row_bg = is_selected ? RGB15(12, 12, 18) : RGB15(6, 6, 8);
        uint16_t border_col = is_selected ? app_theme_color : RGB15(12, 12, 14);
        
        // Row box: x = 10..246, y = row_y..row_y+20
        drawRect(10, row_y, 246, row_y + 20, row_bg);
        drawRectOutline(10, row_y, 246, row_y + 20, border_col);
        
        // Note text
        char label[64];
        sprintf(label, "%s %s", is_selected ? ">" : " ", filenames[i]);
        renderDrawText(label, 18, row_y + 6, is_selected ? RGB15(31, 31, 31) : RGB15(24, 24, 24), 0);
        
        row_y += 26;
    }
    
    // Draw scrolling markers if needed
    if (start_visible > 0) {
        renderDrawText("^ MAS NOTAS ARRIBA ^", 48, 18, app_theme_color, 0);
    }
    if (end_visible < total_count) {
        renderDrawText("v MAS NOTAS ABAJO v", 52, 156, app_theme_color, 0);
    }
    
    // Instructions at the very bottom
    renderDrawText("Usa DPAD Arriba/Abajo o Stylus", 12, 172, RGB15(16, 16, 18), 0);
}
