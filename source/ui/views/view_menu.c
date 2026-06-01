#define NO_UI_COMPAT_MACROS
#include "view_menu.h"
#include "ui.h"
#include "render.h"
#include "logo_data.h"
#include "modals.h"
#include <stdio.h>
#include <string.h>

#define app_theme_color g_app_state.ui.app_theme_color
#define active_theme_idx g_app_state.ui.active_theme_idx
#define current_lang g_app_state.ui.current_lang
#define show_lang_modal g_app_state.ui.show_lang_modal

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

const char* uiGetThemeName(int idx) {
    if (current_lang == 2) {
        switch(idx) {
            case 0: return "BANANE (JAUNE)";
            case 1: return "MENTHE (VERT)";
            case 2: return "CIEL (BLEU)";
            case 3: return "FRAISE (ROUGE)";
            case 4: return "RAISIN (VIOLET)";
            default: return "INCONNU";
        }
    } else if (current_lang == 1) {
        switch(idx) {
            case 0: return "BANANA (YELLOW)";
            case 1: return "MINT (GREEN)";
            case 2: return "SKY (BLUE)";
            case 3: return "STRAWBERRY (RED)";
            case 4: return "GRAPE (PURPLE)";
            default: return "UNKNOWN";
        }
    } else {
        switch(idx) {
            case 0: return "BANANA (AMARILLO)";
            case 1: return "MENTA (VERDE)";
            case 2: return "CIELO (AZUL)";
            case 3: return "FRESA (ROJO)";
            case 4: return "UVA (MORADO)";
            default: return "DESCONOCIDO";
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
    renderDrawText(uiTxt(TXT_WELCOME_TITLE), 32, 12, app_theme_color, 0);
    
    // Draw language/flag button at top-right
    // x = 226..246, y = 4..24
    renderDrawRect(226, 4, 246, 24, RGB15(10, 10, 12));
    renderDrawRectOutline(226, 4, 246, 24, app_theme_color);
    
    if (current_lang == 0) {
        // Spain Flag
        // Red top, yellow middle, red bottom
        for (int y = 5; y <= 23; y++) {
            uint16_t col;
            if (y >= 5 && y <= 8) col = RGB15(28, 2, 2);
            else if (y >= 9 && y <= 19) col = RGB15(31, 28, 0);
            else col = RGB15(28, 2, 2);
            
            for (int x = 227; x <= 245; x++) {
                renderSetPixel(x, y, col);
            }
        }
    } else if (current_lang == 2) {
        // French Flag
        // Blue vertical columns, White, Red
        for (int y = 5; y <= 23; y++) {
            for (int x = 227; x <= 245; x++) {
                uint16_t col;
                if (x >= 227 && x <= 232) col = RGB15(2, 5, 20); // Blue
                else if (x >= 233 && x <= 239) col = RGB15(31, 31, 31); // White
                else col = RGB15(28, 2, 2); // Red
                renderSetPixel(x, y, col);
            }
        }
    } else {
        // USA Flag
        // Blue canton at x=227..234, y=5..13
        for (int y = 5; y <= 23; y++) {
            for (int x = 227; x <= 245; x++) {
                if (x >= 227 && x <= 234 && y >= 5 && y <= 13) {
                    renderSetPixel(x, y, RGB15(2, 5, 20));
                } else {
                    int stripe_idx = (y - 5) / 2;
                    uint16_t col = (stripe_idx % 2 == 0) ? RGB15(28, 2, 2) : RGB15(31, 31, 31);
                    renderSetPixel(x, y, col);
                }
            }
        }
        // Draw some tiny stars in the canton
        renderSetPixel(229, 7, RGB15(31, 31, 31));
        renderSetPixel(232, 7, RGB15(31, 31, 31));
        renderSetPixel(230, 9, RGB15(31, 31, 31));
        renderSetPixel(229, 11, RGB15(31, 31, 31));
        renderSetPixel(232, 11, RGB15(31, 31, 31));
    }
    
    // Button 1: Crear Nueva Nota
    // x = 32..224, y = 34..52
    renderDrawRect(32, 34, 224, 52, RGB15(10, 10, 12));
    renderDrawRectOutline(32, 34, 224, 52, app_theme_color);
    char create_lbl[48];
    sprintf(create_lbl, "%s", uiTxt(TXT_CREATE_NOTE));
    int create_pad = (24 - strlen(create_lbl)) * 4;
    if (create_pad < 0) create_pad = 0;
    renderDrawText(create_lbl, 32 + create_pad, 39, RGB15(31, 31, 31), 0);
    
    // Button 2: Ver Notas Creadas
    // x = 32..224, y = 60..78
    renderDrawRect(32, 60, 224, 78, RGB15(10, 10, 12));
    renderDrawRectOutline(32, 60, 224, 78, app_theme_color);
    char view_lbl[48];
    sprintf(view_lbl, "%s", uiTxt(TXT_VIEW_NOTES));
    int view_pad = (24 - strlen(view_lbl)) * 4;
    if (view_pad < 0) view_pad = 0;
    renderDrawText(view_lbl, 32 + view_pad, 65, RGB15(31, 31, 31), 0);
    
    // Button 3: WiFi / Conexión
    // x = 32..224, y = 86..104
    renderDrawRect(32, 86, 224, 104, RGB15(10, 10, 12));
    renderDrawRectOutline(32, 86, 224, 104, app_theme_color);
    char wifi_lbl[48];
    sprintf(wifi_lbl, "%s", uiTxt(TXT_WIFI_CONNECTION));
    int wifi_pad = (24 - strlen(wifi_lbl)) * 4;
    if (wifi_pad < 0) wifi_pad = 0;
    renderDrawText(wifi_lbl, 32 + wifi_pad, 91, RGB15(31, 31, 31), 0);
    
    // Button 4: Cambiar Tema
    // x = 32..224, y = 112..130
    renderDrawRect(32, 112, 224, 130, RGB15(10, 10, 12));
    renderDrawRectOutline(32, 112, 224, 130, app_theme_color);
    char theme_lbl[48];
    sprintf(theme_lbl, uiTxt(TXT_THEME_LABEL), uiGetThemeName(active_theme_idx));
    int theme_pad = (24 - strlen(theme_lbl)) * 4;
    if (theme_pad < 0) theme_pad = 0;
    renderDrawText(theme_lbl, 32 + theme_pad, 117, RGB15(31, 31, 31), 0);
    
    // Button 5: Guía de Controles (Help)
    // x = 32..224, y = 138..156
    renderDrawRect(32, 138, 224, 156, RGB15(10, 10, 12));
    renderDrawRectOutline(32, 138, 224, 156, app_theme_color);
    char help_lbl[48];
    sprintf(help_lbl, "%s", uiTxt(TXT_HELP_BUTTON));
    int help_pad = (24 - strlen(help_lbl)) * 4;
    if (help_pad < 0) help_pad = 0;
    renderDrawText(help_lbl, 32 + help_pad, 143, RGB15(31, 31, 31), 0);
    
    // Bottom instructions
    renderDrawText(uiTxt(TXT_START_INSTRUCTIONS), 20, 168, RGB15(20, 20, 22), 0);
    
    uiDrawLogo();
    
    if (show_lang_modal) {
        uiDrawLanguageModal(&g_app_state);
    }
    
    if (g_app_state.ui.show_help_modal) {
        uiDrawHelpModal(&g_app_state);
    }
}

void view_menu_show(void) {
    uiDrawStartMenu();
}
