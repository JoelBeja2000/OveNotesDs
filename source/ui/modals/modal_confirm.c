#define NO_UI_COMPAT_MACROS
#include "modals.h"
#include "ui.h"
#include "render.h"
#include "widget_buttons.h"
#include <stdio.h>
#include <math.h>

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
    renderDrawCircleOutline(cx, cy, r, app->ui.app_theme_color);
    
    renderDrawRect(cx - 2, cy, cx + 2, cy, RGB15(15, 15, 15));
    renderDrawRect(cx, cy - 2, cx, cy + 2, RGB15(15, 15, 15));
    
    int current_angle = (app->draw.angle_target == 1) ? app->draw.bg_angle : app->draw.nib_angle;
    float rad = current_angle * 3.14159265f / 180.0f;
    int end_x = cx + (int)(cosf(rad) * 26.0f);
    int end_y = cy + (int)(sinf(rad) * 26.0f);
    renderDrawLineSimple(cx, cy, end_x, end_y, RGB15(31, 0, 0));
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
