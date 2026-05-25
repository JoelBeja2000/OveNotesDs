#include "ui.h"
#include "render.h"
#include "net.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

PrintConsole subConsole;

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

    // Field 0: IP
    drawSubText("IP", 96, 48, black, 0);
    drawSubRectOutline(136, 44, 244, 58, (step == 0) ? blue : black);
    if (step == 0) {
        drawSubRectOutline(137, 45, 243, 57, blue);
        drawSubText(input_text, 140, 47, black, 0);
    } else {
        drawSubText(http_ip, 140, 47, black, 0);
    }

    // Field 1: Port
    drawSubText("Port", 96, 72, black, 0);
    drawSubRectOutline(136, 68, 190, 82, (step == 1) ? blue : black);
    if (step == 1) {
        drawSubRectOutline(137, 69, 189, 81, blue);
        drawSubText(input_text, 140, 71, black, 0);
    } else {
        drawSubText(http_port_str, 140, 71, black, 0);
    }

    // Field 2: SSID
    drawSubText("SSID", 96, 96, black, 0);
    drawSubRectOutline(136, 92, 244, 106, (step == 2) ? blue : black);
    if (step == 2) {
        drawSubRectOutline(137, 93, 243, 105, blue);
        drawSubText(input_text, 140, 95, black, 0);
    } else {
        drawSubText(wifi_ssid, 140, 95, black, 0);
    }
}

static bool shift_active = false;
static bool caps_active = false;

static void drawKey(int x0, int y0, int x1, int y1, const char* label, bool highlighted) {
    uint16_t bg_color = highlighted ? RGB15(15, 20, 31) : RGB15(31, 31, 31);
    uint16_t border_color = RGB15(0, 0, 0);
    uint16_t text_color = highlighted ? RGB15(31, 31, 31) : RGB15(0, 0, 0);
    
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
    int text_w = len * 8;
    int text_h = 8;
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
    uint16_t cream = RGB15(28, 29, 28);
    for (int y = 0; y < 192; y++) {
        for (int x = 0; x < 256; x++) {
            canvas_buffer[y * 256 + x] = cream;
        }
    }
    
    uint16_t grid_color = RGB15(26, 27, 26);
    for (int y = 0; y < 192; y += 8) {
        for (int x = 0; x < 256; x += 4) {
            renderSetPixel(x, y, grid_color);
        }
    }
    for (int x = 0; x < 256; x += 8) {
        for (int y = 0; y < 192; y += 4) {
            renderSetPixel(x, y, grid_color);
        }
    }
    
    for (int y = 8; y <= 34; y++) {
        for (int x = 10; x <= 246; x++) {
            canvas_buffer[y * 256 + x] = RGB15(31, 31, 31);
        }
    }
    for (int x = 10; x <= 246; x++) {
        canvas_buffer[8 * 256 + x] = RGB15(0, 0, 0);
        canvas_buffer[34 * 256 + x] = RGB15(0, 0, 0);
    }
    for (int y = 8; y <= 34; y++) {
        canvas_buffer[y * 256 + 10] = RGB15(0, 0, 0);
        canvas_buffer[y * 256 + 246] = RGB15(0, 0, 0);
    }
    
    char label[128];
    if (step == 0)      sprintf(label, "IP: %s_", input_text);
    else if (step == 1) sprintf(label, "PORT: %s_", input_text);
    else if (step == 2) sprintf(label, "SSID: %s_", input_text);
    renderDrawText(label, 16, 17, RGB15(0, 0, 0), 0);
    
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
