#define NO_UI_COMPAT_MACROS
#include "keyboard.h"
#include "ui.h"
#include "render.h"
#include <string.h>

#define app_theme_color g_app_state.ui.app_theme_color
#define active_theme_idx g_app_state.ui.active_theme_idx
#define current_lang g_app_state.ui.current_lang

bool shift_active = false;
bool caps_active = false;

void drawKey(int x0, int y0, int x1, int y1, const char* label, bool highlighted) {
    uint16_t bg_color = highlighted ? blendRGB555_int(app_theme_color, RGB15(4, 4, 5), 6) : RGB15(2, 2, 3);
    uint16_t border_color = highlighted ? app_theme_color : blendRGB555_int(app_theme_color, RGB15(2, 2, 3), 8);
    uint16_t text_color = RGB15(31, 31, 31);
    if (highlighted && active_theme_idx == 0) {
        text_color = RGB15(0, 0, 0);
    }
    
    // Draw background
    for (int y = y0; y <= y1; y++) {
        for (int x = x0; x <= x1; x++) {
            canvas_buffer[y * 256 + x] = bg_color;
        }
    }
    // Draw border
    for (int x = x0; x <= x1; x++) {
        canvas_buffer[y0 * 256 + x] = border_color;
        canvas_buffer[y1 * 256 + x] = border_color;
    }
    for (int y = y0; y <= y1; y++) {
        canvas_buffer[y * 256 + x0] = border_color;
        canvas_buffer[y * 256 + x1] = border_color;
    }
    
    // Center label
    int len = strlen(label);
    int text_w = len * 6 - 1; // 5px per char + 1px spacing
    if (text_w < 0) text_w = 0;
    int text_h = 7;
    int tx = x0 + (x1 - x0 + 1 - text_w) / 2;
    int ty = y0 + (y1 - y0 + 1 - text_h) / 2;
    renderDrawText(label, tx, ty, text_color, 0);
}

