#ifndef UI_MODAL_H
#define UI_MODAL_H

#include "ui_types.h"

// UI Layout constants replacing magic numbers
#define MODAL_BOX_X0 8
#define MODAL_BOX_X1 247
#define MODAL_HEADER_H 24
#define MODAL_PADDING 8

void uiDrawModalToolbox(const AppState* app, int y0, int y1);
void uiDrawModalBrushSize(const AppState* app, int y0, int y1);
void uiDrawModalColorPicker(const AppState* app, int y0, int y1);
void uiDrawModalBackgroundSettings(const AppState* app, int y0, int y1);
void uiDrawModalAngleWheel(const AppState* app, int y0, int y1);
void uiDrawModalNoteMenu(const AppState* app, int y0, int y1);
void uiDrawModalSaveConfirm(const AppState* app, int y0, int y1);
void uiDrawLanguageModal(const AppState* app);

#endif // UI_MODAL_H
