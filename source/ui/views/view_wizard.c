#define NO_UI_COMPAT_MACROS
#include "view_wizard.h"
#include "ui.h"

void view_wizard_show(int step, const char* input_text) {
    uiDrawFormUI(step, input_text);
    uiDrawBottomForm(step, input_text);
}
