#define NO_UI_COMPAT_MACROS
#include "ui.h"
#include "render.h"
#include "net.h"
#include "modals.h"
#include "toolbar.h"
#include "sidebar.h"
#include "view_canvas.h"
#include "widget_buttons.h"
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
        .show_help_modal = false,
        .help_page = 0,
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

extern const char* const strings_es[];
extern const char* const strings_en[];
extern const char* const strings_fr[];

const char* uiTxt(TextId id) {
    if (id < 0 || id >= TXT_MAX) return "";
    if (current_lang == 2) return strings_fr[id];
    if (current_lang == 1) return strings_en[id];
    return strings_es[id];
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
    renderDrawCircleOutline(reticle_x, reticle_y, 3, reticle_color);

    // 3. Draw the vertical Preview Swatch
    uint16_t preview_color = palette_colors[active_color_idx];
    renderDrawRect(153, 57, 175, 115, preview_color);

    // 4. Draw the vertical Value (brightness) slider
    for (int dy = 0; dy < 59; dy++) {
        int v = 31 - (dy * 31) / 59;
        uint16_t slider_color = hsv_to_rgb15(picker_h, picker_s, v);
        for (int dx = 0; dx < 23; dx++) {
            canvas_buffer[(57 + dy) * 256 + (193 + dx)] = slider_color;
        }
    }
    
    // Clear left and right sides of Value indicator line to grey background
    renderDrawRect(190, 57, 191, 115, RGB15(28, 28, 28));
    renderDrawRect(217, 57, 218, 115, RGB15(28, 28, 28));
    // Restore vertical borders of Value slider
    for (int y = 57; y <= 115; y++) {
        canvas_buffer[y * 256 + 192] = RGB15(0, 0, 0);
        canvas_buffer[y * 256 + 216] = RGB15(0, 0, 0);
    }
    
    // Draw Value indicator line
    int v_indicator_y = 57 + ((31 - picker_v) * 59) / 31;
    if (v_indicator_y < 57) v_indicator_y = 57;
    if (v_indicator_y > 115) v_indicator_y = 115;
    renderDrawRect(190, v_indicator_y - 1, 218, v_indicator_y + 1, RGB15(0, 0, 0));
    renderDrawRect(191, v_indicator_y, 217, v_indicator_y, RGB15(31, 31, 31));
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
    renderDrawRect(8, y0, 247, y1, modal_bg);
    renderDrawRectOutline(8, y0, 247, y1, app_theme_color);
    
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
    renderDrawRect(16, 96, 220, 104, light_grey);
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
    renderDrawFilledCircle(cx, cy, 27, light_grey);
    
    // Redraw crosshair inside circle
    renderDrawRect(cx - 2, cy, cx + 2, cy, RGB15(15, 15, 15));
    renderDrawRect(cx, cy - 2, cx, cy + 2, RGB15(15, 15, 15));
    
    // Draw the new angle line
    int current_angle = (angle_target == 1) ? bg_angle : nib_angle;
    float rad = current_angle * 3.14159265f / 180.0f;
    int end_x = cx + (int)(cosf(rad) * 26.0f);
    int end_y = cy + (int)(sinf(rad) * 26.0f);
    renderDrawLineSimple(cx, cy, end_x, end_y, RGB15(31, 0, 0));
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
