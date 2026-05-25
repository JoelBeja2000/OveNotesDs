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
#include <math.h>

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

bool placing_vanishing_points = false;
int placing_point_idx = 0;
bool placing_single_vanishing_point = false;
bool ignore_touch_until_release = false;

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

    drawing_buffer = (uint16_t*)malloc(256 * 192 * sizeof(uint16_t));
    if (drawing_buffer == NULL) {
        printf("[FATAL] Error de asignacion de memoria.\n");
    }

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
        if (placing_vanishing_points) {
            touchPosition touch;
            if (inputGetTouch(&touch)) {
                if (!was_touching) {
                    prev_x = touch.px;
                    prev_y = touch.py;
                }
                was_touching = true;
            } else {
                if (was_touching) {
                    if (prev_y >= 0 && prev_y < 176 && prev_x >= 0 && prev_x < 256) {
                        perspective_points[placing_point_idx][0] = prev_x;
                        perspective_points[placing_point_idx][1] = prev_y;
                        
                        renderComposeCanvas();
                        
                        if (placing_single_vanishing_point) {
                            placing_vanishing_points = false;
                            placing_single_vanishing_point = false;
                            uiDrawToolbar();
                        } else {
                            placing_point_idx++;
                            if (placing_point_idx >= perspective_mode) {
                                placing_vanishing_points = false;
                                uiDrawToolbar();
                            }
                        }
                        ignore_touch_until_release = true;
                    }
                    was_touching = false;
                }
            }
            return;
        }
        if (open_modal != -1) {
            // Drag interaction for size slider (open_modal == 1), color sliders (open_modal == 2), and angle selector (open_modal == 4)
            if ((open_modal == 1 || open_modal == 2 || open_modal == 4) && (keys_held & KEY_TOUCH)) {
                touchPosition touch;
                if (inputGetTouch(&touch)) {
                    if (open_modal == 1) {
                        if (touch.py >= 120 && touch.py <= 170 && touch.px >= 8 && touch.px <= 247) {
                            int tx = touch.px;
                            if (tx < 24) tx = 24;
                            if (tx > 232) tx = 232;
                            
                            if (is_eraser) {
                                int new_size = 2 + (tx - 24) * 28 / 208;
                                if (new_size < 2) new_size = 2;
                                if (new_size > 30) new_size = 30;
                                if (eraser_size != new_size) {
                                    eraser_size = new_size;
                                    uiOpenModal(1);
                                }
                            } else {
                                int new_size = 1 + (tx - 24) * 14 / 208;
                                if (new_size < 1) new_size = 1;
                                if (new_size > 15) new_size = 15;
                                if (active_brush_size != new_size) {
                                    active_brush_size = new_size;
                                    uiOpenModal(1);
                                }
                            }
                        }
                    } else if (open_modal == 2) {
                        if (touch.px >= 48 && touch.px <= 224) {
                            if (touch.py >= 52 && touch.py <= 70) {
                                int h = ((touch.px - 48) * 360) / 176;
                                if (h < 0) h = 0;
                                if (h > 360) h = 360;
                                if (picker_h != h) {
                                    picker_h = h;
                                    palette_colors[active_color_idx] = hsv_to_rgb15(picker_h, picker_s, picker_v);
                                    is_eraser = false;
                                    uiUpdateColorPickerSelection();
                                    uiDrawToolbar();
                                }
                            } else if (touch.py >= 72 && touch.py <= 90) {
                                int s = ((touch.px - 48) * 31) / 176;
                                if (s < 0) s = 0;
                                if (s > 31) s = 31;
                                if (picker_s != s) {
                                    picker_s = s;
                                    palette_colors[active_color_idx] = hsv_to_rgb15(picker_h, picker_s, picker_v);
                                    is_eraser = false;
                                    uiUpdateColorPickerSelection();
                                    uiDrawToolbar();
                                }
                            } else if (touch.py >= 92 && touch.py <= 110) {
                                int v = ((touch.px - 48) * 31) / 176;
                                if (v < 0) v = 0;
                                if (v > 31) v = 31;
                                if (picker_v != v) {
                                    picker_v = v;
                                    palette_colors[active_color_idx] = hsv_to_rgb15(picker_h, picker_s, picker_v);
                                    is_eraser = false;
                                    uiUpdateColorPickerSelection();
                                    uiDrawToolbar();
                                }
                            }
                        }
                    } else if (open_modal == 4) {
                        float dx = touch.px - 128;
                        float dy = touch.py - 132;
                        if (dx*dx + dy*dy <= 28*28) {
                            float rad = atan2f(dy, dx);
                            int angle = (int)(rad * 180.0f / 3.14159f);
                            if (angle < 0) angle += 360;
                            if (angle_target == 1) {
                                if (bg_angle != angle) {
                                    bg_angle = angle;
                                    renderApplyBackgroundPattern(bg_pattern_idx);
                                    renderComposeCanvas();
                                    uiUpdateAngleWheelVisuals();
                                }
                            } else {
                                if (nib_angle != angle) {
                                    nib_angle = angle;
                                    uiUpdateAngleWheelVisuals();
                                }
                            }
                        }
                    }
                }
            }

            // Click/tap interaction for selections
            touchPosition touch;
            if (inputGetTouch(&touch)) {
                if (!was_touching) {
                    prev_x = touch.px;
                    prev_y = touch.py;
                }
                was_touching = true;
            } else {
                if (was_touching) {
                    int modal_y0 = (open_modal == 0 || open_modal == 2 || open_modal == 3) ? 20 : ((open_modal == 4) ? 90 : 120);
                    if (prev_y >= modal_y0 && prev_y <= 170 && prev_x >= 8 && prev_x <= 247) {
                        bool option_selected = false;
                        if (open_modal == 0) { // TOOL
                            // Row 1: UTENSILIO (y = 32..52)
                            if (prev_y >= 32 && prev_y <= 52) {
                                if (prev_x >= 16 && prev_x <= 86) {
                                    is_eraser = false;
                                    is_bucket = false;
                                    option_selected = true;
                                } else if (prev_x >= 92 && prev_x <= 162) {
                                    is_eraser = true;
                                    is_bucket = false;
                                    option_selected = true;
                                } else if (prev_x >= 168 && prev_x <= 238) {
                                    is_eraser = false;
                                    is_bucket = true;
                                    option_selected = true;
                                }
                            }
                            // Row 2: TRAZO (y = 66..86)
                            else if (prev_y >= 66 && prev_y <= 86) {
                                if (prev_x >= 16 && prev_x <= 86) {
                                    drawing_mode = 0;
                                    is_eraser = false;
                                    is_bucket = false;
                                    option_selected = true;
                                } else if (prev_x >= 92 && prev_x <= 162) {
                                    drawing_mode = 1;
                                    is_eraser = false;
                                    is_bucket = false;
                                    option_selected = true;
                                }
                            }
                            // Row 3: PATRON (y = 100..120)
                            else if (prev_y >= 100 && prev_y <= 120) {
                                for (int i = 0; i < 5; i++) {
                                    int x0 = 16 + i * 46;
                                    int x1 = x0 + 40;
                                    if (prev_x >= x0 && prev_x <= x1) {
                                        drawing_mode = 2 + i;
                                        is_eraser = false;
                                        is_bucket = false;
                                        option_selected = true;
                                        break;
                                    }
                                }
                            }
                            // Row 4: PLUMAS (y = 134..154)
                            else if (prev_y >= 134 && prev_y <= 154) {
                                if (prev_x >= 16 && prev_x <= 56) {
                                    drawing_mode = 7;
                                    is_eraser = false;
                                    is_bucket = false;
                                    option_selected = true;
                                } else if (prev_x >= 62 && prev_x <= 102) {
                                    drawing_mode = 8;
                                    is_eraser = false;
                                    is_bucket = false;
                                    option_selected = true;
                                } else if (prev_x >= 108 && prev_x <= 148) {
                                    drawing_mode = 9;
                                    is_eraser = false;
                                    is_bucket = false;
                                    option_selected = true;
                                } else if (prev_x >= 154 && prev_x <= 194) {
                                    drawing_mode = 10;
                                    is_eraser = false;
                                    is_bucket = false;
                                    option_selected = true;
                                } else if (prev_x >= 200 && prev_x <= 240) {
                                    nib_angle = (nib_angle + 45) % 180;
                                    angle_target = 0;
                                    uiOpenModal(0);
                                    uiDrawToolbar();
                                    option_selected = false;
                                }
                            }
                        } else if (open_modal == 4) { // ANGLE
                            float dx = prev_x - 128;
                            float dy = prev_y - 132;
                            if (dx*dx + dy*dy <= 28*28) {
                                float rad = atan2f(dy, dx);
                                int angle = (int)(rad * 180.0f / 3.14159f);
                                if (angle < 0) angle += 360;
                                if (angle_target == 1) {
                                    bg_angle = angle;
                                    renderApplyBackgroundPattern(bg_pattern_idx);
                                    renderComposeCanvas();
                                } else {
                                    nib_angle = angle;
                                }
                                uiUpdateAngleWheelVisuals();
                            }
                        } else if (open_modal == 2) { // COLOR
                            // 1. Check tab bar (y = 20..32)
                            if (prev_y >= 20 && prev_y <= 32) {
                                if (prev_x >= 8 && prev_x <= 128) {
                                    if (color_modal_tab != 0) {
                                        color_modal_tab = 0;
                                        uiOpenModal(2);
                                    }
                                } else if (prev_x > 128 && prev_x <= 247) {
                                    if (color_modal_tab != 1) {
                                        color_modal_tab = 1;
                                        uiOpenModal(2);
                                    }
                                }
                            }
                            // 2. Check active swatches (y = 36..48)
                            else if (prev_y >= 36 && prev_y <= 48) {
                                for (int i = 0; i < 5; i++) {
                                    int x0 = 16 + i * 46;
                                    int x1 = x0 + 40;
                                    if (prev_x >= x0 && prev_x <= x1) {
                                        active_color_idx = i;
                                        is_eraser = false;
                                        uiUpdatePickerPosFromActiveColor();
                                        uiOpenModal(2);
                                        uiDrawToolbar();
                                        break;
                                    }
                                }
                            }
                            // 3. Check HSV sliders
                            else if (prev_x >= 48 && prev_x <= 224) {
                                if (prev_y >= 52 && prev_y <= 70) {
                                    int h = ((prev_x - 48) * 360) / 176;
                                    if (h < 0) h = 0;
                                    if (h > 360) h = 360;
                                    picker_h = h;
                                    palette_colors[active_color_idx] = hsv_to_rgb15(picker_h, picker_s, picker_v);
                                    is_eraser = false;
                                    uiUpdateColorPickerSelection();
                                    uiDrawToolbar();
                                } else if (prev_y >= 72 && prev_y <= 90) {
                                    int s = ((prev_x - 48) * 31) / 176;
                                    if (s < 0) s = 0;
                                    if (s > 31) s = 31;
                                    picker_s = s;
                                    palette_colors[active_color_idx] = hsv_to_rgb15(picker_h, picker_s, picker_v);
                                    is_eraser = false;
                                    uiUpdateColorPickerSelection();
                                    uiDrawToolbar();
                                } else if (prev_y >= 92 && prev_y <= 110) {
                                    int v = ((prev_x - 48) * 31) / 176;
                                    if (v < 0) v = 0;
                                    if (v > 31) v = 31;
                                    picker_v = v;
                                    palette_colors[active_color_idx] = hsv_to_rgb15(picker_h, picker_s, picker_v);
                                    is_eraser = false;
                                    uiUpdateColorPickerSelection();
                                    uiDrawToolbar();
                                }
                            }
                            // 4. Check bottom area (y = 120..170)
                            else if (prev_y >= 120 && prev_y <= 170) {
                                if (color_modal_tab == 0) {
                                    // PRESETS page buttons:
                                    // Preset buttons: slot i (0..4) -> x0 = 12 + i * 48, x1 = x0 + 38, y = 134..152
                                    if (prev_y >= 134 && prev_y <= 152) {
                                        if (prev_x >= 12 && prev_x <= 242) {
                                            for (int i = 0; i < 5; i++) {
                                                int x0 = 12 + i * 48;
                                                int x1 = x0 + 38;
                                                if (prev_x >= x0 && prev_x <= x1) {
                                                    int preset_idx = preset_page * 5 + i;
                                                    if (preset_idx < 20) {
                                                        memcpy(palette_colors, preset_palettes[preset_idx], sizeof(palette_colors));
                                                        is_eraser = false;
                                                        uiUpdatePickerPosFromActiveColor();
                                                        uiOpenModal(2);
                                                        uiDrawToolbar();
                                                    }
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                    // Prev / Next button -> y = 154..170
                                    else if (prev_y >= 154 && prev_y <= 170) {
                                        // Prev button <- (x = 74..114)
                                        if (prev_x >= 74 && prev_x <= 114) {
                                            preset_page--;
                                            if (preset_page < 0) preset_page = 3;
                                            uiOpenModal(2);
                                        }
                                        // Next button -> (x = 142..182)
                                        else if (prev_x >= 142 && prev_x <= 182) {
                                            preset_page++;
                                            if (preset_page > 3) preset_page = 0;
                                            uiOpenModal(2);
                                        }
                                    }
                                } else {
                                    // MIS PALETAS:
                                    // Slots buttons: slot i (0..4) -> x0 = 12 + i * 48, x1 = x0 + 38, y = 134..152
                                    if (prev_y >= 134 && prev_y <= 152) {
                                        if (prev_x >= 12 && prev_x <= 242) {
                                            for (int i = 0; i < 5; i++) {
                                                int x0 = 12 + i * 48;
                                                int x1 = x0 + 38;
                                                if (prev_x >= x0 && prev_x <= x1) {
                                                    selected_custom_slot = i;
                                                    int global_idx = custom_page * 5 + i;
                                                    memcpy(palette_colors, custom_palettes[global_idx], sizeof(palette_colors));
                                                    is_eraser = false;
                                                    uiUpdatePickerPosFromActiveColor();
                                                    uiOpenModal(2);
                                                    uiDrawToolbar();
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                    // GUARDAR & PAGE NAV buttons: y = 154..170
                                    else if (prev_y >= 154 && prev_y <= 170) {
                                        // GUARDAR button (x = 12..82)
                                        if (prev_x >= 12 && prev_x <= 82) {
                                            int global_idx = custom_page * 5 + selected_custom_slot;
                                            memcpy(custom_palettes[global_idx], palette_colors, sizeof(palette_colors));
                                            netSaveConfig();
                                            uiOpenModal(2);
                                            uiDrawToolbar();
                                        }
                                        // Prev button <- (x = 114..144)
                                        else if (prev_x >= 114 && prev_x <= 144) {
                                            custom_page--;
                                            if (custom_page < 0) custom_page = 9;
                                            int global_idx = custom_page * 5 + selected_custom_slot;
                                            memcpy(palette_colors, custom_palettes[global_idx], sizeof(palette_colors));
                                            uiUpdatePickerPosFromActiveColor();
                                            uiOpenModal(2);
                                            uiDrawToolbar();
                                        }
                                        // Next button -> (x = 174..204)
                                        else if (prev_x >= 174 && prev_x <= 204) {
                                            custom_page++;
                                            if (custom_page > 9) custom_page = 0;
                                            int global_idx = custom_page * 5 + selected_custom_slot;
                                            memcpy(palette_colors, custom_palettes[global_idx], sizeof(palette_colors));
                                            uiUpdatePickerPosFromActiveColor();
                                            uiOpenModal(2);
                                            uiDrawToolbar();
                                        }
                                    }
                                }
                            }
                            option_selected = false;
                        } else if (open_modal == 3) { // BG
                            // Check tab bar (y = 20..32)
                            if (prev_y >= 20 && prev_y <= 32) {
                                if (prev_x >= 8 && prev_x <= 128) {
                                    if (bg_modal_tab != 0) {
                                        bg_modal_tab = 0;
                                        uiOpenModal(3);
                                    }
                                } else if (prev_x > 128 && prev_x <= 247) {
                                    if (bg_modal_tab != 1) {
                                        bg_modal_tab = 1;
                                        uiOpenModal(3);
                                    }
                                }
                            }
                            // Tab 0: PATRONES
                            else if (bg_modal_tab == 0) {
                                // Row 1 (y = 38..78)
                                if (prev_y >= 38 && prev_y <= 78) {
                                    for (int i = 0; i < 4; i++) {
                                        int x0 = 12 + i * 58;
                                        int x1 = x0 + 52;
                                        if (prev_x >= x0 && prev_x <= x1) {
                                            bg_pattern_idx = i;
                                            renderApplyBackgroundPattern(bg_pattern_idx);
                                            renderComposeCanvas();
                                            uiUpdateModalBackup();
                                            uiOpenModal(3);
                                            break;
                                        }
                                    }
                                }
                                // Row 2 (y = 84..124)
                                else if (prev_y >= 84 && prev_y <= 124) {
                                    for (int i = 0; i < 4; i++) {
                                        int x0 = 12 + i * 58;
                                        int x1 = x0 + 52;
                                        if (prev_x >= x0 && prev_x <= x1) {
                                            bg_pattern_idx = 4 + i;
                                            renderApplyBackgroundPattern(bg_pattern_idx);
                                            renderComposeCanvas();
                                            uiUpdateModalBackup();
                                            uiOpenModal(3);
                                            break;
                                        }
                                    }
                                }
                                // Row 3 settings (y = 132..150)
                                else if (prev_y >= 132 && prev_y <= 150) {
                                    if (prev_x >= 12 && prev_x <= 70) {
                                        // COLOR P
                                        bg_color_p_idx = (bg_color_p_idx + 1) % 4;
                                        renderApplyBackgroundPattern(bg_pattern_idx);
                                        renderComposeCanvas();
                                        uiUpdateModalBackup();
                                        uiOpenModal(3);
                                    } else if (prev_x >= 74 && prev_x <= 132) {
                                        // COLOR S
                                        bg_color_s_idx = (bg_color_s_idx + 1) % 4;
                                        renderApplyBackgroundPattern(bg_pattern_idx);
                                        renderComposeCanvas();
                                        uiUpdateModalBackup();
                                        uiOpenModal(3);
                                    } else if (prev_x >= 136 && prev_x <= 194) {
                                        // MODIFICABLE
                                        bg_modifiable = !bg_modifiable;
                                        renderApplyBackgroundPattern(bg_pattern_idx);
                                        renderComposeCanvas();
                                        uiUpdateModalBackup();
                                        uiOpenModal(3);
                                    } else if (prev_x >= 198 && prev_x <= 244) {
                                         angle_target = 1;
                                         uiOpenModal(4);
                                         uiDrawToolbar();
                                    }
                                }
                            }
                            // Tab 1: PERSPECTIVA
                            else if (bg_modal_tab == 1) {
                                // Mode buttons (y = 46..62)
                                if (prev_y >= 46 && prev_y <= 62) {
                                    if (prev_x >= 12 && prev_x <= 53) {
                                         perspective_mode = 0;
                                         renderComposeCanvas();
                                         uiUpdateModalBackup();
                                         uiOpenModal(3);
                                     } else if (prev_x >= 57 && prev_x <= 101) {
                                         perspective_mode = 1;
                                         perspective_points[0][0] = 128;
                                         perspective_points[0][1] = 88;
                                         renderComposeCanvas();
                                         uiUpdateModalBackup();
                                         uiOpenModal(3);
                                     } else if (prev_x >= 105 && prev_x <= 149) {
                                         perspective_mode = 2;
                                         perspective_points[0][0] = 32;
                                         perspective_points[0][1] = 88;
                                         perspective_points[1][0] = 224;
                                         perspective_points[1][1] = 88;
                                         renderComposeCanvas();
                                         uiUpdateModalBackup();
                                         uiOpenModal(3);
                                     } else if (prev_x >= 153 && prev_x <= 197) {
                                         perspective_mode = 3;
                                         perspective_points[0][0] = 32;
                                         perspective_points[0][1] = 64;
                                         perspective_points[1][0] = 224;
                                         perspective_points[1][1] = 64;
                                         perspective_points[2][0] = 128;
                                         perspective_points[2][1] = 150;
                                         renderComposeCanvas();
                                         uiUpdateModalBackup();
                                         uiOpenModal(3);
                                     } else if (prev_x >= 201 && prev_x <= 244) {
                                         perspective_mode = 4;
                                         perspective_points[0][0] = 20;
                                         perspective_points[0][1] = 88;
                                         perspective_points[1][0] = 236;
                                         perspective_points[1][1] = 88;
                                         perspective_points[2][0] = 128;
                                         perspective_points[2][1] = 12;
                                         perspective_points[3][0] = 128;
                                         perspective_points[3][1] = 164;
                                         renderComposeCanvas();
                                         uiUpdateModalBackup();
                                         uiOpenModal(3);
                                     }
                                }
                                 // Placing points button (y = 68..84)
                                 else if (prev_y >= 68 && prev_y <= 84) {
                                     if (prev_x >= 12 && prev_x <= 244) {
                                         if (perspective_mode > 0) {
                                             uiCloseModal();
                                             placing_vanishing_points = true;
                                             placing_single_vanishing_point = false;
                                             placing_point_idx = 0;
                                             ignore_touch_until_release = true;
                                         }
                                     }
                                 }
                                 // Individual point edit buttons (y = 90..106)
                                 else if (prev_y >= 90 && prev_y <= 106) {
                                     if (prev_x >= 12 && prev_x <= 65) {
                                         if (perspective_mode >= 1) {
                                             uiCloseModal();
                                             placing_vanishing_points = true;
                                             placing_single_vanishing_point = true;
                                             placing_point_idx = 0;
                                             ignore_touch_until_release = true;
                                         }
                                     } else if (prev_x >= 71 && prev_x <= 124) {
                                         if (perspective_mode >= 2) {
                                             uiCloseModal();
                                             placing_vanishing_points = true;
                                             placing_single_vanishing_point = true;
                                             placing_point_idx = 1;
                                             ignore_touch_until_release = true;
                                         }
                                     } else if (prev_x >= 130 && prev_x <= 183) {
                                         if (perspective_mode >= 3) {
                                             uiCloseModal();
                                             placing_vanishing_points = true;
                                             placing_single_vanishing_point = true;
                                             placing_point_idx = 2;
                                             ignore_touch_until_release = true;
                                         }
                                     } else if (prev_x >= 189 && prev_x <= 244) {
                                         if (perspective_mode >= 4) {
                                             uiCloseModal();
                                             placing_vanishing_points = true;
                                             placing_single_vanishing_point = true;
                                             placing_point_idx = 3;
                                             ignore_touch_until_release = true;
                                         }
                                     }
                                 }
                                // Density button (y = 112..128)
                                else if (prev_y >= 112 && prev_y <= 128) {
                                    if (prev_x >= 12 && prev_x <= 244) {
                                        if (perspective_step == 64) perspective_step = 32;
                                        else if (perspective_step == 32) perspective_step = 16;
                                        else if (perspective_step == 16) perspective_step = 12;
                                        else if (perspective_step == 12) perspective_step = 8;
                                        else perspective_step = 64;
                                        
                                        renderComposeCanvas();
                                        uiUpdateModalBackup();
                                        uiOpenModal(3);
                                    }
                                }
                            }
                            option_selected = false;
                        }
                        
                        if (option_selected) {
                            uiCloseModal();
                            uiDrawToolbar();
                        }
                    } else {
                        // Tapped outside the modal: close it!
                        uiCloseModal();
                        uiDrawToolbar();
                    }
                    was_touching = false;
                }
            }
            return;
        }

        // Toggle toolbar visibility with D-pad Up or Down
        if (keys_down & (KEY_UP | KEY_DOWN)) {
            uiCloseModal(); // Close open modal if any
            toolbar_hidden = !toolbar_hidden;
            if (toolbar_hidden) {
                // Compose background pattern region on the bottom 16 pixels
                renderComposeCanvas();
            } else {
                // Restore toolbar
                uiDrawToolbar();
            }
            renderUpdatePreview();
        }

        printf("\x1b[12;0H"); 
        printf("Raw Keys: %08lX      \n", (unsigned long)keys_held);
        
        if (ignore_touch_until_release) {
            if (!(keys_held & KEY_TOUCH)) {
                ignore_touch_until_release = false;
            }
        }
        
        touchPosition touch;
        if (!ignore_touch_until_release && inputGetTouch(&touch)) {
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
                if (!was_touching) {
                    prev_x = touch.px;
                    prev_y = touch.py;
                }
                was_touching = true;
            }
        } else {
            if (was_touching && touch_started_in_toolbar && prev_y >= 176) {
                if (prev_x >= 0 && prev_x < 42) {
                    uiOpenModal(0);
                    uiDrawToolbar();
                } else if (prev_x >= 42 && prev_x < 84) {
                    uiOpenModal(1);
                    uiDrawToolbar();
                } else if (prev_x >= 84 && prev_x < 126) {
                    uiOpenModal(2);
                    uiDrawToolbar();
                } else if (prev_x >= 126 && prev_x < 168) {
                    uiOpenModal(3);
                    uiDrawToolbar();
                } else if (prev_x >= 168 && prev_x < 212) {
                    enterWizardState();
                } else if (prev_x >= 212 && prev_x <= 255) {
                    current_state = STATE_UPLOAD;
                }
            }
            was_touching = false;
            touch_started_in_toolbar = false;
        }
    } 
    else if (current_state == STATE_WIZARD) {
        touchPosition touch;
        if (inputGetTouch(&touch)) {
            if (!was_touching) {
                prev_x = touch.px;
                prev_y = touch.py;
            }
            was_touching = true;
        } else {
            if (was_touching) {
                // 1. Check if they touched the tabs: y = 42 to 62
                if (prev_y >= 42 && prev_y <= 62) {
                    if (prev_x >= 10 && prev_x <= 70) {
                        changeWizardStep(0);
                    } else if (prev_x >= 76 && prev_x <= 136) {
                        changeWizardStep(1);
                    } else if (prev_x >= 142 && prev_x <= 202) {
                        changeWizardStep(2);
                    }
                }
                // 2. Check if they touched the keyboard: y = 96 to 182
                else if (prev_y >= 96 && prev_y <= 182) {
                    bool shift_toggled = false;
                    bool caps_toggled = false;
                    bool enter_pressed = false;
                    bool backspace_pressed = false;
                    
                    char key = uiHandleKeyboardTouch(prev_x, prev_y, &shift_toggled, &caps_toggled, &enter_pressed, &backspace_pressed);
                    
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
                            was_touching = false;
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
                was_touching = false;
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
            was_touching = false;
            return;
        }

        if (keys_down & KEY_B) {
            exitWizardState(true);
            was_touching = false;
            return;
        }
    } 
    else if (current_state == STATE_UPLOAD) {
        runUpload();
    }
}
