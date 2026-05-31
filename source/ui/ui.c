#define NO_UI_COMPAT_MACROS
#include "ui.h"
#include "render.h"
#include "net.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>


AppState g_app_state = {
    .draw = {
        .active_brush_size = 1,
        .eraser_size = 4,
        .active_color_idx = 0,
        .drawing_mode = 0,
        .is_bucket = false,
        .is_eraser = false,
        .palette_colors = {
            RGB15(0, 0, 0),       // Black
            RGB15(0, 0, 28),      // Blue
            RGB15(28, 0, 0),      // Red
            RGB15(0, 20, 0),      // Green
            RGB15(30, 10, 20)     // Pink
        },
        .bg_modifiable = false,
        .bg_pattern_idx = 0,
        .bg_angle = 0,
        .bg_color_p_idx = 0,
        .bg_color_s_idx = 0,
        .angle_target = 0,
        .nib_angle = 0,
        .perspective_mode = 0,
        .perspective_points = { {32, 88}, {224, 88}, {128, 40}, {128, 136} },
        .perspective_step = 32,
        
        .layers_count = 1,
        .active_layer_idx = 0,
        .layers_visible = {true, false, false, false, false, false, false, false},
        .layer_names = {
            "Capa 0", "Capa 1", "Capa 2", "Capa 3", "Capa 4", "Capa 5", "Capa 6", "Capa 7"
        },
        .layers_opacity = {100, 100, 100, 100, 100, 100, 100, 100}
    },
    .ui = {
        .open_modal = -1,
        .preset_page = 0,
        .custom_page = 0,
        .selected_custom_slot = 0,
        .color_modal_tab = 0,
        .bg_modal_tab = 0,
        .picker_x = 128,
        .picker_y = 84,
        .picker_h = 0,
        .picker_s = 31,
        .picker_v = 31,
        .app_theme_color = RGB15(31, 31, 0),
        .active_theme_idx = 0,
        .current_lang = 0,
        .show_lang_modal = false,
        .layers_panel_open = false,
        .dragging_layer_idx = -1,
        .toolbar_hidden = false
    }
};

#undef NO_UI_COMPAT_MACROS
#include "ui_compat.h"

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

const char* uiTxt(const char* es, const char* en, const char* fr) {
    if (current_lang == 2) return fr;
    if (current_lang == 1) return en;
    return es;
}

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

uint16_t modal_backup[256 * 156];
static void fillButtonBg(int x_start, int x_end, uint16_t color) {
    for (int y = 177; y < 191; y++) {
        for (int x = x_start + 1; x < x_end; x++) {
            canvas_buffer[y * 256 + x] = color;
        }
    }
}

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



