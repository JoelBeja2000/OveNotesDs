#define NO_UI_COMPAT_MACROS
#include "modals.h"
#include "ui.h"
#include "render.h"
#include "widget_buttons.h"
#include <stdio.h>

static void uiDrawBackgroundSettingsTabs(const AppState* app) {
    uint16_t tab_active_bg = blendRGB555_int(app->ui.app_theme_color, RGB15(4, 4, 5), 6);
    uint16_t tab_inactive_bg = RGB15(2, 2, 3);
    uint16_t tab_border = app->ui.app_theme_color;
    
    renderDrawRect(8, 20, 127, 32, (app->ui.bg_modal_tab == 0) ? tab_active_bg : tab_inactive_bg);
    renderDrawRect(128, 20, 247, 32, (app->ui.bg_modal_tab == 1) ? tab_active_bg : tab_inactive_bg);
    
    renderDrawRectOutline(8, 20, 127, 32, (app->ui.bg_modal_tab == 0) ? tab_border : blendRGB555_int(app->ui.app_theme_color, RGB15(2,2,3), 8));
    renderDrawRectOutline(128, 20, 247, 32, (app->ui.bg_modal_tab == 1) ? tab_border : blendRGB555_int(app->ui.app_theme_color, RGB15(2,2,3), 8));
    
    renderDrawText(uiTxt(TXT_PATRONES), 44, 23, (app->ui.bg_modal_tab == 0) ? RGB15(31, 31, 31) : RGB15(15, 15, 15), 0);
    renderDrawText(uiTxt(TXT_PERSPECTIVA), 152, 23, (app->ui.bg_modal_tab == 1) ? RGB15(31, 31, 31) : RGB15(15, 15, 15), 0);
}

static void uiDrawBackgroundSettingsPatterns(const AppState* app) {
    for (int i = 0; i < 4; i++) {
        renderDrawPatternPreview(12 + i * 58, 38, 12 + i * 58 + 52, 78, i, (app->draw.bg_pattern_idx == i));
    }
    for (int i = 0; i < 4; i++) {
        renderDrawPatternPreview(12 + i * 58, 84, 12 + i * 58 + 52, 124, 4 + i, (app->draw.bg_pattern_idx == 4 + i));
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
