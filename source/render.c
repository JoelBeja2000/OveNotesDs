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

void renderSetCanvasPixel(int x, int y, uint16_t color) {
    int max_y = toolbar_hidden ? 192 : 176;
    if (x >= 0 && x < 256 && y >= 0 && y < max_y) {
        canvas_buffer[y * 256 + x] = color;
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
    for (int dy = -5; dy <= 5; dy++) {
        for (int dx = -5; dx <= 5; dx++) {
            if (dx*dx + dy*dy <= 25) {
                renderSetCanvasPixel(xc + dx, yc + dy, color);
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
