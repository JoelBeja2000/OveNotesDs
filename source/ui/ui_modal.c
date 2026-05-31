#define NO_UI_COMPAT_MACROS
#include "ui_modal.h"
#include "ui_shared.h"
#include "ui.h"
#include "render.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

void uiDrawModalToolbox(const AppState* app, int y0, int y1) {
    renderDrawText(uiTxt(TXT_UTENSILIO), 16, y0 + 2, RGB15(31, 31, 31), 0);
    drawToolButtonWithIconAt(16, 86, 32, 52, uiTxt(TXT_PINCEL), 0, (!app->draw.is_eraser && !app->draw.is_bucket));
    drawToolButtonWithIconAt(92, 162, 32, 52, uiTxt(TXT_BORRADOR), 1, app->draw.is_eraser);
    drawToolButtonWithIconAt(168, 238, 32, 52, uiTxt(TXT_RELLENO), 2, app->draw.is_bucket);
    
    renderDrawText(uiTxt(TXT_TRAZO), 16, y0 + 36, RGB15(31, 31, 31), 0);
    drawToolButtonWithIconAt(16, 86, 66, 86, uiTxt(TXT_NORMAL), 3, (app->draw.drawing_mode == 0));
    drawToolButtonWithIconAt(92, 162, 66, 86, uiTxt(TXT_ROTUL), 4, (app->draw.drawing_mode == 1));
    
    renderDrawText(uiTxt(TXT_PATRON_BRUSH), 16, y0 + 70, RGB15(31, 31, 31), 0);
    for (int i = 0; i < 5; i++) {
        drawPatternBrushButtonAt(16 + i * 46, 16 + i * 46 + 40, 100, 120, 2 + i, (app->draw.drawing_mode == 2 + i));
    }
    
    renderDrawText(uiTxt(TXT_PLUMAS), 16, y0 + 104, RGB15(31, 31, 31), 0);
    drawPlumaButtonAt(16, 56, 134, 154, "PL1", 7, (app->draw.drawing_mode == 7));
    drawPlumaButtonAt(62, 102, 134, 154, "PL2", 8, (app->draw.drawing_mode == 8));
    drawPlumaButtonAt(108, 148, 134, 154, "PL3", 9, (app->draw.drawing_mode == 9));
    drawPlumaButtonAt(154, 194, 134, 154, "PL4", 10, (app->draw.drawing_mode == 10));
    char ang_lbl[16];
    sprintf(ang_lbl, "ANG:%d", app->draw.nib_angle);
    drawModalButtonAt(200, 240, 134, 154, ang_lbl, false);
}
 
void uiDrawModalBrushSize(const AppState* app, int y0, int y1) {
    char label[32];
    if (app->draw.is_eraser) {
        sprintf(label, uiTxt(TXT_GROSOR_BORRADOR), app->draw.eraser_size);
    } else {
        sprintf(label, uiTxt(TXT_GROSOR_PINCEL), app->draw.active_brush_size);
    }
    renderDrawText(label, 16, 126, RGB15(31, 31, 31), 0);
    
    drawRect(24, 144, 232, 148, RGB15(20, 20, 20));
    drawRectOutline(24, 144, 232, 148, RGB15(0, 0, 0));
    
    int knob_x = 24;
    if (app->draw.is_eraser) {
        knob_x = 24 + (app->draw.eraser_size - 2) * 208 / 28;
    } else {
        knob_x = 24 + (app->draw.active_brush_size - 1) * 208 / 14;
    }
    if (knob_x < 24) knob_x = 24;
    if (knob_x > 232) knob_x = 232;
    
    drawRect(knob_x - 4, 138, knob_x + 4, 154, RGB15(12, 12, 18));
    drawRectOutline(knob_x - 4, 138, knob_x + 4, 154, RGB15(0, 0, 0));
}
 
static void uiDrawColorPickerTabs(const AppState* app) {
    uint16_t tab_active_bg = blendRGB555_int(app->ui.app_theme_color, RGB15(4, 4, 5), 6);
    uint16_t tab_inactive_bg = RGB15(2, 2, 3);
    uint16_t tab_border = app->ui.app_theme_color;
    
    drawRect(8, 20, 127, 32, (app->ui.color_modal_tab == 0) ? tab_active_bg : tab_inactive_bg);
    drawRect(128, 20, 247, 32, (app->ui.color_modal_tab == 1) ? tab_active_bg : tab_inactive_bg);
    
    drawRectOutline(8, 20, 127, 32, (app->ui.color_modal_tab == 0) ? tab_border : blendRGB555_int(app->ui.app_theme_color, RGB15(2,2,3), 8));
    drawRectOutline(128, 20, 247, 32, (app->ui.color_modal_tab == 1) ? tab_border : blendRGB555_int(app->ui.app_theme_color, RGB15(2,2,3), 8));
    
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
    drawRectOutline(16, 56, 136, 116, RGB15(0, 0, 0));
    
    int reticle_x = 17 + (app->ui.picker_h * 119) / 360;
    int reticle_y = 57 + ((31 - app->ui.picker_s) * 59) / 31;
    if (reticle_x < 17) reticle_x = 17;
    if (reticle_x > 135) reticle_x = 135;
    if (reticle_y < 57) reticle_y = 57;
    if (reticle_y > 115) reticle_y = 115;
    uint16_t bg_col = hsv_to_rgb15(app->ui.picker_h, app->ui.picker_s, 31);
    int r = bg_col & 31, g = (bg_col >> 5) & 31, b = (bg_col >> 10) & 31;
    uint16_t reticle_color = (r + g + b > 45) ? RGB15(0, 0, 0) : RGB15(31, 31, 31);
    drawCircleOutline(reticle_x, reticle_y, 3, reticle_color);
}

