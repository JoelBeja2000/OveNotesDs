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
    
    // Ocultar el dibujo (BG2) en la pantalla inferior (Main) y activar BG0 y BG3
    videoSetMode(MODE_0_2D | DISPLAY_BG0_ACTIVE | DISPLAY_BG3_ACTIVE);
    
    // Configurar la pantalla superior (Sub Engine) para un bitmap completo de 256x256 (BG3 activo)
    videoSetModeSub(MODE_5_2D | DISPLAY_BG3_ACTIVE);
    bg_sub_wizard = bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    wizard_buffer = (uint16_t*)bgGetGfxPtr(bg_sub_wizard);
    
    // Inicializar la consola para la pantalla inferior (Main Engine)
    consoleInit(&bottom_form_console, 
                0,                  // layer 0
                BgType_Text4bpp,     // text mode
                BgSize_T_256x256,    // map size 256x256
                22,                 // map base 22
                3,                  // tile base 3 (evita colisionar con el teclado)
                true,               // true = Main Engine
                true);              // load default graphics

    // Configurar la paleta de colores de la consola inferior (texto negro sobre fondo gris claro)
    BG_PALETTE[0] = RGB15(28, 28, 28);        // Fondo gris claro pantalla inferior
    BG_PALETTE[255] = RGB15(0, 0, 0);         // Texto negro pantalla inferior

    // Inicializar el teclado en la pantalla inferior (Main Engine)
    keyboardInit(NULL, 3, BgType_Text4bpp, BgSize_T_256x512, 20, 0, true, true);
    keyboardShow();
    
    wizard_step = 0;
    strcpy(current_input, http_ip);
    input_len = strlen(current_input);
    
    uiDrawFormUI(wizard_step, current_input);
    uiDrawBottomButtons(wizard_step);
}

static void exitWizardState(bool canceled) {
    if (!canceled) {
        if (wizard_step == 0)      strcpy(http_ip, current_input);
        else if (wizard_step == 1) strcpy(http_port_str, current_input);
        else if (wizard_step == 2) strcpy(wifi_ssid, current_input);
        netSaveConfig();
        netDisconnect();
    }
    printf("[GAME] Saliendo de exitWizardState (cancelado: %d, IP: %s, Puerto: %s, SSID: %s)\n", canceled, http_ip, http_port_str, wifi_ssid);
    keyboardHide();
    
    // Restaurar los modos de video de ambas pantallas a su estado de dibujo original
    videoSetMode(MODE_5_2D | DISPLAY_BG2_ACTIVE);
    videoSetModeSub(MODE_5_2D | DISPLAY_BG0_ACTIVE | DISPLAY_BG2_ACTIVE);
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
    
    if (canceled) {
        printf("\x1b[2J");
        printf("Envio cancelado.\n");
        current_state = STATE_DRAW;
    } else {
        current_state = STATE_UPLOAD;
    }
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
    uiDrawBottomButtons(wizard_step);
}

void gameUpdate(void) {
    uint32_t keys_held = inputGetKeysHeld();
    uint32_t keys_down = inputGetKeysDown();
    
    if (current_state == STATE_DRAW) {
        printf("\x1b[12;0H"); 
        printf("Raw Keys: %08lX      \n", (unsigned long)keys_held);
        
        touchPosition touch;
        if (inputGetTouch(&touch)) {
            if (!was_touching) {
                touch_started_in_toolbar = (touch.py >= 176);
            }
            
            if (touch.py < 176) {
                uint16_t draw_color = is_eraser ? RGB15(31, 31, 31) : RGB15(0, 0, 0);
                if (!touch_started_in_toolbar) {
                    if (was_touching && prev_y < 176) {
                        renderDrawLine(prev_x, prev_y, touch.px, touch.py, draw_color, active_brush_size, is_eraser);
                    } else {
                        if (is_eraser) {
                            renderDrawEraserPoint(touch.px, touch.py);
                        } else {
                            renderDrawBrushPoint(touch.px, touch.py, draw_color, active_brush_size);
                        }
                    }
                }
                prev_x = touch.px;
                prev_y = touch.py;
                was_touching = true;
                renderUpdatePreview();
            } else {
                prev_x = touch.px;
                prev_y = touch.py;
                was_touching = true;
            }
        } else {
            if (was_touching && touch_started_in_toolbar && prev_y >= 176) {
                if (prev_x >= 0 && prev_x < 30) {
                    active_brush_size = 1;
                    is_eraser = false;
                    uiDrawToolbar();
                } else if (prev_x >= 30 && prev_x < 60) {
                    active_brush_size = 3;
                    is_eraser = false;
                    uiDrawToolbar();
                } else if (prev_x >= 60 && prev_x < 90) {
                    active_brush_size = 5;
                    is_eraser = false;
                    uiDrawToolbar();
                } else if (prev_x >= 90 && prev_x < 130) {
                    is_eraser = true;
                    uiDrawToolbar();
                } else if (prev_x >= 130 && prev_x < 185) {
                    enterWizardState();
                } else if (prev_x >= 185 && prev_x <= 255) {
                    current_state = STATE_UPLOAD;
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
                if (touch.py < 64) {
                    int tapped_step = touch.px / 85;
                    if (tapped_step > 2) tapped_step = 2;
                    changeWizardStep(tapped_step);
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
            else if (wizard_step == 2) strcpy(wifi_ssid, current_input);
            exitWizardState(false);
            return;
        }

        if (keys_down & KEY_B) {
            exitWizardState(true);
            return;
        }
        
        int key = keyboardUpdate();
        if (key > 0) {
            if (key == '\n') {
                if (wizard_step < 2) {
                    changeWizardStep(wizard_step + 1);
                } else {
                    strcpy(wifi_ssid, current_input);
                    exitWizardState(false);
                    return;
                }
            } else if (key == '\b') {
                if (input_len > 0) {
                    input_len--;
                    current_input[input_len] = '\0';
                }
                uiDrawFormUI(wizard_step, current_input);
            } else if (input_len < 63) {
                current_input[input_len] = (char)key;
                input_len++;
                current_input[input_len] = '\0';
                uiDrawFormUI(wizard_step, current_input);
            }
        }
    } 
    else if (current_state == STATE_UPLOAD) {
        runUpload();
    }
}
