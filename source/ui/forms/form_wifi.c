#define NO_UI_COMPAT_MACROS
#include "form_wifi.h"
#include "keyboard.h"
#include "ui.h"
#include "net.h"
#include "render.h"
#include "pointer_sheep_data.h"
#include "font8x8.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define app_theme_color g_app_state.ui.app_theme_color
#define active_theme_idx g_app_state.ui.active_theme_idx

bool ssid_manual_input = false;

// Sub-screen drawing helpers
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

static void drawSubGridBackground(void) {
    drawSubRect(0, 0, 255, 191, RGB15(4, 4, 5));
    uint16_t grid_color = RGB15(6, 6, 8);
    for (int y = 0; y < 192; y += 16) {
        for (int x = 0; x < 256; x += 16) {
            drawSubPixel(x, y, grid_color);
        }
    }
}

static void drawSubPointerSheep(int target_x, int target_y) {
    if (wizard_buffer == NULL) return;
    for (int y = 0; y < 48; y++) {
        for (int x = 0; x < 48; x++) {
            int px = target_x + x;
            int py = target_y + y;
            if (px >= 0 && px < 256 && py >= 0 && py < 192) {
                uint8_t alpha = pointer_sheep_data[y * 48 + x];
                if (alpha > 0) {
                    int alpha_32 = alpha >> 3;
                    uint16_t bg_pixel = wizard_buffer[py * 256 + px];
                    uint16_t blended = blendRGB555_int(app_theme_color, bg_pixel, alpha_32);
                    wizard_buffer[py * 256 + px] = blended;
                }
            }
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

    // 1. Header
    drawSubRect(0, 0, 255, 15, header_bg);
    drawSubLine(0, 16, 255, 16, border_col);
    drawSubText(uiTxt(TXT_CONN_SETTINGS), 8, 4, header_text_col, 0);

    // 2. Footer (Cancel / Save)
    drawSubRect(0, 176, 255, 191, header_bg);
    drawSubLine(0, 175, 255, 175, border_col);

    // [B] Cancel
    drawSubRectOutline(8, 180, 20, 188, border_col);
    drawSubText("B", 11, 181, header_text_col, 0);
    drawSubText(uiTxt(TXT_CANCEL), 24, 181, header_text_col, 0);

    // [A] Save
    drawSubRectOutline(204, 180, 216, 188, border_col);
    drawSubText("A", 207, 181, header_text_col, 0);
    drawSubText(uiTxt(TXT_SAVE), 220, 181, header_text_col, 0);

    // 3. Main Panel (centered, full width)
    drawSubRect(6, 22, 250, 110, panel_bg);
    drawSubRectOutline(6, 22, 250, 110, border_col);

    // Inner Header
    drawSubRect(7, 23, 249, 36, blendRGB555_int(app_theme_color, RGB15(4, 4, 5), 8));
    drawSubLine(6, 37, 250, 37, border_col);
    drawSubText(uiTxt(TXT_PAIRING_CONFIG), 10, 26, header_text_col, 0);

    // Field 0: Code (y = 44 to 62)
    drawSubText(uiTxt(TXT_CODE), 12, 50, text_col, 0);
    drawSubRectOutline(50, 44, 244, 62, (step == 0) ? active_col : RGB15(15, 15, 15));
    if (step == 0) {
        drawSubRectOutline(51, 45, 243, 61, active_col);
        drawSubText(input_text, 56, 49, text_col, 0);
    } else {
        drawSubText(pairing_code, 56, 49, text_col, 0);
    }

    // Field 1: WiFi (y = 70 to 88)
    drawSubText("WiFi", 12, 76, text_col, 0);
    drawSubRectOutline(50, 70, 244, 88, (step == 1) ? active_col : RGB15(15, 15, 15));
    if (step == 1) {
        drawSubRectOutline(51, 71, 243, 87, active_col);
        drawSubText(input_text, 56, 75, text_col, 0);
    } else {
        drawSubText(wifi_ssid, 56, 75, text_col, 0);
    }

    // Help text with pointer sheep pointing at it
    drawSubPointerSheep(6, 118);
    drawSubText(uiTxt(TXT_ENTER_CODE_1), 56, 124, text_col, 0);
    drawSubText(uiTxt(TXT_ENTER_CODE_2), 56, 136, inactive_text, 0);
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
    renderDrawRectOutline(10, 8, 246, 36, app_theme_color);
    
    char label[128];
    if (step == 0)      sprintf(label, "%s: %s_", uiTxt(TXT_CODIGO_CAPS), input_text);
    else if (step == 1) sprintf(label, "WIFI: %s_", input_text);
    renderDrawText(label, 16, 17, RGB15(31, 31, 31), 0);
    
    drawKey(10, 42, 120, 62, uiTxt(TXT_CODIGO_CAPS), (step == 0));
    drawKey(126, 42, 246, 62, "WIFI", (step == 1));
    
    if (step == 0) {
        drawKeyboard();
    } else {
        if (ssid_manual_input) {
            drawKeyboard();
        } else {
            // Draw 2-column WiFi profile selection menu
            for (int i = 0; i < 3; i++) {
                char label[32];
                bool high = false;
                if (net_wfc_ssids[i][0] != '\0') {
                    high = (strcmp(input_text, net_wfc_ssids[i]) == 0);
                    if (strlen(net_wfc_ssids[i]) > 13) {
                        sprintf(label, "%d: %.10s...", i + 1, net_wfc_ssids[i]);
                    } else {
                        sprintf(label, "%d: %s", i + 1, net_wfc_ssids[i]);
                    }
                } else {
                    sprintf(label, "%d: %s", i + 1, uiTxt(TXT_EMPTY_SLOT));
                }
                drawKey(10, 68 + i * 30, 124, 92 + i * 30, label, high);
            }
            
            for (int i = 3; i < 6; i++) {
                char label[32];
                bool high = false;
                if (net_wfc_ssids[i][0] != '\0') {
                    high = (strcmp(input_text, net_wfc_ssids[i]) == 0);
                    if (strlen(net_wfc_ssids[i]) > 13) {
                        sprintf(label, "%d: %.10s...", i + 1, net_wfc_ssids[i]);
                    } else {
                        sprintf(label, "%d: %s", i + 1, net_wfc_ssids[i]);
                    }
                } else {
                    sprintf(label, "%d: %s", i + 1, uiTxt(TXT_EMPTY_SLOT));
                }
                drawKey(132, 68 + (i - 3) * 30, 246, 92 + (i - 3) * 30, label, high);
            }
            
            drawKey(10, 158, 246, 182, uiTxt(TXT_MANUAL_KEYBOARD), false);
        }
    }
}
