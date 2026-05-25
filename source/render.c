#include "render.h"
#include "font8x8.h"
#include "ui.h"
#include <stdlib.h>
#include <string.h>

uint16_t* canvas_buffer = NULL;
uint16_t* preview_buffer = NULL;
uint16_t* wizard_buffer = NULL;
bool toolbar_hidden = false;

void renderSetPixel(int x, int y, uint16_t color) {
    if (x >= 0 && x < 256 && y >= 0 && y < 192) {
        canvas_buffer[y * 256 + x] = color;
    }
}

uint16_t blendRGB555_int(uint16_t src, uint16_t dst, int alpha_32) {
    int src_r = src & 0x1F;
    int src_g = (src >> 5) & 0x1F;
    int src_b = (src >> 10) & 0x1F;
    
    int dst_r = dst & 0x1F;
    int dst_g = (dst >> 5) & 0x1F;
    int dst_b = (dst >> 10) & 0x1F;
    
    int r = (src_r * alpha_32 + dst_r * (32 - alpha_32)) >> 5;
    int g = (src_g * alpha_32 + dst_g * (32 - alpha_32)) >> 5;
    int b = (src_b * alpha_32 + dst_b * (32 - alpha_32)) >> 5;
    
    return BIT(15) | r | (g << 5) | (b << 10);
}

void renderSetCanvasPixel(int x, int y, uint16_t color) {
    int max_y = toolbar_hidden ? 192 : 176;
    if (x >= 0 && x < 256 && y >= 0 && y < max_y) {
        if (color == RGB15(31, 31, 31)) {
            // Eraser mode: draw solid white
            canvas_buffer[y * 256 + x] = color;
        } else {
            // Brush mode: apply active color and drawing modes
            uint16_t brush_color = palette_colors[active_color_idx];
            if (drawing_mode == 0) {
                canvas_buffer[y * 256 + x] = brush_color;
            } else if (drawing_mode == 1) {
                uint16_t current = canvas_buffer[y * 256 + x];
                canvas_buffer[y * 256 + x] = blendRGB555_int(brush_color, current, 12);
            } else if (drawing_mode == 2) {
                if ((x + y) % 2 == 0) {
                    canvas_buffer[y * 256 + x] = brush_color;
                }
            } else if (drawing_mode == 3) {
                if ((x - y) % 4 == 0) {
                    canvas_buffer[y * 256 + x] = brush_color;
                }
            }
        }
    }
}

void renderApplyBackgroundPattern(int pat_index) {
    int limit_y = toolbar_hidden ? 192 : 176;
    
    // Base is white
    for (int y = 0; y < limit_y; y++) {
        for (int x = 0; x < 256; x++) {
            canvas_buffer[y * 256 + x] = RGB15(31, 31, 31);
        }
    }
    
    // Light grid color blended with active color
    uint16_t grid_color = blendRGB555_int(palette_colors[active_color_idx], RGB15(31, 31, 31), 6);
    
    if (pat_index == 1) {
        // Dotted grid: dots spaced by 16px
        for (int y = 8; y < limit_y; y += 16) {
            for (int x = 8; x < 256; x += 16) {
                canvas_buffer[y * 256 + x] = grid_color;
            }
        }
    } else if (pat_index == 2) {
        // Lined paper: horizontal lines spaced by 16px
        for (int y = 16; y < limit_y; y += 16) {
            for (int x = 0; x < 256; x++) {
                canvas_buffer[y * 256 + x] = grid_color;
            }
        }
    } else if (pat_index == 3) {
        // Checked grid: squares of 16px
        for (int y = 0; y < limit_y; y++) {
            for (int x = 0; x < 256; x++) {
                if (y % 16 == 0 || x % 16 == 0) {
                    canvas_buffer[y * 256 + x] = grid_color;
                }
            }
        }
    }
}

