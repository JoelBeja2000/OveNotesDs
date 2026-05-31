#define NO_UI_COMPAT_MACROS
#include "view_gallery.h"
#include "ui.h"
#include "render.h"
#include <stdio.h>
#include <string.h>

#define app_theme_color g_app_state.ui.app_theme_color

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
    renderDrawRect(0, 0, 255, 14, RGB15(12, 12, 12));
    renderDrawText(uiTxt(TXT_VIEW_NOTES), 8, 3, app_theme_color, 0);
    
    // Back button
    // x = 180..250, y = 1..13
    renderDrawRect(180, 1, 250, 13, RGB15(24, 6, 6));
    renderDrawRectOutline(180, 1, 250, 13, RGB15(31, 0, 0));
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
        renderDrawRect(10, row_y, 246, row_y + 20, row_bg);
        renderDrawRectOutline(10, row_y, 246, row_y + 20, border_col);
        
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

void view_gallery_show(int selected_idx, int total_count, const char filenames[][32]) {
    uiDrawNotesGallery(selected_idx, total_count, filenames);
}
