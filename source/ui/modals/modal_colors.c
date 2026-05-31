#define NO_UI_COMPAT_MACROS
#include "modals.h"
#include "ui.h"
#include "render.h"
#include "widget_buttons.h"
#include <stdio.h>

static void uiDrawColorPickerTabs(const AppState* app) {
    uint16_t tab_active_bg = blendRGB555_int(app->ui.app_theme_color, RGB15(4, 4, 5), 6);
    uint16_t tab_inactive_bg = RGB15(2, 2, 3);
    uint16_t tab_border = app->ui.app_theme_color;
    
    renderDrawRect(8, 20, 127, 32, (app->ui.color_modal_tab == 0) ? tab_active_bg : tab_inactive_bg);
    renderDrawRect(128, 20, 247, 32, (app->ui.color_modal_tab == 1) ? tab_active_bg : tab_inactive_bg);
    
    renderDrawRectOutline(8, 20, 127, 32, (app->ui.color_modal_tab == 0) ? tab_border : blendRGB555_int(app->ui.app_theme_color, RGB15(2,2,3), 8));
    renderDrawRectOutline(128, 20, 247, 32, (app->ui.color_modal_tab == 1) ? tab_border : blendRGB555_int(app->ui.app_theme_color, RGB15(2,2,3), 8));
    
    renderDrawText(uiTxt(TXT_PRESETS), 44, 23, (app->ui.color_modal_tab == 0) ? RGB15(31, 31, 31) : RGB15(15, 15, 15), 0);
    renderDrawText(uiTxt(TXT_MIS_PALETAS), 152, 23, (app->ui.color_modal_tab == 1) ? RGB15(31, 31, 31) : RGB15(15, 15, 15), 0);
}

static void uiDrawColorPickerSwatches(const AppState* app) {
    for (int i = 0; i < 5; i++) {
        drawModalColorButtonAt(16 + i * 46, 16 + i * 46 + 40, 36, 48, app->draw.palette_colors[i], (app->draw.active_color_idx == i));
    }
}

static void uiDrawColorPickerHSMap(const AppState* app) {
    renderDrawHSMap(17, 57, 119, 59);
    renderDrawRectOutline(16, 56, 136, 116, RGB15(0, 0, 0));
    
    int reticle_x = 17 + (app->ui.picker_h * 119) / 360;
    int reticle_y = 57 + ((31 - app->ui.picker_s) * 59) / 31;
    if (reticle_x < 17) reticle_x = 17;
    if (reticle_x > 135) reticle_x = 135;
    if (reticle_y < 57) reticle_y = 57;
    if (reticle_y > 115) reticle_y = 115;
    uint16_t bg_col = hsv_to_rgb15(app->ui.picker_h, app->ui.picker_s, 31);
    int r = bg_col & 31, g = (bg_col >> 5) & 31, b = (bg_col >> 10) & 31;
    uint16_t reticle_color = (r + g + b > 45) ? RGB15(0, 0, 0) : RGB15(31, 31, 31);
    renderDrawCircleOutline(reticle_x, reticle_y, 3, reticle_color);
}

static void uiDrawColorPickerBrightness(const AppState* app) {
    uint16_t preview_color = app->draw.palette_colors[app->draw.active_color_idx];
    renderDrawRect(153, 57, 175, 115, preview_color);
    renderDrawRectOutline(152, 56, 176, 116, RGB15(0, 0, 0));

    renderDrawBrightnessSlider(193, 57, 23, 59, app->ui.picker_h, app->ui.picker_s);
    renderDrawRectOutline(192, 56, 216, 116, RGB15(0, 0, 0));
    
    int v_indicator_y = 57 + ((31 - app->ui.picker_v) * 59) / 31;
    if (v_indicator_y < 57) v_indicator_y = 57;
    if (v_indicator_y > 115) v_indicator_y = 115;
    renderDrawRect(190, v_indicator_y - 1, 218, v_indicator_y + 1, RGB15(0, 0, 0));
    renderDrawRect(191, v_indicator_y, 217, v_indicator_y, RGB15(31, 31, 31));
}

static void uiDrawColorPickerPresets(const AppState* app) {
    char page_lbl[16];
    sprintf(page_lbl, "%d/4", app->ui.preset_page + 1);
    
    for (int i = 0; i < 5; i++) {
        int preset_idx = app->ui.preset_page * 5 + i;
        int x0 = 12 + i * 48;
        int x1 = x0 + 38;
        int y0 = 134, y1 = 152;
        
        renderDrawRectOutline(x0, y0, x1, y1, RGB15(0, 0, 0));
        int pad = 1;
        int x0_inner = x0 + pad;
        int x1_inner = x1 - pad;
        int W = x1_inner - x0_inner + 1;
        for (int c = 0; c < 5; c++) {
            int cx0 = x0_inner + (c * W) / 5;
            int cx1 = x0_inner + ((c + 1) * W) / 5 - 1;
            renderDrawRect(cx0, y0 + pad, cx1, y1 - pad, preset_palettes[preset_idx][c]);
        }
    }
    
    drawModalButtonAt(74, 114, 154, 170, "<-", false);
    renderDrawText(page_lbl, 122, 158, RGB15(31, 31, 31), 0);
    drawModalButtonAt(142, 182, 154, 170, "->", false);
}

static void uiDrawColorPickerCustom(const AppState* app) {
    char page_lbl[16];
    sprintf(page_lbl, "%d/10", app->ui.custom_page + 1);
    
    for (int i = 0; i < 5; i++) {
        int global_idx = app->ui.custom_page * 5 + i;
        int x0 = 12 + i * 48;
        int x1 = x0 + 38;
        int y0 = 134, y1 = 152;
        
        bool is_sel = (app->ui.selected_custom_slot == i);
        uint16_t outline = is_sel ? app->ui.app_theme_color : RGB15(0, 0, 0);
        renderDrawRectOutline(x0, y0, x1, y1, outline);
        if (is_sel) {
            renderDrawRectOutline(x0 + 1, y0 + 1, x1 - 1, y1 - 1, outline);
        }
        
        int pad = is_sel ? 2 : 1;
        int x0_inner = x0 + pad;
        int x1_inner = x1 - pad;
        int W = x1_inner - x0_inner + 1;
        for (int c = 0; c < 5; c++) {
            int cx0 = x0_inner + (c * W) / 5;
            int cx1 = x0_inner + ((c + 1) * W) / 5 - 1;
            renderDrawRect(cx0, y0 + pad, cx1, y1 - pad, custom_palettes[global_idx][c]);
        }
    }
    
    drawModalButtonAt(12, 82, 154, 170, uiTxt(TXT_GUARDAR_BTN), false);
    drawModalButtonAt(114, 144, 154, 170, "<-", false);
    drawModalButtonAt(174, 204, 154, 170, "->", false);
    renderDrawText(page_lbl, 210, 158, RGB15(31, 31, 31), 0);
}

void uiDrawModalColorPicker(const AppState* app, int y0, int y1) {
    uiDrawColorPickerTabs(app);
    uiDrawColorPickerSwatches(app);
    uiDrawColorPickerHSMap(app);
    uiDrawColorPickerBrightness(app);
    
    if (app->ui.color_modal_tab == 0) {
        uiDrawColorPickerPresets(app);
    } else {
        uiDrawColorPickerCustom(app);
    }
}
