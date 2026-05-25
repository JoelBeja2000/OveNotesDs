#include "game.h"
#include "render.h"
#include "input.h"
#include "ui.h"
#include "net.h"
#include "lodepng.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <nds/arm9/keyboard.h>
#include "log.h"

GameState current_state = STATE_DRAW;

static char current_input[64] = "";
static int input_len = 0;
static int wizard_step = 0;
static uint16_t* backup_canvas = NULL;
static uint16_t* backup_preview = NULL;
static bool was_touching = false;
static bool touch_started_in_toolbar = false;
static int prev_x = 0;
static int prev_y = 0;
static int bg_sub_wizard = -1;

void gameInit(void) {
    // Enable 2D graphics systems
    powerOn(POWER_ALL_2D);

    // Swap displays: Main engine to bottom screen, Sub engine to top screen
    lcdMainOnBottom();

    // Map Video RAM:
    // Bank A: Main background memory (0x06000000) for drawing canvas
    vramSetBankA(VRAM_A_MAIN_BG);
    // Bank C: Sub background memory (0x06200000) for console and preview (offset 0KB)
    vramSetBankC(VRAM_C_SUB_BG);

    // Configure bottom screen (Main Engine): Mode 5, enable Background 2 (Extended Rotation Bitmap)
    videoSetMode(MODE_5_2D | DISPLAY_BG2_ACTIVE);
    int bg_canvas = bgInit(2, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    canvas_buffer = (uint16_t*)bgGetGfxPtr(bg_canvas);

    // Configure top screen (Sub Engine): Mode 5, enable Background 0 (Console) and Background 2 (Bitmap preview)
    videoSetModeSub(MODE_5_2D | DISPLAY_BG0_ACTIVE | DISPLAY_BG2_ACTIVE);

    // Initialize sub text console on BG0 (top screen)
    consoleInit(&subConsole, 
                0,                  // layer 0
                BgType_Text4bpp,     // text mode
                BgSize_T_256x256,    // map size 256x256
                28,                 // map base 28 (56KB offset)
                4,                  // tile base 4 (64KB offset)
                false,              // false = Sub Engine
                true);              // load default graphics

    // Initialize 128x128 16-bit bitmap on BG2 (top screen, map base 0, offset 0KB)
    int bg_sub_preview = bgInitSub(2, BgType_Bmp16, BgSize_B16_128x128, 0, 0);
    preview_buffer = (uint16_t*)bgGetGfxPtr(bg_sub_preview);

    // Center the 128x128 preview on the 256x192 top screen
    bgSet(bg_sub_preview, 0, 1 << 8, 1 << 8, -64 << 8, -32 << 8, 0, 0);
    bgUpdate();

    // Initialize drawing paper & toolbar
    renderInitCanvas();
    renderInitPreview();
    renderUpdatePreview();

    // Display title & welcome info
    printf("OveNotes DS v1.0\n");
    printf("====================\n");
    printf("Dibuje en la pantalla inferior.\n");
    printf("Seleccione pincel o borrador.\n");
    printf("Presione PUBLICAR para subir.\n\n");
}

static void enterWizardState(void) {
    printf("[GAME] Entrando a enterWizardState...\n");
    current_state = STATE_WIZARD;
    
    // Backup canvas and preview
    printf("[GAME] Creando backups en RAM del canvas y preview...\n");
    backup_canvas = malloc(256 * 192 * 2);
    backup_preview = malloc(128 * 128 * 2);
    if (backup_canvas && backup_preview) {
        memcpy(backup_canvas, canvas_buffer, 256 * 192 * 2);
        memcpy(backup_preview, preview_buffer, 128 * 128 * 2);
        printf("[GAME] Backups creados con exito\n");
    } else {
        printf("[GAME] Advertencia: fallo al asignar backups de memoria\n");
    }
    
    // Keep the bottom screen video mode as MODE_5_2D | DISPLAY_BG2_ACTIVE, so we can draw on canvas_buffer
    
    // Configurar la pantalla superior (Sub Engine) para un bitmap completo de 256x256 (BG3 activo)
    videoSetModeSub(MODE_5_2D | DISPLAY_BG3_ACTIVE);
    bg_sub_wizard = bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    wizard_buffer = (uint16_t*)bgGetGfxPtr(bg_sub_wizard);
    
    wizard_step = 0;
    strcpy(current_input, http_ip);
    input_len = strlen(current_input);
    
    uiDrawFormUI(wizard_step, current_input);
    uiDrawBottomForm(wizard_step, current_input);
}

static void exitWizardState(bool canceled) {
    if (!canceled) {
        netSaveConfig();
    }
    printf("[GAME] Saliendo de exitWizardState (cancelado: %d, IP: %s, Puerto: %s, SSID: %s)\n", canceled, http_ip, http_port_str, wifi_ssid);
    
    // Deshabilitar el fondo bitmap de la pantalla superior
    videoBgDisableSub(3);
    
    // Re-inicializar el subConsole en la pantalla superior para limpiar la corrupcion de VRAM
    consoleInit(&subConsole, 
                0,                  // layer 0
                BgType_Text4bpp,     // text mode
                BgSize_T_256x256,    // map size 256x256
                28,                 // map base 28 (56KB offset)
                4,                  // tile base 4 (64KB offset)
                false,              // false = Sub Engine
                true);              // load default graphics
    consoleSelect(&subConsole);
    
    // Restaurar la paleta de colores del fondo de la consola
    BG_PALETTE[0] = RGB15(31, 31, 31);
    BG_PALETTE[255] = RGB15(31, 31, 31);
    
    wizard_buffer = NULL;
    bg_sub_wizard = -1;
    
    if (backup_canvas && backup_preview) {
        memcpy(canvas_buffer, backup_canvas, 256 * 192 * 2);
        memcpy(preview_buffer, backup_preview, 128 * 128 * 2);
        free(backup_canvas);
        free(backup_preview);
        backup_canvas = NULL;
        backup_preview = NULL;
        printf("[GAME] Backups restaurados y liberados\n");
    }
    
    printf("\x1b[2J");
    if (canceled) {
        printf("Configuracion cancelada.\n");
    } else {
        printf("Configuracion guardada.\n");
    }
    current_state = STATE_DRAW;
}

static void runUpload(void) {
    printf("[GAME] Iniciando proceso de subida runUpload()...\n");
    consoleSelect(&subConsole);
    printf("\x1b[2J");
    printf("Iniciando Wi-Fi (BlocksDS)...\n");
    BG_PALETTE_SUB[0] = RGB15(31, 31, 0); // YELLOW: Initializing WiFi
    
    if (!netInitWifi()) {
        printf("[GAME] Error: fallo al iniciar Wi-Fi!\n");
        BG_PALETTE_SUB[0] = RGB15(31, 0, 0);
        for(int i=0; i<60; i++) swiWaitForVBlank();
        BG_PALETTE_SUB[0] = RGB15(31, 31, 31);
        current_state = STATE_DRAW;
        return;
    }
    
    printf("Codificando PNG...\n");
    printf("[GAME] Convirtiendo canvas a buffer RGB888...\n");
    BG_PALETTE_SUB[0] = RGB15(0, 0, 31); // BLUE: Encoding PNG
    
    unsigned char* rgb_buf = malloc(256 * 176 * 3);
    if (!rgb_buf) {
        printf("[GAME] Error: no hay memoria RAM para rgb_buf\n");
        current_state = STATE_DRAW;
        return;
    }
    
    int i = 0;
    for (int y = 0; y < 176; y++) {
        for (int x = 0; x < 256; x++) {
            uint16_t color = canvas_buffer[y * 256 + x];
            rgb_buf[i++] = (color & 0x1F) << 3;
            rgb_buf[i++] = ((color >> 5) & 0x1F) << 3;
            rgb_buf[i++] = ((color >> 10) & 0x1F) << 3;
        }
    }
    
    printf("[GAME] Iniciando codificacion lodepng...\n");
    unsigned char* png_data = NULL;
    size_t png_size = 0;
    unsigned png_err = lodepng_encode_memory(&png_data, &png_size, rgb_buf, 256, 176, LCT_RGB, 8);
    free(rgb_buf);
    
    if (png_err) {
        printf("[GAME] Error al codificar PNG: %d\n", png_err);
        BG_PALETTE_SUB[0] = RGB15(31, 0, 0);
        for(int j=0; j<60; j++) swiWaitForVBlank();
        BG_PALETTE_SUB[0] = RGB15(31, 31, 31);
        current_state = STATE_DRAW;
        return;
    }
    printf("[GAME] PNG codificado con exito (%u bytes)\n", (unsigned int)png_size);
    
    printf("Conectando al servidor HTTP %s:%s...\n", http_ip, http_port_str);
    BG_PALETTE_SUB[0] = RGB15(31, 15, 0); // ORANGE: Sending via Socket
    
    printf("[GAME] Llamando a enviarNotaHTTP...\n");
    if (enviarNotaHTTP(http_ip, atoi(http_port_str), png_data, png_size)) {
        printf("[GAME] Nota publicada con exito!\n");
        BG_PALETTE_SUB[0] = RGB15(0, 31, 0); // GREEN: Success!
    } else {
        printf("[GAME] Error: fallo al enviar nota HTTP\n");
        BG_PALETTE_SUB[0] = RGB15(31, 0, 0); // RED: Fail
    }
    
    free(png_data);
    printf("[GAME] Liberando png_data\n");
    for(int j=0; j<60; j++) swiWaitForVBlank();
    BG_PALETTE_SUB[0] = RGB15(31, 31, 31);
    printf("Listo para dibujar.\n");
    
    current_state = STATE_DRAW;
}

static void changeWizardStep(int new_step) {
    if (new_step < 0 || new_step > 2 || new_step == wizard_step) return;
    
    // Save current step
    if (wizard_step == 0)      strcpy(http_ip, current_input);
    else if (wizard_step == 1) strcpy(http_port_str, current_input);
    else if (wizard_step == 2) strcpy(wifi_ssid, current_input);
    
    wizard_step = new_step;
    
    // Load new step
    if (wizard_step == 0)      strcpy(current_input, http_ip);
    else if (wizard_step == 1) strcpy(current_input, http_port_str);
    else if (wizard_step == 2) strcpy(current_input, wifi_ssid);
    
    input_len = strlen(current_input);
    uiDrawFormUI(wizard_step, current_input);
    uiDrawBottomForm(wizard_step, current_input);
}

void gameUpdate(void) {
    uint32_t keys_held = inputGetKeysHeld();
    uint32_t keys_down = inputGetKeysDown();
    
    if (current_state == STATE_DRAW) {
        // Toggle toolbar visibility with D-pad Up or Down
        if (keys_down & (KEY_UP | KEY_DOWN)) {
            toolbar_hidden = !toolbar_hidden;
            if (toolbar_hidden) {
                // Clear the toolbar area (y = 176 to 191) to white (canvas bg color)
                for (int y = 176; y < 192; y++) {
                    for (int x = 0; x < 256; x++) {
                        canvas_buffer[y * 256 + x] = RGB15(31, 31, 31);
                    }
                }
            } else {
                // Restore toolbar
                uiDrawToolbar();
            }
            renderUpdatePreview();
        }

        printf("\x1b[12;0H"); 
        printf("Raw Keys: %08lX      \n", (unsigned long)keys_held);
        
        touchPosition touch;
        if (inputGetTouch(&touch)) {
            if (!was_touching) {
                touch_started_in_toolbar = !toolbar_hidden && (touch.py >= 176);
            }
            
            int limit_y = toolbar_hidden ? 192 : 176;
            if (touch.py < limit_y) {
                if (!touch_started_in_toolbar) {
                    if (is_bucket) {
                        if (!was_touching) {
                            renderFloodFill(touch.px, touch.py, palette_colors[active_color_idx]);
                            renderUpdatePreview();
                        }
                    } else {
                        uint16_t draw_color = is_eraser ? RGB15(31, 31, 31) : palette_colors[active_color_idx];
                        if (was_touching && prev_y < limit_y) {
                            renderDrawLine(prev_x, prev_y, touch.px, touch.py, draw_color, is_eraser ? eraser_size : active_brush_size, is_eraser);
                        } else {
                            if (is_eraser) {
                                renderDrawEraserPoint(touch.px, touch.py);
                            } else {
                                renderDrawBrushPoint(touch.px, touch.py, draw_color, active_brush_size);
                            }
                        }
                    }
                }
                prev_x = touch.px;
                prev_y = touch.py;
                was_touching = true;
                if (!is_bucket) {
                    renderUpdatePreview();
                }
            } else {
                prev_x = touch.px;
                prev_y = touch.py;
                was_touching = true;
            }
        } else {
            if (was_touching && touch_started_in_toolbar && prev_y >= 176) {
                int btn = prev_x / 32;
                if (btn >= 0 && btn < 8) {
                    if (btn == 0) {
                        if (is_eraser || is_bucket) {
                            is_eraser = false;
                            is_bucket = false;
                        } else {
                            if (active_brush_size == 1) active_brush_size = 3;
                            else if (active_brush_size == 3) active_brush_size = 5;
                            else active_brush_size = 1;
                        }
                        uiDrawToolbar();
                    } else if (btn == 1) {
                        if (!is_eraser) {
                            is_eraser = true;
                            is_bucket = false;
                        } else {
                            if (eraser_size == 4) eraser_size = 8;
                            else if (eraser_size == 8) eraser_size = 16;
                            else eraser_size = 4;
                        }
                        uiDrawToolbar();
                    } else if (btn == 2) {
                        is_eraser = false;
                        is_bucket = false;
                        active_color_idx = (active_color_idx + 1) % 5;
                        uiDrawToolbar();
                    } else if (btn == 3) {
                        is_eraser = false;
                        is_bucket = false;
                        drawing_mode = (drawing_mode + 1) % 4;
                        uiDrawToolbar();
                    } else if (btn == 4) {
                        is_eraser = false;
                        is_bucket = !is_bucket;
                        uiDrawToolbar();
                    } else if (btn == 5) {
                        bg_pattern_idx = (bg_pattern_idx + 1) % 4;
                        renderApplyBackgroundPattern(bg_pattern_idx);
                        uiDrawToolbar();
                    } else if (btn == 6) {
                        enterWizardState();
                    } else if (btn == 7) {
                        current_state = STATE_UPLOAD;
                    }
                }
            }
            was_touching = false;
            touch_started_in_toolbar = false;
        }
    } 
    else if (current_state == STATE_WIZARD) {
        if (keys_down & KEY_TOUCH) {
            touchPosition touch;
            if (inputGetTouch(&touch)) {
                // 1. Check if they touched the tabs: y = 42 to 62
                if (touch.py >= 42 && touch.py <= 62) {
                    if (touch.px >= 10 && touch.px <= 70) {
                        changeWizardStep(0);
                    } else if (touch.px >= 76 && touch.px <= 136) {
                        changeWizardStep(1);
                    } else if (touch.px >= 142 && touch.px <= 202) {
                        changeWizardStep(2);
                    }
                }
                // 2. Check if they touched the keyboard: y = 96 to 182
                else if (touch.py >= 96 && touch.py <= 182) {
                    bool shift_toggled = false;
                    bool caps_toggled = false;
                    bool enter_pressed = false;
                    bool backspace_pressed = false;
                    
                    char key = uiHandleKeyboardTouch(touch.px, touch.py, &shift_toggled, &caps_toggled, &enter_pressed, &backspace_pressed);
                    
                    if (shift_toggled || caps_toggled) {
                        uiDrawBottomForm(wizard_step, current_input);
                    } else if (enter_pressed) {
                        if (wizard_step < 2) {
                            changeWizardStep(wizard_step + 1);
                        } else {
                            if (strcmp(wifi_ssid, current_input) != 0) {
                                strcpy(wifi_ssid, current_input);
                                netDisconnect();
                            }
                            exitWizardState(false);
                            return;
                        }
                    } else if (backspace_pressed) {
                        if (input_len > 0) {
                            input_len--;
                            current_input[input_len] = '\0';
                        }
                        uiDrawFormUI(wizard_step, current_input);
                        uiDrawBottomForm(wizard_step, current_input);
                    } else if (key > 0) {
                        if (input_len < 63) {
                            current_input[input_len] = key;
                            input_len++;
                            current_input[input_len] = '\0';
                        }
                        uiDrawFormUI(wizard_step, current_input);
                        uiDrawBottomForm(wizard_step, current_input);
                    }
                }
            }
        }
        
        // Navegacion con cruceta (D-pad)
        if (keys_down & (KEY_UP | KEY_LEFT)) {
            int prev = (wizard_step - 1 + 3) % 3;
            changeWizardStep(prev);
        }
        if (keys_down & (KEY_DOWN | KEY_RIGHT)) {
            int next = (wizard_step + 1) % 3;
            changeWizardStep(next);
        }
        
        if (keys_down & KEY_A) {
            if (wizard_step == 0)      strcpy(http_ip, current_input);
            else if (wizard_step == 1) strcpy(http_port_str, current_input);
            else if (wizard_step == 2) {
                if (strcmp(wifi_ssid, current_input) != 0) {
                    strcpy(wifi_ssid, current_input);
                    netDisconnect();
                }
            }
            exitWizardState(false);
            return;
        }

        if (keys_down & KEY_B) {
            exitWizardState(true);
            return;
        }
    } 
    else if (current_state == STATE_UPLOAD) {
        runUpload();
    }
}