void uiUpdateColorPickerSelection(void) {
    if (open_modal != 2) return;
    
    // 1. Redraw the top 5 color swatches
    for (int i = 0; i < 5; i++) {
        drawModalColorButtonAt(16 + i * 46, 16 + i * 46 + 40, 36, 48, palette_colors[i], (active_color_idx == i));
    }
    
    // 2. Redraw the 2D Hue-Saturation Map
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

    // 3. Draw the vertical Preview Swatch
    uint16_t preview_color = palette_colors[active_color_idx];
    drawRect(153, 57, 175, 115, preview_color);

    // 4. Draw the vertical Value (brightness) slider
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
        uiDrawModalToolbox(&g_app_state, y0, y1);
    } 
    else if (open_modal == 1) {
        uiDrawModalBrushSize(&g_app_state, y0, y1);
    } 
    else if (open_modal == 2) {
        uiDrawModalColorPicker(&g_app_state, y0, y1);
    }
    else if (open_modal == 3) {
        uiDrawModalBackgroundSettings(&g_app_state, y0, y1);
    } 
    else if (open_modal == 4) {
        uiDrawModalAngleWheel(&g_app_state, y0, y1);
    }
    else if (open_modal == 5) {
        uiDrawModalNoteMenu(&g_app_state, y0, y1);
    }
    else if (open_modal == 6) {
        uiDrawModalSaveConfirm(&g_app_state, y0, y1);
    }
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
    // Fill toolbar background
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

    // Highlight active modal buttons
    uint16_t active_bg = blendRGB555_int(app_theme_color, RGB15(4, 4, 5), 8);
    if (open_modal == 0) fillButtonBg(0, 42, active_bg);
    if (open_modal == 1) fillButtonBg(42, 84, active_bg);
    if (open_modal == 2) fillButtonBg(84, 126, active_bg);
    if (open_modal == 3) fillButtonBg(126, 168, active_bg);

    // Highlight Menu (Button 4)
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
    const char* tool_label = uiTxt("PINC", "PEN", "PINC");
    if (is_eraser) tool_label = uiTxt("BORR", "ERAS", "GOMM");
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
    renderDrawText(uiTxt("ENVIAR", "SEND", "ENVOI"), 216, 180, RGB15(20, 31, 20), 0);
}

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
    renderDrawText(uiTxt("CAPAS", "LAYERS", "CALQS"), 148, 2, text_col, 0);
    
    // Close button "X"
    drawRect(236, 1, 252, 11, RGB15(6, 2, 2));
    drawRectOutline(236, 1, 252, 11, border_col);
    renderDrawText("X", 241, 2, RGB15(31, 10, 10), 0);
    
    // Add layer button
    if (layers_count < MAX_LAYERS) {
        uint16_t plus_capa_bg = blendRGB555_int(app_theme_color, panel_bg, 6);
        drawRect(148, 14, 252, 24, plus_capa_bg);
        drawRectOutline(148, 14, 252, 24, border_col);
        renderDrawText(uiTxt("+ CAPA", "+ LAYER", "+ CALQ"), 182, 16, text_col, 0);
    } else {
        drawRect(148, 14, 252, 24, RGB15(2, 2, 3));
        drawRectOutline(148, 14, 252, 24, RGB15(6, 6, 8));
        renderDrawText(uiTxt("LLENO", "FULL", "PLEIN"), 188, 16, RGB15(10, 10, 10), 0);
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
        
        drawRect(158, y_pos, 212, y_pos + 11, btn_bg);
        drawRectOutline(158, y_pos, 212, y_pos + 11, btn_border);
        
        char disp_name[7];
        if (strncmp(layer_names[i], "CAPA ", 5) == 0) {
            sprintf(disp_name, "%s%c", uiTxt("CAPA", "LAY", "CLQ"), layer_names[i][5]);
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
        
        drawRect(216, y_pos, 232, y_pos + 11, vis_bg);
        drawRectOutline(216, y_pos, 232, y_pos + 11, vis_border);
        renderDrawText(is_visible ? "V" : "H", 220, y_pos + 2, vis_txt, 0);
        
        // Delete button
        if (i > 0) {
            drawRect(236, y_pos, 252, y_pos + 11, RGB15(12, 3, 3));
            drawRectOutline(236, y_pos, 252, y_pos + 11, RGB15(6, 2, 2));
            renderDrawText("X", 241, y_pos + 2, RGB15(31, 10, 10), 0);
        } else {
            drawRect(236, y_pos, 252, y_pos + 11, RGB15(2, 2, 3));
            drawRectOutline(236, y_pos, 252, y_pos + 11, RGB15(6, 6, 8));
            renderDrawText("-", 241, y_pos + 2, RGB15(10, 10, 10), 0);
        }
    }
    
    // Draw "FONDO" item
    int bg_y = 27 + layers_count * 13;
    drawRect(148, bg_y, 212, bg_y + 11, RGB15(2, 2, 3));
    drawRectOutline(148, bg_y, 212, bg_y + 11, RGB15(6, 6, 8));
    renderDrawText(uiTxt("FONDO", "BG", "FOND"), 158, bg_y + 2, text_col, 0);
    
    // Lock toggle
    drawRect(216, bg_y, 252, bg_y + 11, bg_modifiable ? blendRGB555_int(app_theme_color, panel_bg, 6) : RGB15(12, 3, 3));
    drawRectOutline(216, bg_y, 252, bg_y + 11, bg_modifiable ? border_col : RGB15(6, 2, 2));
    renderDrawText(bg_modifiable ? uiTxt(" MOD", " EDIT", " MOD") : uiTxt(" LOK", " LOCK", " CAD"), 218, bg_y + 2, bg_modifiable ? text_col : RGB15(31, 10, 10), 0);
    drawLockIcon(244, bg_y + 1, !bg_modifiable);
    
    // Draw Opacity Slider
    renderDrawText("OPAC:", 148, 145, text_col, 0);
    drawRect(178, 148, 222, 149, RGB15(6, 6, 8));
    int thumb_x = 178 + (layers_opacity[active_layer_idx] * 44) / 100;
    drawRect(thumb_x - 2, 144, thumb_x + 2, 152, app_theme_color);
    char op_str[16];
    sprintf(op_str, "%d%%", layers_opacity[active_layer_idx]);
    renderDrawText(op_str, 226, 145, text_col, 0);
    
    // Draw merge buttons
    bool can_merge_down = (active_layer_idx > 0);
    uint16_t mdown_bg = can_merge_down ? blendRGB555_int(app_theme_color, panel_bg, 8) : RGB15(2, 2, 3);
    uint16_t mdown_border = can_merge_down ? app_theme_color : RGB15(6, 6, 8);
    uint16_t mdown_txt = can_merge_down ? (active_theme_idx == 0 ? RGB15(0, 0, 0) : RGB15(31, 31, 31)) : RGB15(10, 10, 10);
    
    drawRect(148, 157, 198, 169, mdown_bg);
    drawRectOutline(148, 157, 198, 169, mdown_border);
    renderDrawText(uiTxt("C. ABJ", "MRG DN", "F. BAS"), 154, 160, mdown_txt, 0);
    
    bool can_merge_up = (active_layer_idx < layers_count - 1);
    uint16_t mup_bg = can_merge_up ? blendRGB555_int(app_theme_color, panel_bg, 8) : RGB15(2, 2, 3);
    uint16_t mup_border = can_merge_up ? app_theme_color : RGB15(6, 6, 8);
    uint16_t mup_txt = can_merge_up ? (active_theme_idx == 0 ? RGB15(0, 0, 0) : RGB15(31, 31, 31)) : RGB15(10, 10, 10);
    
    drawRect(202, 157, 252, 169, mup_bg);
    drawRectOutline(202, 157, 252, 169, mup_border);
    renderDrawText(uiTxt("C. ARB", "MRG UP", "F. HAUT"), 208, 160, mup_txt, 0);
}

