#ifndef UI_H
#define UI_H

#include <nds.h>
#include <stdbool.h>
#include "ui_types.h"

extern AppState g_app_state;

#ifndef NO_UI_COMPAT_MACROS
#include "ui_compat.h"
#endif

// Include new modular headers
#include "widget_buttons.h"
#include "keyboard.h"
#include "toolbar.h"
#include "sidebar.h"
#include "modals.h"
#include "form_wifi.h"
#include "form_rename.h"
#include "view_canvas.h"
#include "view_menu.h"
#include "view_gallery.h"
#include "view_wizard.h"
#include "texts.h"

extern uint16_t modal_backup[256 * 156];
extern uint16_t preset_palettes[20][5];
extern uint16_t custom_palettes[50][5];

void uiDrawToolbar(void);
void uiOpenModal(int modal_idx);
void uiCloseModal(void);
void uiUpdateModalBackup(void);

void uiUpdatePickerPosFromActiveColor(void);
void uiUpdateColorPickerSelection(void);
void uiUpdateAngleWheelVisuals(void);
void uiDrawLayersOverlay(void);
extern bool ssid_manual_input;
void uiDrawUndoRedoButtons(void);
void uiDrawTopConsoleBox(const char* title);

extern uint16_t theme_colors[5];
extern const char* theme_names[5];

#endif // UI_H