void renderFloodFill(int start_x, int start_y, uint16_t fill_color) {
    int limit_y = toolbar_hidden ? 192 : 176;
    if (start_x < 0 || start_x >= 256 || start_y < 0 || start_y >= limit_y) return;
    
    uint16_t target_color = canvas_buffer[start_y * 256 + start_x];
    if (target_color == fill_color) return;
    
    static int q_x[8192];
    static int q_y[8192];
    int head = 0;
    int tail = 0;
    
    q_x[tail] = start_x;
    q_y[tail] = start_y;
    tail = (tail + 1) % 8192;
    
    canvas_buffer[start_y * 256 + start_x] = fill_color;
    
    while (head != tail) {
        int cx = q_x[head];
        int cy = q_y[head];
        head = (head + 1) % 8192;
        
        int dx[4] = {0, 0, -1, 1};
        int dy[4] = {-1, 1, 0, 0};
        
        for (int i = 0; i < 4; i++) {
            int nx = cx + dx[i];
            int ny = cy + dy[i];
            
            if (nx >= 0 && nx < 256 && ny >= 0 && ny < limit_y) {
                if (canvas_buffer[ny * 256 + nx] == target_color) {
                    canvas_buffer[ny * 256 + nx] = fill_color;
                    
                    int next_tail = (tail + 1) % 8192;
                    if (next_tail != head) {
                        q_x[tail] = nx;
                        q_y[tail] = ny;
                        tail = next_tail;
                    }
                }
            }
        }
    }
}

void renderDrawChar(char c, int x, int y, uint16_t color, uint16_t bg_color) {
    if ((unsigned char)c < 32) return;
    const unsigned char *glyph = &font5x7[((unsigned char)c) * 5];
    for (int dx = 0; dx < 5; dx++) {
        unsigned char col_data = glyph[dx];
        for (int dy = 0; dy < 8; dy++) {
            if (col_data & (1 << dy)) {
                renderSetPixel(x + dx, y + dy, color);
            } else if (bg_color != 0) {
                renderSetPixel(x + dx, y + dy, bg_color);
            }
        }
    }
}

void renderDrawText(const char* text, int x, int y, uint16_t color, uint16_t bg_color) {
    while (*text) {
        renderDrawChar(*text, x, y, color, bg_color);
        x += 6;
        text++;
    }
}

void renderDrawBrushPoint(int xc, int yc, uint16_t color, int size) {
    if (size == 1) {
        renderSetCanvasPixel(xc, yc, color);
    } else if (size <= 3) {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (dx*dx + dy*dy <= 2) {
                    renderSetCanvasPixel(xc + dx, yc + dy, color);
                }
            }
        }
    } else {
        for (int dy = -2; dy <= 2; dy++) {
            for (int dx = -2; dx <= 2; dx++) {
                if (dx*dx + dy*dy <= 5) {
                    renderSetCanvasPixel(xc + dx, yc + dy, color);
                }
            }
        }
    }
}

void renderDrawEraserPoint(int xc, int yc) {
    uint16_t color = RGB15(31, 31, 31);
    int radius = eraser_size / 2;
    if (radius == 0) {
        renderSetCanvasPixel(xc, yc, color);
    } else {
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                if (dx*dx + dy*dy <= radius*radius) {
                    renderSetCanvasPixel(xc + dx, yc + dy, color);
                }
            }
        }
    }
}

void renderDrawLine(int x0, int y0, int x1, int y1, uint16_t color, int size, bool eraser) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        if (eraser) {
            renderDrawEraserPoint(x0, y0);
        } else {
            renderDrawBrushPoint(x0, y0, color, size);
        }

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

void renderInitCanvas(void) {
    for (int y = 0; y < 176; y++) {
        for (int x = 0; x < 256; x++) {
            canvas_buffer[y * 256 + x] = RGB15(31, 31, 31);
        }
    }
    uiDrawToolbar();
}

void renderInitPreview(void) {
    uint16_t border_color = RGB15(10, 10, 10);
    for (int i = 0; i < 128 * 128; i++) {
        preview_buffer[i] = border_color;
    }
}

void renderUpdatePreview(void) {
    for (int y = 0; y < 88; y++) {
        for (int x = 0; x < 128; x++) {
            uint16_t p = canvas_buffer[(y * 2) * 256 + (x * 2)];
            preview_buffer[(y + 20) * 128 + x] = p;
        }
    }
}