void uiDrawLayersOverlay(void) {
    if (layers_panel_open) {
        uiDrawLayersSidebar();
    } else {
        // Draw a small floating tab button on the right edge
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

void uiDrawTopConsoleBox(const char* title) {
    if (preview_buffer == NULL) return;
    
    renderUpdatePreview();
    
    uint16_t border_color = app_theme_color;
    uint16_t bg_color = RGB15(3, 3, 4);
    
    for (int y = 44; y <= 147; y++) {
        for (int x = 20; x <= 235; x++) {
            if (y <= 46 || y >= 145 || x <= 22 || x >= 233) {
                preview_buffer[y * 256 + x] = border_color;
            } else {
                preview_buffer[y * 256 + x] = bg_color;
            }
        }
    }
    
    uint16_t header_bg = blendRGB555_int(app_theme_color, bg_color, 8);
    for (int y = 47; y <= 59; y++) {
        for (int x = 23; x <= 232; x++) {
            preview_buffer[y * 256 + x] = header_bg;
        }
    }
    
    int title_len = strlen(title);
    int text_width = title_len * 6;
    int tx = 23 + (210 - text_width) / 2;
    int ty = 47 + (13 - 8) / 2;
    if (tx < 23) tx = 23;
    renderDrawTextOnBuffer(preview_buffer, title, tx, ty, RGB15(31, 31, 31), 0);
}
