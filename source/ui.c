#include "ui.h"
#include "render.h"
#include "net.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

PrintConsole subConsole;
PrintConsole bottom_form_console;

int active_brush_size = 1;
bool is_eraser = false;

static void fillButtonBg(int x_start, int x_end, uint16_t color) {
    for (int y = 177; y < 191; y++) {
        for (int x = x_start + 1; x < x_end; x++) {
            canvas_buffer[y * 256 + x] = color;
        }
    }
}

static void drawBrushIcon(int xc, int yc, int size) {
    uint16_t color = RGB15(28, 28, 30);
    int radius = size / 2;
    if (radius == 0) {
        renderSetPixel(xc, yc, color);
    } else {
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                if (dx*dx + dy*dy <= radius*radius) {
                    renderSetPixel(xc + dx, yc + dy, color);
                }
            }
        }
    }
}

void uiDrawToolbar(void) {
    // Fill toolbar background (dark blue-gray)
    for (int y = 176; y < 192; y++) {
        for (int x = 0; x < 256; x++) {
            canvas_buffer[y * 256 + x] = RGB15(5, 5, 7);
        }
    }

    // Draw vertical separators
    for (int y = 176; y < 192; y++) {
        renderSetPixel(30, y, RGB15(15, 15, 17));
        renderSetPixel(60, y, RGB15(15, 15, 17));
        renderSetPixel(90, y, RGB15(15, 15, 17));
        renderSetPixel(130, y, RGB15(15, 15, 17));
        renderSetPixel(185, y, RGB15(15, 15, 17));
    }

    // Highlight selected tool button background
    if (!is_eraser && active_brush_size == 1) fillButtonBg(0, 30, RGB15(12, 12, 18));
    if (!is_eraser && active_brush_size == 3) fillButtonBg(30, 60, RGB15(12, 12, 18));
    if (!is_eraser && active_brush_size == 5) fillButtonBg(60, 90, RGB15(12, 12, 18));
    if (is_eraser) fillButtonBg(90, 130, RGB15(12, 12, 18));

    // Blue background for Config
    fillButtonBg(130, 185, RGB15(6, 6, 12));

    // Green highlight for the Publish button
    fillButtonBg(185, 255, RGB15(2, 16, 2));

    // Draw brush indicator circles
    drawBrushIcon(15, 184, 1);
    drawBrushIcon(45, 184, 3);
    drawBrushIcon(75, 184, 5);

    // Draw labels (Spanish)
    uint16_t text_color = RGB15(22, 22, 24);
    uint16_t active_text_color = RGB15(31, 31, 31);
    renderDrawText("BORR", 94, 180, is_eraser ? active_text_color : text_color, 0);
    renderDrawText("CONFIG", 133, 180, active_text_color, 0);
    renderDrawText("PUBLICAR", 188, 180, active_text_color, 0);
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
    if (c < 32 || c > 127) return;
    const unsigned char *glyph = font8x8_basic[(unsigned char)c];
    for (int dy = 0; dy < 8; dy++) {
        unsigned char row = glyph[dy];
        for (int dx = 0; dx < 8; dx++) {
            if (row & (1 << dx)) {
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
        x += 8;
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
    drawSubRect(0, 0, 255, 191, RGB15(31, 31, 31));
    uint16_t grid_color = RGB15(29, 29, 29);
    for (int y = 0; y < 192; y += 8) {
        for (int x = 0; x < 256; x++) {
            drawSubPixel(x, y, grid_color);
        }
    }
    for (int x = 0; x < 256; x += 8) {
        for (int y = 0; y < 192; y += 8) {
            drawSubPixel(x, y, grid_color);
        }
    }
}

void uiDrawFormUI(int step, const char* input_text) {
    if (wizard_buffer == NULL) return;

    drawSubGridBackground();

    uint16_t black = RGB15(0, 0, 0);
    uint16_t white = RGB15(31, 31, 31);
    uint16_t light_grey = RGB15(26, 26, 26);
    uint16_t dark_grey = RGB15(12, 12, 12);
    uint16_t blue = RGB15(0, 16, 30);

    // 1. Header (Connection Settings)
    drawSubRect(0, 0, 255, 15, light_grey);
    drawSubLine(0, 16, 255, 16, black);
    drawSubText("Connection Settings", 8, 4, black, 0);

    // 2. Footer (Cancel / Save)
    drawSubRect(0, 176, 255, 191, light_grey);
    drawSubLine(0, 175, 255, 175, black);

    // [B] Cancel
    drawSubRectOutline(8, 180, 20, 188, black);
    drawSubText("B", 11, 181, black, 0);
    drawSubText("Cancel", 24, 181, black, 0);

    // [A] Save
    drawSubRectOutline(204, 180, 216, 188, black);
    drawSubText("A", 207, 181, black, 0);
    drawSubText("Save", 220, 181, black, 0);

    // 3. Left Panel (Connection 1 / Connection 2)
    drawSubRect(6, 22, 86, 170, white);
    drawSubRectOutline(6, 22, 86, 170, black);

    drawSubText("Connection 1", 10, 26, black, 0);

    // Connection 1 Box (HTTP selected)
    drawSubRectOutline(8, 36, 84, 52, blue);
    drawSubRectOutline(9, 37, 83, 51, blue);
    drawBlueArrow(12, 41);
    drawSubText("HTTP", 20, 40, blue, 0);

    // Dotted separator
    for (int x = 8; x < 84; x += 2) {
        drawSubPixel(x, 60, dark_grey);
    }

    drawSubText("Connection 2", 10, 66, black, 0);
    drawSubText("Not Configured", 10, 76, dark_grey, 0);

    drawSubLine(6, 120, 86, 120, black);

    drawSubText("Create or modify", 10, 126, black, 0);
    drawSubText("an HTTP server.", 10, 136, black, 0);

    // 4. Right Panel (HTTP Configuration Fields)
    drawSubRect(92, 22, 250, 170, white);
    drawSubRectOutline(92, 22, 250, 170, black);

    // Inner Header (HTTP Configuration)
    drawSubRect(93, 23, 249, 36, light_grey);
    drawSubLine(92, 37, 250, 37, black);
    drawSubText("HTTP Configuration", 96, 26, black, 0);

    // Field 0: IP Address
    drawSubText("IP Address", 96, 48, black, 0);
    drawSubRectOutline(152, 44, 244, 58, (step == 0) ? blue : black);
    if (step == 0) {
        drawSubRectOutline(153, 45, 243, 57, blue);
        drawSubText(input_text, 156, 47, black, 0);
    } else {
        drawSubText(http_ip, 156, 47, black, 0);
    }

    // Field 1: Port
    drawSubText("Port", 96, 72, black, 0);
    drawSubRectOutline(152, 68, 206, 82, (step == 1) ? blue : black);
    if (step == 1) {
        drawSubRectOutline(153, 69, 205, 81, blue);
        drawSubText(input_text, 156, 71, black, 0);
    } else {
        drawSubText(http_port_str, 156, 71, black, 0);
    }

    // Field 2: WiFi SSID
    drawSubText("WiFi SSID", 96, 96, black, 0);
    drawSubRectOutline(152, 92, 244, 106, (step == 2) ? blue : black);
    if (step == 2) {
        drawSubRectOutline(153, 93, 243, 105, blue);
        drawSubText(input_text, 156, 95, black, 0);
    } else {
        drawSubText(wifi_ssid, 156, 95, black, 0);
    }
}

void uiDrawBottomButtons(int active_step) {
    consoleSelect(&bottom_form_console);

    printf("\x1b[1;1H                                \n");
    printf("                                \n");
    printf("                                \n");
    printf("                                \n");
    printf("                                \n");

    printf("\x1b[2;1H");
    printf("  +----+   +----+   +-----+\n");

    printf("  |");
    if (active_step == 0) printf("\x1b[7m IP \x1b[0m");
    else printf(" IP ");
    printf("|   |");

    if (active_step == 1) printf("\x1b[7mPORT\x1b[0m");
    else printf("PORT");
    printf("|   |");

    if (active_step == 2) printf("\x1b[7mSSID\x1b[0m");
    else printf("SSID");
    printf("|  \n");

    printf("  +----+   +----+   +-----+\n");
}