static void uiDrawColorPickerBrightness(const AppState* app) {
    uint16_t preview_color = app->draw.palette_colors[app->draw.active_color_idx];
    drawRect(153, 57, 175, 115, preview_color);
    drawRectOutline(152, 56, 176, 116, RGB15(0, 0, 0));

    renderDrawBrightnessSlider(193, 57, 23, 59, app->ui.picker_h, app->ui.picker_s);
    drawRectOutline(192, 56, 216, 116, RGB15(0, 0, 0));
    
    int v_indicator_y = 57 + ((31 - app->ui.picker_v) * 59) / 31;
    if (v_indicator_y < 57) v_indicator_y = 57;
    if (v_indicator_y > 115) v_indicator_y = 115;
    drawRect(190, v_indicator_y - 1, 218, v_indicator_y + 1, RGB15(0, 0, 0));
    drawRect(191, v_indicator_y, 217, v_indicator_y, RGB15(31, 31, 31));
}

static void uiDrawColorPickerPresets(const AppState* app) {
    char page_lbl[16];
    sprintf(page_lbl, "%d/4", app->ui.preset_page + 1);
    
    for (int i = 0; i < 5; i++) {
        int preset_idx = app->ui.preset_page * 5 + i;
        int x0 = 12 + i * 48;
        int x1 = x0 + 38;
        int y0 = 134, y1 = 152;
        
        drawRectOutline(x0, y0, x1, y1, RGB15(0, 0, 0));
        int pad = 1;
        int x0_inner = x0 + pad;
        int x1_inner = x1 - pad;
        int W = x1_inner - x0_inner + 1;
        for (int c = 0; c < 5; c++) {
            int cx0 = x0_inner + (c * W) / 5;
            int cx1 = x0_inner + ((c + 1) * W) / 5 - 1;
            drawRect(cx0, y0 + pad, cx1, y1 - pad, preset_palettes[preset_idx][c]);
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
        drawRectOutline(x0, y0, x1, y1, outline);
        if (is_sel) {
            drawRectOutline(x0 + 1, y0 + 1, x1 - 1, y1 - 1, outline);
        }
        
        int pad = is_sel ? 2 : 1;
        int x0_inner = x0 + pad;
        int x1_inner = x1 - pad;
        int W = x1_inner - x0_inner + 1;
        for (int c = 0; c < 5; c++) {
            int cx0 = x0_inner + (c * W) / 5;
            int cx1 = x0_inner + ((c + 1) * W) / 5 - 1;
            drawRect(cx0, y0 + pad, cx1, y1 - pad, custom_palettes[global_idx][c]);
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

static void uiDrawBackgroundSettingsTabs(const AppState* app) {
    uint16_t tab_active_bg = blendRGB555_int(app->ui.app_theme_color, RGB15(4, 4, 5), 6);
    uint16_t tab_inactive_bg = RGB15(2, 2, 3);
    uint16_t tab_border = app->ui.app_theme_color;
    
    drawRect(8, 20, 127, 32, (app->ui.bg_modal_tab == 0) ? tab_active_bg : tab_inactive_bg);
    drawRect(128, 20, 247, 32, (app->ui.bg_modal_tab == 1) ? tab_active_bg : tab_inactive_bg);
    
    drawRectOutline(8, 20, 127, 32, (app->ui.bg_modal_tab == 0) ? tab_border : blendRGB555_int(app->ui.app_theme_color, RGB15(2,2,3), 8));
    drawRectOutline(128, 20, 247, 32, (app->ui.bg_modal_tab == 1) ? tab_border : blendRGB555_int(app->ui.app_theme_color, RGB15(2,2,3), 8));
    
    renderDrawText(uiTxt(TXT_PATRONES), 44, 23, (app->ui.bg_modal_tab == 0) ? RGB15(31, 31, 31) : RGB15(15, 15, 15), 0);
    renderDrawText(uiTxt(TXT_PERSPECTIVA), 152, 23, (app->ui.bg_modal_tab == 1) ? RGB15(31, 31, 31) : RGB15(15, 15, 15), 0);
}

static void uiDrawBackgroundSettingsPatterns(const AppState* app) {
    for (int i = 0; i < 4; i++) {
        drawPatternPreview(12 + i * 58, 38, 12 + i * 58 + 52, 78, i, (app->draw.bg_pattern_idx == i));
    }
    for (int i = 0; i < 4; i++) {
        drawPatternPreview(12 + i * 58, 84, 12 + i * 58 + 52, 124, 4 + i, (app->draw.bg_pattern_idx == 4 + i));
    }
    
    char color_p_lbl[24];
    sprintf(color_p_lbl, "%s P:%d", uiTxt(TXT_COL_ABBR), app->draw.bg_color_p_idx);
    drawModalButtonAt(12, 70, 132, 150, color_p_lbl, false);
    
    char color_s_lbl[24];
    sprintf(color_s_lbl, "%s S:%d", uiTxt(TXT_COL_ABBR), app->draw.bg_color_s_idx);
    drawModalButtonAt(74, 132, 132, 150, color_s_lbl, false);
    
    drawModalButtonAt(136, 194, 132, 150, app->draw.bg_modifiable ? uiTxt(TXT_MOD_SI) : uiTxt(TXT_MOD_NO), app->draw.bg_modifiable);
    
    char rot_lbl[16];
    sprintf(rot_lbl, "ROT:%d", app->draw.bg_angle);
    drawModalButtonAt(198, 244, 132, 150, rot_lbl, false);
}
 
static void uiDrawBackgroundSettingsPerspective(const AppState* app) {
    renderDrawText(uiTxt(TXT_MODO_PERSPECTIVA), 16, 36, RGB15(31, 31, 31), 0);
    
    drawModalButtonAt(12, 53, 46, 62, "OFF", (app->draw.perspective_mode == 0));
    drawModalButtonAt(57, 101, 46, 62, "1 VP", (app->draw.perspective_mode == 1));
    drawModalButtonAt(105, 149, 46, 62, "2 VP", (app->draw.perspective_mode == 2));
    drawModalButtonAt(153, 197, 46, 62, "3 VP", (app->draw.perspective_mode == 3));
    drawModalButtonAt(201, 244, 46, 62, "4 VP", (app->draw.perspective_mode == 4));
    
    if (app->draw.perspective_mode > 0) {
        drawModalButtonAt(12, 244, 68, 84, uiTxt(TXT_REUBICAR_PUNTOS), false);
    } else {
        drawModalButtonAt(12, 244, 68, 84, uiTxt(TXT_SIN_PERSPECTIVA), false);
    }
    
    if (app->draw.perspective_mode >= 1) {
        drawModalButtonAt(12, 65, 90, 106, uiTxt(TXT_PUNTO_1), false);
    } else {
        drawModalButtonDisabledAt(12, 65, 90, 106, uiTxt(TXT_PUNTO_1));
    }
    
    if (app->draw.perspective_mode >= 2) {
        drawModalButtonAt(71, 124, 90, 106, uiTxt(TXT_PUNTO_2), false);
    } else {
        drawModalButtonDisabledAt(71, 124, 90, 106, uiTxt(TXT_PUNTO_2));
    }
    
    if (app->draw.perspective_mode >= 3) {
        drawModalButtonAt(130, 183, 90, 106, uiTxt(TXT_PUNTO_3), false);
    } else {
        drawModalButtonDisabledAt(130, 183, 90, 106, uiTxt(TXT_PUNTO_3));
    }
    
    if (app->draw.perspective_mode >= 4) {
        drawModalButtonAt(189, 244, 90, 106, uiTxt(TXT_PUNTO_4), false);
    } else {
        drawModalButtonDisabledAt(189, 244, 90, 106, uiTxt(TXT_PUNTO_4));
    }
    
    char dens_lbl[32];
    sprintf(dens_lbl, uiTxt(TXT_DENSIDAD_REJILLA), app->draw.perspective_step);
    drawModalButtonAt(12, 244, 112, 128, dens_lbl, false);
}
 
void uiDrawModalBackgroundSettings(const AppState* app, int y0, int y1) {
    uiDrawBackgroundSettingsTabs(app);
    if (app->ui.bg_modal_tab == 0) {
        uiDrawBackgroundSettingsPatterns(app);
    } else {
        uiDrawBackgroundSettingsPerspective(app);
    }
}
 
void uiDrawModalAngleWheel(const AppState* app, int y0, int y1) {
    char label[32];
    if (app->draw.angle_target == 1) {
        sprintf(label, uiTxt(TXT_ROTACION_FONDO), app->draw.bg_angle);
    } else {
        sprintf(label, uiTxt(TXT_ANGULO_PLUMA), app->draw.nib_angle);
    }
    renderDrawText(label, 16, 96, RGB15(31, 31, 31), 0);
    
    int cx = 128;
    int cy = 132;
    int r = 28;
    drawCircleOutline(cx, cy, r, app->ui.app_theme_color);
    
    drawRect(cx - 2, cy, cx + 2, cy, RGB15(15, 15, 15));
    drawRect(cx, cy - 2, cx, cy + 2, RGB15(15, 15, 15));
    
    int current_angle = (app->draw.angle_target == 1) ? app->draw.bg_angle : app->draw.nib_angle;
    float rad = current_angle * 3.14159265f / 180.0f;
    int end_x = cx + (int)(cosf(rad) * 26.0f);
    int end_y = cy + (int)(sinf(rad) * 26.0f);
    drawLine(cx, cy, end_x, end_y, RGB15(31, 0, 0));
}
 
void uiDrawModalNoteMenu(const AppState* app, int y0, int y1) {
    renderDrawText(uiTxt(TXT_MENU_NOTA), 80, y0 + 6, RGB15(31, 31, 31), 0);
    
    drawModalButtonAt(24, 232, y0 + 22, y0 + 44, uiTxt(TXT_GUARDAR_NOTA_SD), false);
    drawModalButtonAt(24, 232, y0 + 50, y0 + 72, uiTxt(TXT_WIFI_CONNECTION), false);
    drawModalButtonAt(24, 232, y0 + 78, y0 + 100, uiTxt(TXT_VOLVER_AL_MENU_INICIO), false);
    drawModalButtonAt(24, 232, y0 + 106, y0 + 128, uiTxt(TXT_CANCEL), false);
}

void uiDrawModalSaveConfirm(const AppState* app, int y0, int y1) {
    renderDrawText(uiTxt(TXT_GUARDAR_ANTES_DE_SALIR), 56, y0 + 8, RGB15(31, 5, 5), 0);
    
    drawModalButtonAt(24, 232, y0 + 26, y0 + 48, uiTxt(TXT_SI_GUARDAR_Y_SALIR), false);
    drawModalButtonAt(24, 232, y0 + 54, y0 + 76, uiTxt(TXT_NO_SALIR_SIN_GUARDAR), false);
    drawModalButtonAt(24, 232, y0 + 82, y0 + 104, uiTxt(TXT_CANCEL), false);
}

void uiDrawLanguageModal(const AppState* app) {
    uint16_t modal_bg = RGB15(4, 4, 5);
    drawRect(32, 30, 224, 160, modal_bg);
    drawRectOutline(32, 30, 224, 160, app->ui.app_theme_color);
    drawRectOutline(33, 31, 223, 159, app->ui.app_theme_color);
    
    renderDrawText(uiTxt(TXT_SELECCIONAR_IDIOMA), 48, 38, RGB15(31, 31, 31), 0);
    
    uint16_t es_bg = (app->ui.current_lang == 0) ? blendRGB555_int(app->ui.app_theme_color, RGB15(4, 4, 5), 8) : RGB15(12, 12, 14);
    uint16_t es_border = (app->ui.current_lang == 0) ? app->ui.app_theme_color : RGB15(18, 18, 20);
    drawRect(48, 52, 208, 74, es_bg);
    drawRectOutline(48, 52, 208, 74, es_border);
    renderDrawText("ESPANOL", 98, 60, RGB15(31, 31, 31), 0);
    
    uint16_t en_bg = (app->ui.current_lang == 1) ? blendRGB555_int(app->ui.app_theme_color, RGB15(4, 4, 5), 8) : RGB15(12, 12, 14);
    uint16_t en_border = (app->ui.current_lang == 1) ? app->ui.app_theme_color : RGB15(18, 18, 20);
    drawRect(48, 78, 208, 100, en_bg);
    drawRectOutline(48, 78, 208, 100, en_border);
    renderDrawText("ENGLISH", 98, 86, RGB15(31, 31, 31), 0);

    uint16_t fr_bg = (app->ui.current_lang == 2) ? blendRGB555_int(app->ui.app_theme_color, RGB15(4, 4, 5), 8) : RGB15(12, 12, 14);
    uint16_t fr_border = (app->ui.current_lang == 2) ? app->ui.app_theme_color : RGB15(18, 18, 20);
    drawRect(48, 104, 208, 126, fr_bg);
    drawRectOutline(48, 104, 208, 126, fr_border);
    renderDrawText("FRANCAIS", 98, 112, RGB15(31, 31, 31), 0);
    
    drawRect(48, 132, 208, 152, RGB15(24, 6, 6));
    drawRectOutline(48, 132, 208, 152, RGB15(31, 0, 0));
    renderDrawText(uiTxt(TXT_CERRAR), 108, 138, RGB15(31, 31, 31), 0);
}
