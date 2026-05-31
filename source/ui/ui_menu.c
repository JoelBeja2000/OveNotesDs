#include "ui_menu.h"
#include "ui_shared.h"
#include "ui.h"
#include "ui_modal.h"
#include "render.h"
#include "logo_data.h"
#include <stdio.h>
#include <string.h>

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
    renderDrawText(uiTxt(TXT_WELCOME_TITLE), 32, 24, app_theme_color, 0);
    
    // Draw language/flag button at top-right
    // x = 226..246, y = 8..28
    drawRect(226, 8, 246, 28, RGB15(10, 10, 12));
    drawRectOutline(226, 8, 246, 28, app_theme_color);
    
    if (current_lang == 0) {
        // Spain Flag
        // Red top, yellow middle, red bottom
        for (int y = 9; y <= 27; y++) {
            uint16_t col;
            if (y >= 9 && y <= 12) col = RGB15(28, 2, 2);
            else if (y >= 13 && y <= 23) col = RGB15(31, 28, 0);
            else col = RGB15(28, 2, 2);
            
            for (int x = 227; x <= 245; x++) {
                renderSetPixel(x, y, col);
            }
        }
    } else if (current_lang == 2) {
        // French Flag
        // Blue vertical columns, White, Red
        for (int y = 9; y <= 27; y++) {
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
        // Blue canton at x=227..234, y=9..17
        for (int y = 9; y <= 27; y++) {
            for (int x = 227; x <= 245; x++) {
                if (x >= 227 && x <= 234 && y >= 9 && y <= 17) {
                    renderSetPixel(x, y, RGB15(2, 5, 20));
                } else {
                    // Alternating Red and White stripes
                    // 19 rows total (9 to 27). Let's make stripes 2px thick.
                    // (y - 9) / 2 is stripe index. Even = Red, Odd = White
                    int stripe_idx = (y - 9) / 2;
                    uint16_t col = (stripe_idx % 2 == 0) ? RGB15(28, 2, 2) : RGB15(31, 31, 31);
                    renderSetPixel(x, y, col);
                }
            }
        }
        // Draw some tiny stars in the canton
        renderSetPixel(229, 11, RGB15(31, 31, 31));
        renderSetPixel(232, 11, RGB15(31, 31, 31));
        renderSetPixel(230, 13, RGB15(31, 31, 31));
        renderSetPixel(229, 15, RGB15(31, 31, 31));
        renderSetPixel(232, 15, RGB15(31, 31, 31));
    }
    
    // Button 1: Crear Nueva Nota
    // x = 32..224, y = 42..64
    drawRect(32, 42, 224, 64, RGB15(10, 10, 12));
    drawRectOutline(32, 42, 224, 64, app_theme_color);
    char create_lbl[48];
    sprintf(create_lbl, "%s", uiTxt(TXT_CREATE_NOTE));
    int create_pad = (24 - strlen(create_lbl)) * 4;
    if (create_pad < 0) create_pad = 0;
    renderDrawText(create_lbl, 32 + create_pad, 49, RGB15(31, 31, 31), 0);
    
    // Button 2: Ver Notas Creadas
    // x = 32..224, y = 72..94
    drawRect(32, 72, 224, 94, RGB15(10, 10, 12));
    drawRectOutline(32, 72, 224, 94, app_theme_color);
    char view_lbl[48];
    sprintf(view_lbl, "%s", uiTxt(TXT_VIEW_NOTES));
    int view_pad = (24 - strlen(view_lbl)) * 4;
    if (view_pad < 0) view_pad = 0;
    renderDrawText(view_lbl, 32 + view_pad, 79, RGB15(31, 31, 31), 0);
    
    // Button 3: WiFi / Conexión
    // x = 32..224, y = 102..124
    drawRect(32, 102, 224, 124, RGB15(10, 10, 12));
    drawRectOutline(32, 102, 224, 124, app_theme_color);
    char wifi_lbl[48];
    sprintf(wifi_lbl, "%s", uiTxt(TXT_WIFI_CONNECTION));
    int wifi_pad = (24 - strlen(wifi_lbl)) * 4;
    if (wifi_pad < 0) wifi_pad = 0;
    renderDrawText(wifi_lbl, 32 + wifi_pad, 109, RGB15(31, 31, 31), 0);
    
    // Button 4: Cambiar Tema
    // x = 32..224, y = 132..154
    drawRect(32, 132, 224, 154, RGB15(10, 10, 12));
    drawRectOutline(32, 132, 224, 154, app_theme_color);
    char theme_lbl[48];
    sprintf(theme_lbl, uiTxt(TXT_THEME_LABEL), uiGetThemeName(active_theme_idx));
    int theme_pad = (24 - strlen(theme_lbl)) * 4;
    if (theme_pad < 0) theme_pad = 0;
    renderDrawText(theme_lbl, 32 + theme_pad, 139, RGB15(31, 31, 31), 0);
    
    // Bottom instructions
    renderDrawText(uiTxt(TXT_START_INSTRUCTIONS), 20, 168, RGB15(20, 20, 22), 0);
    
    uiDrawLogo();
    
    if (show_lang_modal) {
        uiDrawLanguageModal(&g_app_state);
    }
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
    renderDrawText(uiTxt(TXT_VIEW_NOTES), 8, 3, app_theme_color, 0);
    
    // Back button
    // x = 180..250, y = 1..13
    drawRect(180, 1, 250, 13, RGB15(24, 6, 6));
    drawRectOutline(180, 1, 250, 13, RGB15(31, 0, 0));
    renderDrawText(uiTxt(TXT_BACK_B), 186, 3, RGB15(31, 31, 31), 0);
    
    // If no notes are present
    if (total_count == 0) {
        renderDrawText(uiTxt(TXT_NO_NOTES_FOUND), 32, 80, RGB15(20, 20, 22), 0);
        renderDrawText(uiTxt(TXT_CREATE_NOTE_IN_START_MENU), 16, 96, RGB15(16, 16, 18), 0);
        
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
        renderDrawText(uiTxt(TXT_MORE_NOTES_ABOVE), 48, 18, app_theme_color, 0);
    }
    if (end_visible < total_count) {
        renderDrawText(uiTxt(TXT_MORE_NOTES_BELOW), 52, 156, app_theme_color, 0);
    }
    
    // Instructions at the very bottom
    renderDrawText(uiTxt(TXT_GALLERY_INSTRUCTIONS), 12, 172, RGB15(16, 16, 18), 0);
}