void drawKeyboard(void) {
    bool upper = shift_active || caps_active;
    
    // Row 1: 1 to = and Bksp
    const char* r1_labels_low[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "="};
    const char* r1_labels_up[]  = {"!", "@", "#", "$", "%", "^", "&", "*", "(", ")", "_", "+"};
    for (int i = 0; i < 12; i++) {
        int x0 = 4 + i * 18;
        drawKey(x0, 96, x0 + 16, 116, upper ? r1_labels_up[i] : r1_labels_low[i], false);
    }
    drawKey(220, 96, 252, 116, "<-", false);
    
    // Row 2: q to backslash
    const char* r2_labels_low_qwerty[] = {"q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]", "\\"};
    const char* r2_labels_up_qwerty[]  = {"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "{", "}", "|"};
    const char* r2_labels_low_azerty[] = {"a", "z", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]", "\\"};
    const char* r2_labels_up_azerty[]  = {"A", "Z", "E", "R", "T", "Y", "U", "I", "O", "P", "{", "}", "|"};
    
    const char** r2_labels_low = (current_lang == 2) ? r2_labels_low_azerty : r2_labels_low_qwerty;
    const char** r2_labels_up  = (current_lang == 2) ? r2_labels_up_azerty : r2_labels_up_qwerty;
    
    for (int i = 0; i < 13; i++) {
        int x0 = 4 + i * 19;
        drawKey(x0, 118, x0 + 17, 138, upper ? r2_labels_up[i] : r2_labels_low[i], false);
    }
    
    // Row 3: Caps, a to ' and Rtrn
    drawKey(4, 140, 29, 160, "CPS", caps_active);
    
    const char* r3_labels_low_qwerty[] = {"a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "'"};
    const char* r3_labels_up_qwerty[]  = {"A", "S", "D", "F", "G", "H", "J", "K", "L", ":", "\""};
    const char* r3_labels_low_azerty[] = {"q", "s", "d", "f", "g", "h", "j", "k", "l", "m", "'"};
    const char* r3_labels_up_azerty[]  = {"Q", "S", "D", "F", "G", "H", "J", "K", "L", "M", "\""};
    
    const char** r3_labels_low = (current_lang == 2) ? r3_labels_low_azerty : r3_labels_low_qwerty;
    const char** r3_labels_up  = (current_lang == 2) ? r3_labels_up_azerty : r3_labels_up_qwerty;
    
    for (int i = 0; i < 11; i++) {
        int x0 = 31 + i * 18;
        drawKey(x0, 140, x0 + 16, 160, upper ? r3_labels_up[i] : r3_labels_low[i], false);
    }
    drawKey(229, 140, 253, 160, "Ent", false);
    
    // Row 4: Shift, z to /, Space
    drawKey(4, 162, 29, 182, "SFT", shift_active);
    
    const char* r4_labels_low_qwerty[] = {"z", "x", "c", "v", "b", "n", "m", ",", ".", "/"};
    const char* r4_labels_up_qwerty[]  = {"Z", "X", "C", "V", "B", "N", "M", "<", ">", "?"};
    const char* r4_labels_low_azerty[] = {"w", "x", "c", "v", "b", "n", ",", ";", ".", "/"};
    const char* r4_labels_up_azerty[]  = {"W", "X", "C", "V", "B", "N", "<", ";", ".", "?"};
    
    const char** r4_labels_low = (current_lang == 2) ? r4_labels_low_azerty : r4_labels_low_qwerty;
    const char** r4_labels_up  = (current_lang == 2) ? r4_labels_up_azerty : r4_labels_up_qwerty;
    
    for (int i = 0; i < 10; i++) {
        int x0 = 31 + i * 18;
        drawKey(x0, 162, x0 + 16, 182, upper ? r4_labels_up[i] : r4_labels_low[i], false);
    }
    drawKey(211, 162, 254, 182, uiTxt(TXT_KEY_SPACE), false);
}

char uiHandleKeyboardTouch(int tx, int ty, bool* shift_toggled, bool* caps_toggled, bool* enter_pressed, bool* backspace_pressed) {
    *shift_toggled = false;
    *caps_toggled = false;
    *enter_pressed = false;
    *backspace_pressed = false;
    
    bool upper = shift_active || caps_active;
    
    if (ty >= 96 && ty <= 116) {
        for (int i = 0; i < 12; i++) {
            int x0 = 4 + i * 18;
            if (tx >= x0 && tx <= x0 + 16) {
                const char* r1_labels_low[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "="};
                const char* r1_labels_up[]  = {"!", "@", "#", "$", "%", "^", "&", "*", "(", ")", "_", "+"};
                return upper ? r1_labels_up[i][0] : r1_labels_low[i][0];
            }
        }
        if (tx >= 220 && tx <= 252) {
            *backspace_pressed = true;
            return 0;
        }
    }
    
    if (ty >= 118 && ty <= 138) {
        for (int i = 0; i < 13; i++) {
            int x0 = 4 + i * 19;
            if (tx >= x0 && tx <= x0 + 17) {
                const char* r2_labels_low_qwerty[] = {"q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]", "\\"};
                const char* r2_labels_up_qwerty[]  = {"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "{", "}", "|"};
                const char* r2_labels_low_azerty[] = {"a", "z", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]", "\\"};
                const char* r2_labels_up_azerty[]  = {"A", "Z", "E", "R", "T", "Y", "U", "I", "O", "P", "{", "}", "|"};
                
                const char** r2_labels_low = (current_lang == 2) ? r2_labels_low_azerty : r2_labels_low_qwerty;
                const char** r2_labels_up  = (current_lang == 2) ? r2_labels_up_azerty : r2_labels_up_qwerty;
                return upper ? r2_labels_up[i][0] : r2_labels_low[i][0];
            }
        }
    }
    
    if (ty >= 140 && ty <= 160) {
        if (tx >= 4 && tx <= 29) {
            *caps_toggled = true;
            caps_active = !caps_active;
            return 0;
        }
        for (int i = 0; i < 11; i++) {
            int x0 = 31 + i * 18;
            if (tx >= x0 && tx <= x0 + 16) {
                const char* r3_labels_low_qwerty[] = {"a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "'"};
                const char* r3_labels_up_qwerty[]  = {"A", "S", "D", "F", "G", "H", "J", "K", "L", ":", "\""};
                const char* r3_labels_low_azerty[] = {"q", "s", "d", "f", "g", "h", "j", "k", "l", "m", "'"};
                const char* r3_labels_up_azerty[]  = {"Q", "S", "D", "F", "G", "H", "J", "K", "L", "M", "\""};
                
                const char** r3_labels_low = (current_lang == 2) ? r3_labels_low_azerty : r3_labels_low_qwerty;
                const char** r3_labels_up  = (current_lang == 2) ? r3_labels_up_azerty : r3_labels_up_qwerty;
                return upper ? r3_labels_up[i][0] : r3_labels_low[i][0];
            }
        }
        if (tx >= 229 && tx <= 253) {
            *enter_pressed = true;
            return 0;
        }
    }
    
    if (ty >= 162 && ty <= 182) {
        if (tx >= 4 && tx <= 29) {
            *shift_toggled = true;
            shift_active = !shift_active;
            return 0;
        }
        for (int i = 0; i < 10; i++) {
            int x0 = 31 + i * 18;
            if (tx >= x0 && tx <= x0 + 16) {
                const char* r4_labels_low_qwerty[] = {"z", "x", "c", "v", "b", "n", "m", ",", ".", "/"};
                const char* r4_labels_up_qwerty[]  = {"Z", "X", "C", "V", "B", "N", "M", "<", ">", "?"};
                const char* r4_labels_low_azerty[] = {"w", "x", "c", "v", "b", "n", ",", ";", ".", "/"};
                const char* r4_labels_up_azerty[]  = {"W", "X", "C", "V", "B", "N", "<", ";", ".", "?"};
                
                const char** r4_labels_low = (current_lang == 2) ? r4_labels_low_azerty : r4_labels_low_qwerty;
                const char** r4_labels_up  = (current_lang == 2) ? r4_labels_up_azerty : r4_labels_up_qwerty;
                
                char c = upper ? r4_labels_up[i][0] : r4_labels_low[i][0];
                if (shift_active) {
                    shift_active = false;
                    *shift_toggled = true;
                }
                return c;
            }
        }
        if (tx >= 211 && tx <= 254) {
            if (shift_active) {
                shift_active = false;
                *shift_toggled = true;
            }
            return ' ';
        }
    }
    
    return 0;
}
