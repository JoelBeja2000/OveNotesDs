#define NO_UI_COMPAT_MACROS
#include "sidebar.h"
#include "ui.h"
#include "render.h"
#include <stdio.h>
#include <string.h>

#define app_theme_color g_app_state.ui.app_theme_color
#define active_theme_idx g_app_state.ui.active_theme_idx
#define layers_count g_app_state.draw.layers_count
#define active_layer_idx g_app_state.draw.active_layer_idx
#define dragging_layer_idx g_app_state.ui.dragging_layer_idx
#define layer_names g_app_state.draw.layer_names
#define layers_visible g_app_state.draw.layers_visible
#define bg_modifiable g_app_state.draw.bg_modifiable
#define layers_opacity g_app_state.draw.layers_opacity
#define layers_panel_open g_app_state.ui.layers_panel_open

static void uiDrawLayersSidebar(void) {
    uint16_t panel_bg = RGB15(4, 4, 5);
    uint16_t border_col = app_theme_color;
    uint16_t text_col = RGB15(31, 31, 31);
    
    // Fill sidebar region
    for (int y = 0; y < 176; y++) {
        for (int x = 144; x < 256; x++) {
            canvas_buffer[y * 256 + x] = panel_bg;
        }
        canvas_buffer[y * 256 + 143] = border_col;
    }
    
    // Header title
    renderDrawText(uiTxt(TXT_SIDEBAR_CAPAS), 148, 2, text_col, 0);
    
    // Close button "X"
    renderDrawRect(236, 1, 252, 11, RGB15(6, 2, 2));
    renderDrawRectOutline(236, 1, 252, 11, border_col);
    renderDrawText("X", 241, 2, RGB15(31, 10, 10), 0);
    
    // Add layer button
    if (layers_count < MAX_LAYERS) {
        uint16_t plus_capa_bg = blendRGB555_int(app_theme_color, panel_bg, 6);
        renderDrawRect(148, 14, 252, 24, plus_capa_bg);
        renderDrawRectOutline(148, 14, 252, 24, border_col);
        renderDrawText(uiTxt(TXT_SIDEBAR_ADD_CAPA), 182, 16, text_col, 0);
    } else {
        renderDrawRect(148, 14, 252, 24, RGB15(2, 2, 3));
        renderDrawRectOutline(148, 14, 252, 24, RGB15(6, 6, 8));
        renderDrawText(uiTxt(TXT_SIDEBAR_LLENO), 188, 16, RGB15(10, 10, 10), 0);
    }
    
    // Layer items
    for (int i = layers_count - 1; i >= 0; i--) {
        int idx_from_top = layers_count - 1 - i;
        int y_pos = 27 + idx_from_top * 13;
        
        // Drag handle
        uint16_t circle_col = (dragging_layer_idx == i) ? app_theme_color : RGB15(8, 8, 10);
        for (int dy = -2; dy <= 2; dy++) {
            for (int dx = -2; dx <= 2; dx++) {
                if (dx*dx + dy*dy <= 5) {
                    int px = 151 + dx;
                    int py = y_pos + 5 + dy;
                    if (px >= 144 && px < 256 && py >= 0 && py < 192) {
                        canvas_buffer[py * 256 + px] = circle_col;
                    }
                }
            }
        }
        for (int dy = -3; dy <= 3; dy++) {
            for (int dx = -3; dx <= 3; dx++) {
                if (dx*dx + dy*dy > 5 && dx*dx + dy*dy <= 10) {
                    int px = 151 + dx;
                    int py = y_pos + 5 + dy;
                    if (px >= 144 && px < 256 && py >= 0 && py < 192) {
                        canvas_buffer[py * 256 + px] = RGB15(0, 0, 0);
                    }
                }
            }
        }
        
        // Selection button
        bool is_active = (active_layer_idx == i);
        uint16_t btn_bg = is_active ? blendRGB555_int(app_theme_color, panel_bg, 8) : RGB15(2, 2, 3);
        uint16_t btn_border = is_active ? app_theme_color : RGB15(6, 6, 8);
        uint16_t btn_txt = is_active ? (active_theme_idx == 0 ? RGB15(0, 0, 0) : RGB15(31, 31, 31)) : RGB15(20, 20, 22);
        
        renderDrawRect(158, y_pos, 212, y_pos + 11, btn_bg);
        renderDrawRectOutline(158, y_pos, 212, y_pos + 11, btn_border);
        
        char disp_name[7];
        if (strncmp(layer_names[i], "CAPA ", 5) == 0) {
            sprintf(disp_name, "%s%c", uiTxt(TXT_SIDEBAR_CAPA), layer_names[i][5]);
        } else {
            strncpy(disp_name, layer_names[i], 6);
            disp_name[6] = '\0';
        }
        renderDrawText(disp_name, 162, y_pos + 2, btn_txt, 0);
        
        // Visibility toggle
        bool is_visible = layers_visible[i];
        uint16_t vis_bg = is_visible ? blendRGB555_int(app_theme_color, panel_bg, 6) : RGB15(2, 2, 3);
        uint16_t vis_border = is_visible ? app_theme_color : RGB15(6, 6, 8);
        uint16_t vis_txt = is_visible ? (is_active && active_theme_idx == 0 ? RGB15(0, 0, 0) : RGB15(31, 31, 31)) : RGB15(10, 10, 10);
        
        renderDrawRect(216, y_pos, 232, y_pos + 11, vis_bg);
        renderDrawRectOutline(216, y_pos, 232, y_pos + 11, vis_border);
        renderDrawText(is_visible ? "V" : "H", 220, y_pos + 2, vis_txt, 0);
        
        // Delete button
        if (i > 0) {
            renderDrawRect(236, y_pos, 252, y_pos + 11, RGB15(12, 3, 3));
            renderDrawRectOutline(236, y_pos, 252, y_pos + 11, RGB15(6, 2, 2));
            renderDrawText("X", 241, y_pos + 2, RGB15(31, 10, 10), 0);
        } else {
            renderDrawRect(236, y_pos, 252, y_pos + 11, RGB15(2, 2, 3));
            renderDrawRectOutline(236, y_pos, 252, y_pos + 11, RGB15(6, 6, 8));
            renderDrawText("-", 241, y_pos + 2, RGB15(10, 10, 10), 0);
        }
    }
    
    // Draw "FONDO" item
    int bg_y = 27 + layers_count * 13;
    renderDrawRect(148, bg_y, 212, bg_y + 11, RGB15(2, 2, 3));
    renderDrawRectOutline(148, bg_y, 212, bg_y + 11, RGB15(6, 6, 8));
    renderDrawText(uiTxt(TXT_SIDEBAR_FONDO), 158, bg_y + 2, text_col, 0);
    
    // Lock toggle
    renderDrawRect(216, bg_y, 252, bg_y + 11, bg_modifiable ? blendRGB555_int(app_theme_color, panel_bg, 6) : RGB15(12, 3, 3));
    renderDrawRectOutline(216, bg_y, 252, bg_y + 11, bg_modifiable ? border_col : RGB15(6, 2, 2));
    renderDrawText(bg_modifiable ? uiTxt(TXT_SIDEBAR_MOD) : uiTxt(TXT_SIDEBAR_LOK), 218, bg_y + 2, bg_modifiable ? text_col : RGB15(31, 10, 10), 0);
    renderDrawLockIcon(244, bg_y + 1, !bg_modifiable);
    
    // Draw Opacity Slider
    renderDrawText("OPAC:", 148, 145, text_col, 0);
    renderDrawRect(178, 148, 222, 149, RGB15(6, 6, 8));
    int thumb_x = 178 + (layers_opacity[active_layer_idx] * 44) / 100;
    renderDrawRect(thumb_x - 2, 144, thumb_x + 2, 152, app_theme_color);
    char op_str[16];
    sprintf(op_str, "%d%%", layers_opacity[active_layer_idx]);
    renderDrawText(op_str, 226, 145, text_col, 0);
    
    // Draw merge buttons
    bool can_merge_down = (active_layer_idx > 0);
    uint16_t mdown_bg = can_merge_down ? blendRGB555_int(app_theme_color, panel_bg, 8) : RGB15(2, 2, 3);
    uint16_t mdown_border = can_merge_down ? app_theme_color : RGB15(6, 6, 8);
    uint16_t mdown_txt = can_merge_down ? (active_theme_idx == 0 ? RGB15(0, 0, 0) : RGB15(31, 31, 31)) : RGB15(10, 10, 10);
    
    renderDrawRect(148, 157, 198, 169, mdown_bg);
    renderDrawRectOutline(148, 157, 198, 169, mdown_border);
    renderDrawText(uiTxt(TXT_SIDEBAR_C_ABJ), 154, 160, mdown_txt, 0);
    
    bool can_merge_up = (active_layer_idx < layers_count - 1);
    uint16_t mup_bg = can_merge_up ? blendRGB555_int(app_theme_color, panel_bg, 8) : RGB15(2, 2, 3);
    uint16_t mup_border = can_merge_up ? app_theme_color : RGB15(6, 6, 8);
    uint16_t mup_txt = can_merge_up ? (active_theme_idx == 0 ? RGB15(0, 0, 0) : RGB15(31, 31, 31)) : RGB15(10, 10, 10);
    
    renderDrawRect(202, 157, 252, 169, mup_bg);
    renderDrawRectOutline(202, 157, 252, 169, mup_border);
    renderDrawText(uiTxt(TXT_SIDEBAR_C_ARB), 208, 160, mup_txt, 0);
}

void uiDrawLayersOverlay(void) {
    if (g_app_state.ui.toolbar_hidden) return;
    if (layers_panel_open) {
        uiDrawLayersSidebar();
    } else {
        // Draw a small floating tab button on the right edge
        uint16_t tab_bg = RGB15(2, 2, 3);
        uint16_t tab_border = app_theme_color;
        
        renderDrawRect(244, 70, 255, 106, tab_bg);
        renderDrawRectOutline(244, 70, 255, 106, tab_border);
        
        // Stack of three sheets
        uint16_t sheet_col = blendRGB555_int(app_theme_color, RGB15(31, 31, 31), 16);
        renderDrawRect(247, 76, 252, 78, sheet_col);
        renderDrawRect(247, 82, 252, 84, sheet_col);
        renderDrawRect(247, 88, 252, 90, sheet_col);
        
        renderDrawText("<", 247, 96, RGB15(31, 31, 31), 0);
    }
}
