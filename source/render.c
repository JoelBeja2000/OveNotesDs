#include "render.h"
#include "font8x8.h"
#include "ui.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

uint16_t getActualSecondaryColor(void);

uint16_t* canvas_buffer = NULL;
uint16_t* preview_buffer = NULL;
uint16_t* wizard_buffer = NULL;
uint16_t* drawing_buffer = NULL;
uint16_t* composite_buffer = NULL;
uint16_t* layers[MAX_LAYERS] = {NULL};
int layers_count = 1;
int active_layer_idx = 0;
bool layers_visible[MAX_LAYERS] = {false};
bool layers_panel_open = false;
int dragging_layer_idx = -1;
char layer_names[MAX_LAYERS][16] = {
    "Capa 0", "Capa 1", "Capa 2", "Capa 3", "Capa 4", "Capa 5", "Capa 6", "Capa 7"
};
uint8_t layers_opacity[MAX_LAYERS] = {100, 100, 100, 100, 100, 100, 100, 100};

UndoStep undo_stack[MAX_UNDO_STEPS];
UndoStep redo_stack[MAX_UNDO_STEPS];
int undo_count = 0;
int redo_count = 0;

bool toolbar_hidden = false;

bool bg_modifiable = false;
int bg_angle = 0;
int bg_color_p_idx = 0;
int bg_color_s_idx = 0;
int angle_target = 0; // 0 = brush, 1 = background

int perspective_mode = 0;
int perspective_points[4][2] = { {32, 88}, {224, 88}, {128, 40}, {128, 136} };
int perspective_step = 32;
int nib_angle = 0;

const uint16_t bg_primary_palette[4] = {
    RGB15(31, 31, 31), // White
    RGB15(31, 30, 25), // Cream / Sepia
    RGB15(26, 29, 31), // Light Blue
    RGB15(5, 5, 6)     // Dark Charcoal (Dark Mode)
};

const uint16_t bg_secondary_palette[4] = {
    RGB15(24, 24, 24), // Grey
    RGB15(16, 20, 26), // Muted Blue
    RGB15(28, 12, 12), // Red
    RGB15(0, 0, 0)     // Black
};

void renderSetPixel(int x, int y, uint16_t color) {
    if (x >= 0 && x < 256 && y >= 0 && y < 192) {
        canvas_buffer[y * 256 + x] = color;
    }
}

uint16_t blendRGB555_int(uint16_t src, uint16_t dst, int alpha_32) {
    int src_r = src & 0x1F;
    int src_g = (src >> 5) & 0x1F;
    int src_b = (src >> 10) & 0x1F;
    
    int dst_r = dst & 0x1F;
    int dst_g = (dst >> 5) & 0x1F;
    int dst_b = (dst >> 10) & 0x1F;
    
    int r = (src_r * alpha_32 + dst_r * (32 - alpha_32)) >> 5;
    int g = (src_g * alpha_32 + dst_g * (32 - alpha_32)) >> 5;
    int b = (src_b * alpha_32 + dst_b * (32 - alpha_32)) >> 5;
    
    return BIT(15) | r | (g << 5) | (b << 10);
}

uint16_t renderGetComposedPixel(int x, int y) {
    int stop_idx = -1;
    for (int i = layers_count - 1; i >= 0; i--) {
        if (layers[i] != NULL && layers_visible[i]) {
            uint16_t p = layers[i][y * 256 + x];
            if (p != RGB15(31, 31, 31)) {
                if (layers_opacity[i] == 100) {
                    stop_idx = i;
                    break;
                }
            }
        }
    }
    
    uint16_t out_color;
    if (stop_idx != -1) {
        out_color = layers[stop_idx][y * 256 + x];
    } else {
        if (bg_modifiable) {
            out_color = RGB15(31, 31, 31);
        } else {
            uint16_t p_color = bg_primary_palette[bg_color_p_idx];
            uint16_t s_color = getActualSecondaryColor();
            uint16_t red_margin = RGB15(30, 8, 8);
            int rx = x;
            int ry = y;
            if (bg_angle != 0) {
                int cx = x - 128;
                int cy = y - 96;
                float rad = -bg_angle * 3.14159265f / 180.0f;
                int cos_a_fp = (int)(cosf(rad) * 256.0f);
                int sin_a_fp = (int)(sinf(rad) * 256.0f);
                rx = ((cx * cos_a_fp - cy * sin_a_fp) >> 8) + 128;
                ry = ((cx * sin_a_fp + cy * cos_a_fp) >> 8) + 96;
            }
            out_color = p_color;
            if (bg_pattern_idx == 1) {
                if ((ry - 8) % 16 == 0 && (rx - 8) % 16 == 0) {
                    out_color = s_color;
                }
            } else if (bg_pattern_idx == 2) {
                if (ry > 0 && ry % 16 == 0) {
                    out_color = s_color;
                }
            } else if (bg_pattern_idx == 3) {
                if (ry % 16 == 0 || rx % 16 == 0) {
                    out_color = s_color;
                }
            } else if (bg_pattern_idx == 4) {
                if (rx % 16 == 0) {
                    out_color = s_color;
                }
            } else if (bg_pattern_idx == 5) {
                if ((rx + ry * 2) % 32 == 0 || (rx - ry * 2) % 32 == 0) {
                    out_color = s_color;
                }
            } else if (bg_pattern_idx == 6) {
                if (ry > 0 && ry % 24 == 0) {
                    out_color = s_color;
                }
            } else if (bg_pattern_idx == 7) {
                if (ry > 0 && ry % 16 == 0) {
                    out_color = s_color;
                }
                if (rx == 32) {
                    out_color = red_margin;
                }
            }
        }
    }
    
    int start_i = (stop_idx == -1) ? 0 : (stop_idx + 1);
    for (int i = start_i; i < layers_count; i++) {
        if (layers[i] != NULL && layers_visible[i]) {
            uint16_t p = layers[i][y * 256 + x];
            if (p != RGB15(31, 31, 31)) {
                if (layers_opacity[i] > 0) {
                    int alpha_32 = (layers_opacity[i] * 32) / 100;
                    out_color = blendRGB555_int(p, out_color, alpha_32);
                }
            }
        }
    }
    
    return out_color;
}

static void renderComposePixel(int x, int y) {
    if (x < 0 || x >= 256 || y < 0 || y >= 192) return;
    uint16_t composed = renderGetComposedPixel(x, y);
    if (composite_buffer != NULL) {
        composite_buffer[y * 256 + x] = composed;
    }
    if (canvas_buffer == NULL) return;
    int limit_y = toolbar_hidden ? 192 : 176;
    if (y < limit_y) {
        canvas_buffer[y * 256 + x] = composed;
    }
}

void renderSetCanvasPixel(int x, int y, uint16_t color) {
    if (drawing_buffer == NULL) return;
    int max_y = toolbar_hidden ? 192 : 176;
    if (x >= 0 && x < 256 && y >= 0 && y < max_y) {
        if (color == RGB15(31, 31, 31)) {
            // Eraser mode: draw solid white marker in the drawing buffer
            drawing_buffer[y * 256 + x] = color;
        } else {
            // Brush mode: apply active color and drawing modes
            uint16_t brush_color = palette_colors[active_color_idx];
            if (drawing_mode == 0 || drawing_mode >= 7) {
                drawing_buffer[y * 256 + x] = brush_color;
            } else if (drawing_mode == 1) {
                uint16_t current = drawing_buffer[y * 256 + x];
                drawing_buffer[y * 256 + x] = blendRGB555_int(brush_color, current, 12);
            } else if (drawing_mode == 2) {
                if ((x + y) % 2 == 0) {
                    drawing_buffer[y * 256 + x] = brush_color;
                }
            } else if (drawing_mode == 3) {
                if ((x - y) % 4 == 0) {
                    drawing_buffer[y * 256 + x] = brush_color;
                }
            } else if (drawing_mode == 4) {
                if (y % 4 == 0) {
                    drawing_buffer[y * 256 + x] = brush_color;
                }
            } else if (drawing_mode == 5) {
                if (x % 4 == 0) {
                    drawing_buffer[y * 256 + x] = brush_color;
                }
            } else if (drawing_mode == 6) {
                if (x % 4 == 0 || y % 4 == 0) {
                    drawing_buffer[y * 256 + x] = brush_color;
                }
            }
        }
        renderComposePixel(x, y);
    }
}

uint16_t getActualSecondaryColor(void) {
    uint16_t p = bg_primary_palette[bg_color_p_idx];
    uint16_t s = bg_secondary_palette[bg_color_s_idx];
    
    // If primary color is dark (sum of components < 30)
    int p_r = p & 31;
    int p_g = (p >> 5) & 31;
    int p_b = (p >> 10) & 31;
    if (p_r + p_g + p_b < 30) {
        // Brighten secondary color
        int s_r = s & 31;
        int s_g = (s >> 5) & 31;
        int s_b = (s >> 10) & 31;
        if (s_r + s_g + s_b < 20) {
            return RGB15(18, 18, 18);
        }
        int nr = s_r + 15 > 31 ? 31 : s_r + 15;
        int ng = s_g + 15 > 31 ? 31 : s_g + 15;
        int nb = s_b + 15 > 31 ? 31 : s_b + 15;
        return RGB15(nr, ng, nb);
    }
    return s;
}

static inline void writePixel(int x, int y, uint16_t color) {
    if (open_modal == 4 && y >= 90) {
        if (y < 176) {
            modal_backup[(y - 90) * 256 + x] = color;
        }
    } else {
        canvas_buffer[y * 256 + x] = color;
    }
}

static void drawPerspectiveLine(int x0, int y0, int x1, int y1, uint16_t color) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    int limit_y = toolbar_hidden ? 192 : 176;
    int max_x = layers_panel_open ? 144 : 256;

    while (1) {
        if (x0 >= 0 && x0 < max_x && y0 >= 0 && y0 < limit_y) {
            // Only draw if drawing_buffer is white (user has not drawn there)
            if (drawing_buffer[y0 * 256 + x0] == RGB15(31, 31, 31)) {
                writePixel(x0, y0, color);
            }
        }
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

static void drawPerspectiveCurve(int x0, int y0, int cx, int cy, int x1, int y1, uint16_t color) {
    int prev_x = x0;
    int prev_y = y0;
    for (int i = 1; i <= 8; i++) {
        int t = i * 8; // 0..64
        int mt = 64 - t;
        int x = (mt * mt * x0 + 2 * mt * t * cx + t * t * x1) / 4096;
        int y = (mt * mt * y0 + 2 * mt * t * cy + t * t * y1) / 4096;
        
        drawPerspectiveLine(prev_x, prev_y, x, y, color);
        prev_x = x;
        prev_y = y;
    }
}

static void drawPerspectiveCrosshair(int cx, int cy, uint16_t color) {
    int limit_y = toolbar_hidden ? 192 : 176;
    int max_x = layers_panel_open ? 144 : 256;
    for (int dx = -2; dx <= 2; dx++) {
        int x = cx + dx;
        if (x >= 0 && x < max_x && cy >= 0 && cy < limit_y) {
            writePixel(x, cy, color);
        }
    }
    for (int dy = -2; dy <= 2; dy++) {
        int y = cy + dy;
        if (cx >= 0 && cx < max_x && y >= 0 && y < limit_y) {
            writePixel(cx, y, color);
        }
    }
}

void renderComposeCanvas(void) {
    if (composite_buffer == NULL) return;
    
    // 1. Recompose the entire clean composite_buffer from the layers
    for (int y = 0; y < 192; y++) {
        for (int x = 0; x < 256; x++) {
            composite_buffer[y * 256 + x] = renderGetComposedPixel(x, y);
        }
    }
    
    if (canvas_buffer == NULL) return;
    
    swiWaitForVBlank();
    
    int limit_y = toolbar_hidden ? 192 : 176;
    int max_x = layers_panel_open ? 144 : 256;
    
    // 2. Copy the composite_buffer to the canvas_buffer, taking care of modal_backup if active
    for (int y = 0; y < limit_y; y++) {
        uint16_t* dest_row = NULL;
        if (open_modal == 4 && y >= 90) {
            if (y < 176) {
                dest_row = &modal_backup[(y - 90) * 256];
            }
        } else {
            dest_row = &canvas_buffer[y * 256];
        }
        
        if (dest_row != NULL) {
            memcpy(dest_row, &composite_buffer[y * 256], max_x * sizeof(uint16_t));
        }
    }

    renderOverlayPerspectiveGuides();
    
    uiDrawUndoRedoButtons();
    uiDrawLayersOverlay();
}

void renderOverlayPerspectiveGuides(void) {
    if (perspective_mode == 0) return;
    int limit_y = toolbar_hidden ? 192 : 176;
    
    uint16_t s_color = getActualSecondaryColor();
    
    // Draw radiating perspective grid lines
    if (perspective_mode < 4) {
        for (int i = 0; i < perspective_mode; i++) {
            int px = perspective_points[i][0];
            int py = perspective_points[i][1];
            
            // Draw lines to screen borders
            // Top border: (bx, 0)
            for (int bx = 0; bx < 256; bx += perspective_step) {
                drawPerspectiveLine(px, py, bx, 0, s_color);
            }
            // Bottom border: (bx, limit_y - 1)
            for (int bx = 0; bx < 256; bx += perspective_step) {
                drawPerspectiveLine(px, py, bx, limit_y - 1, s_color);
            }
            // Left border: (0, by)
            for (int by = 0; by < limit_y; by += perspective_step) {
                drawPerspectiveLine(px, py, 0, by, s_color);
            }
            // Right border: (255, by)
            for (int by = 0; by < limit_y; by += perspective_step) {
                drawPerspectiveLine(px, py, 255, by, s_color);
            }
        }
    }
    
    // Draw boundary/horizon lines connecting points if mode >= 2
    if (perspective_mode == 2) {
        // Horizon line between P1 and P2
        drawPerspectiveLine(perspective_points[0][0], perspective_points[0][1],
                             perspective_points[1][0], perspective_points[1][1],
                             RGB15(31, 0, 0)); // Red horizon!
    } else if (perspective_mode == 3) {
        // Lines connecting P1-P2, P2-P3, P3-P1
        drawPerspectiveLine(perspective_points[0][0], perspective_points[0][1],
                             perspective_points[1][0], perspective_points[1][1],
                             RGB15(31, 0, 0)); // Red horizon!
        drawPerspectiveLine(perspective_points[1][0], perspective_points[1][1],
                             perspective_points[2][0], perspective_points[2][1],
                             RGB15(0, 0, 31)); // Blue perspective boundary!
        drawPerspectiveLine(perspective_points[2][0], perspective_points[2][1],
                             perspective_points[0][0], perspective_points[0][1],
                             RGB15(0, 0, 31)); // Blue perspective boundary!
    } else if (perspective_mode == 4) {
        // Fisheye grid: curves connecting L-R and T-B
        int lx = perspective_points[0][0];
        int ly = perspective_points[0][1];
        int rx = perspective_points[1][0];
        int ry = perspective_points[1][1];
        int tx = perspective_points[2][0];
        int ty = perspective_points[2][1];
        int bx = perspective_points[3][0];
        int by = perspective_points[3][1];
        
        int cx = (lx + rx) / 2;
        int cy = (ty + by) / 2;
        
        // Left-to-Right curves (horizontal lines in fisheye)
        for (int v = -4; v <= 4; v++) {
            int control_y = cy + v * perspective_step;
            drawPerspectiveCurve(lx, ly, cx, control_y, rx, ry, s_color);
        }
        
        // Top-to-Bottom curves (vertical lines in fisheye)
        for (int h = -5; h <= 5; h++) {
            int control_x = cx + h * perspective_step;
            drawPerspectiveCurve(tx, ty, control_x, cy, bx, by, s_color);
        }
        
        // Draw outer border / horizon connections for visual structure
        drawPerspectiveLine(lx, ly, tx, ty, RGB15(0, 0, 31));
        drawPerspectiveLine(tx, ty, rx, ry, RGB15(0, 0, 31));
        drawPerspectiveLine(rx, ry, bx, by, RGB15(0, 0, 31));
        drawPerspectiveLine(bx, by, lx, ly, RGB15(0, 0, 31));
    }
    
    // Draw crosshairs at vanishing points
    for (int i = 0; i < perspective_mode; i++) {
        drawPerspectiveCrosshair(perspective_points[i][0], perspective_points[i][1], RGB15(31, 0, 0));
    }
}

void renderApplyBackgroundPattern(int pat_index) {
    bg_pattern_idx = pat_index;
    if (bg_modifiable) {
        // Bake background pattern into drawing_buffer
        uint16_t p_color = bg_primary_palette[bg_color_p_idx];
        uint16_t s_color = getActualSecondaryColor();
        uint16_t red_margin = RGB15(30, 8, 8);
        
        int cos_a_fp = 256;
        int sin_a_fp = 0;
        if (bg_angle != 0) {
            float rad = -bg_angle * 3.14159265f / 180.0f;
            cos_a_fp = (int)(cosf(rad) * 256.0f);
            sin_a_fp = (int)(sinf(rad) * 256.0f);
        }
        
        for (int y = 0; y < 192; y++) {
            for (int x = 0; x < 256; x++) {
                uint16_t pixel_color = p_color;
                
                int rx = x;
                int ry = y;
                if (bg_angle != 0) {
                    int cx = x - 128;
                    int cy = y - 96;
                    rx = ((cx * cos_a_fp - cy * sin_a_fp) >> 8) + 128;
                    ry = ((cx * sin_a_fp + cy * cos_a_fp) >> 8) + 96;
                }
                
                if (bg_pattern_idx == 1) {
                    if ((ry - 8) % 16 == 0 && (rx - 8) % 16 == 0) {
                        pixel_color = s_color;
                    }
                } else if (bg_pattern_idx == 2) {
                    if (ry > 0 && ry % 16 == 0) {
                        pixel_color = s_color;
                    }
                } else if (bg_pattern_idx == 3) {
                    if (ry % 16 == 0 || rx % 16 == 0) {
                        pixel_color = s_color;
                    }
                } else if (bg_pattern_idx == 4) {
                    if (rx % 16 == 0) {
                        pixel_color = s_color;
                    }
                } else if (bg_pattern_idx == 5) {
                    if ((rx + ry * 2) % 32 == 0 || (rx - ry * 2) % 32 == 0) {
                        pixel_color = s_color;
                    }
                } else if (bg_pattern_idx == 6) {
                    if (ry > 0 && ry % 24 == 0) {
                        pixel_color = s_color;
                    }
                } else if (bg_pattern_idx == 7) {
                    if (ry > 0 && ry % 16 == 0) {
                        pixel_color = s_color;
                    }
                    if (rx == 32) {
                        pixel_color = red_margin;
                    }
                }
                drawing_buffer[y * 256 + x] = pixel_color;
            }
        }
    } else {
        // Locked: clear drawing_buffer to white (transparent)
        for (int i = 0; i < 256 * 192; i++) {
            drawing_buffer[i] = RGB15(31, 31, 31);
        }
    }
}

void renderFloodFill(int start_x, int start_y, uint16_t fill_color) {
    if (drawing_buffer == NULL) return;
    int limit_y = toolbar_hidden ? 192 : 176;
    if (start_x < 0 || start_x >= 256 || start_y < 0 || start_y >= limit_y) return;
    
    uint16_t target_color = drawing_buffer[start_y * 256 + start_x];
    if (target_color == fill_color) return;
    
    static int q_x[8192];
    static int q_y[8192];
    int head = 0;
    int tail = 0;
    
    q_x[tail] = start_x;
    q_y[tail] = start_y;
    tail = (tail + 1) % 8192;
    
    drawing_buffer[start_y * 256 + start_x] = fill_color;
    renderComposePixel(start_x, start_y);
    
    while (head != tail) {
        int cx = q_x[head];
        int cy = q_y[head];
        head = (head + 1) % 8192;
        
        int dx[4] = {0, 0, -1, 1};
        int dy[4] = {-1, 1, 0, 0};
        
        for (int i = 0; i < 4; i++) {
            int nx = cx + dx[i];
            int ny = cy + dy[i];
            
            if (nx >= 0 && nx < 256 && ny >= 0 && ny < limit_y) {
                if (drawing_buffer[ny * 256 + nx] == target_color) {
                    drawing_buffer[ny * 256 + nx] = fill_color;
                    renderComposePixel(nx, ny);
                    
                    int next_tail = (tail + 1) % 8192;
                    if (next_tail != head) {
                        q_x[tail] = nx;
                        q_y[tail] = ny;
                        tail = next_tail;
                    }
                }
            }
        }
    }
}

void renderDrawChar(char c, int x, int y, uint16_t color, uint16_t bg_color) {
    if ((unsigned char)c < 32) return;
    const unsigned char *glyph = &font5x7[((unsigned char)c) * 5];
    for (int dx = 0; dx < 5; dx++) {
        unsigned char col_data = glyph[dx];
        for (int dy = 0; dy < 8; dy++) {
            if (col_data & (1 << dy)) {
                renderSetPixel(x + dx, y + dy, color);
            } else if (bg_color != 0) {
                renderSetPixel(x + dx, y + dy, bg_color);
            }
        }
    }
}

void renderDrawText(const char* text, int x, int y, uint16_t color, uint16_t bg_color) {
    while (*text) {
        renderDrawChar(*text, x, y, color, bg_color);
        x += 6;
        text++;
    }
}

void renderDrawBrushPoint(int xc, int yc, uint16_t color, int size) {
    if (drawing_mode == 7) { // Calligraphy Stub Pen
        float rad = nib_angle * 3.14159265f / 180.0f;
        float cos_a = cosf(rad);
        float sin_a = sinf(rad);
        int w_half = size;
        for (int t = -w_half; t <= w_half; t++) {
            int px = xc + (int)(cos_a * t);
            int py = yc + (int)(sin_a * t);
            renderSetCanvasPixel(px, py, color);
        }
    } else if (drawing_mode == 10) { // Estilógrafo Técnico
        int radius = size / 2;
        if (radius == 0) {
            renderSetCanvasPixel(xc, yc, color);
        } else {
            for (int dy = -radius; dy <= radius; dy++) {
                for (int dx = -radius; dx <= radius; dx++) {
                    if (dx*dx + dy*dy <= radius*radius) {
                        renderSetCanvasPixel(xc + dx, yc + dy, color);
                    }
                }
            }
        }
    } else {
        if (size == 1) {
            renderSetCanvasPixel(xc, yc, color);
        } else if (size <= 3) {
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    if (dx*dx + dy*dy <= 2) {
                        renderSetCanvasPixel(xc + dx, yc + dy, color);
                    }
                }
            }
        } else {
            for (int dy = -2; dy <= 2; dy++) {
                for (int dx = -2; dx <= 2; dx++) {
                    if (dx*dx + dy*dy <= 5) {
                        renderSetCanvasPixel(xc + dx, yc + dy, color);
                    }
                }
            }
        }
    }
}

void renderDrawEraserPoint(int xc, int yc) {
    uint16_t color = RGB15(31, 31, 31);
    int radius = eraser_size / 2;
    if (radius == 0) {
        renderSetCanvasPixel(xc, yc, color);
    } else {
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                if (dx*dx + dy*dy <= radius*radius) {
                    renderSetCanvasPixel(xc + dx, yc + dy, color);
                }
            }
        }
    }
}

void renderDrawLine(int x0, int y0, int x1, int y1, uint16_t color, int size, bool eraser) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    int current_size = size;
    if (!eraser) {
        if (drawing_mode == 8) { // Pluma Flexible
            float dist = sqrtf((x1 - x0)*(x1 - x0) + (y1 - y0)*(y1 - y0));
            if (dist <= 2.0f) {
                current_size = size + 2;
            } else if (dist >= 12.0f) {
                current_size = 1;
            } else {
                current_size = (size + 2) - (int)((dist - 2.0f) * (size + 1) / 10.0f);
                if (current_size < 1) current_size = 1;
            }
        } else if (drawing_mode == 9) { // Fude
            if (x0 != x1 || y0 != y1) {
                float stroke_angle = atan2f(y1 - y0, x1 - x0);
                float rad_diff = stroke_angle - (nib_angle * 3.14159265f / 180.0f);
                float factor = fabsf(sinf(rad_diff));
                current_size = 1 + (int)(factor * size);
            } else {
                current_size = 1;
            }
        }
    }

    while (1) {
        if (eraser) {
            renderDrawEraserPoint(x0, y0);
        } else {
            renderDrawBrushPoint(x0, y0, color, current_size);
        }

        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void renderInitCanvas(void) {
    for (int i = 0; i < MAX_LAYERS; i++) {
        if (layers[i] != NULL) {
            free(layers[i]);
            layers[i] = NULL;
        }
        layers_visible[i] = false;
        sprintf(layer_names[i], "Capa %d", i);
        layers_opacity[i] = 100;
    }
    
    if (composite_buffer != NULL) {
        free(composite_buffer);
        composite_buffer = NULL;
    }
    composite_buffer = (uint16_t*)malloc(256 * 192 * sizeof(uint16_t));
    if (composite_buffer != NULL) {
        for (int i = 0; i < 256 * 192; i++) {
            composite_buffer[i] = RGB15(31, 31, 31);
        }
    }

    layers[0] = (uint16_t*)malloc(256 * 192 * sizeof(uint16_t));
    if (layers[0] != NULL) {
        for (int i = 0; i < 256 * 192; i++) {
            layers[0][i] = RGB15(31, 31, 31);
        }
    }
    
    layers_count = 1;
    active_layer_idx = 0;
    layers_visible[0] = true;
    drawing_buffer = layers[0];
    
    renderInitUndoStack();
    
    renderComposeCanvas();
    uiDrawToolbar();
}

void renderAddLayer(void) {
    if (layers_count >= MAX_LAYERS) return;
    
    renderSaveUndoStructureState();
    
    uint16_t* new_layer = (uint16_t*)malloc(256 * 192 * sizeof(uint16_t));
    if (new_layer == NULL) return;
    
    for (int i = 0; i < 256 * 192; i++) {
        new_layer[i] = RGB15(31, 31, 31);
    }
    
    layers[layers_count] = new_layer;
    layers_visible[layers_count] = true;
    sprintf(layer_names[layers_count], "Capa %d", layers_count);
    layers_opacity[layers_count] = 100;
    active_layer_idx = layers_count;
    drawing_buffer = layers[active_layer_idx];
    layers_count++;
    
    renderComposeCanvas();
}

static void renderDeleteLayerInternal(int idx) {
    if (layers[idx] != NULL) {
        free(layers[idx]);
        layers[idx] = NULL;
    }
    
    for (int i = idx; i < layers_count - 1; i++) {
        layers[i] = layers[i + 1];
        layers_visible[i] = layers_visible[i + 1];
        strcpy(layer_names[i], layer_names[i + 1]);
        layers_opacity[i] = layers_opacity[i + 1];
    }
    layers[layers_count - 1] = NULL;
    layers_visible[layers_count - 1] = false;
    sprintf(layer_names[layers_count - 1], "Capa %d", layers_count - 1);
    layers_opacity[layers_count - 1] = 100;
    layers_count--;
    
    if (active_layer_idx >= layers_count) {
        active_layer_idx = layers_count - 1;
    }
    drawing_buffer = layers[active_layer_idx];
}

void renderDeleteLayer(int idx) {
    if (idx < 0 || idx >= layers_count) return;
    if (layers_count <= 1) return;
    
    renderSaveUndoStructureState();
    renderDeleteLayerInternal(idx);
    
    renderComposeCanvas();
}

void renderMergeActiveLayerDown(void) {
    if (active_layer_idx <= 0 || active_layer_idx >= layers_count) return;
    
    renderSaveUndoStructureState();
    
    int dst_idx = active_layer_idx - 1;
    int src_idx = active_layer_idx;
    
    if (layers[dst_idx] != NULL && layers[src_idx] != NULL) {
        for (int i = 0; i < 256 * 192; i++) {
            uint16_t src_pixel = layers[src_idx][i];
            if (src_pixel != RGB15(31, 31, 31)) {
                layers[dst_idx][i] = src_pixel;
            }
        }
    }
    
    active_layer_idx = dst_idx;
    renderDeleteLayerInternal(src_idx);
    
    renderComposeCanvas();
}

void renderMergeActiveLayerUp(void) {
    if (active_layer_idx < 0 || active_layer_idx >= layers_count - 1) return;
    
    renderSaveUndoStructureState();
    
    int dst_idx = active_layer_idx + 1;
    int src_idx = active_layer_idx;
    
    if (layers[dst_idx] != NULL && layers[src_idx] != NULL) {
        for (int i = 0; i < 256 * 192; i++) {
            uint16_t src_pixel = layers[src_idx][i];
            if (src_pixel != RGB15(31, 31, 31)) {
                layers[dst_idx][i] = src_pixel;
            }
        }
    }
    
    renderDeleteLayerInternal(src_idx);
    
    renderComposeCanvas();
}

void renderInitPreview(void) {
    if (preview_buffer == NULL) return;
    uint16_t border_color = RGB15(31, 31, 31);
    for (int i = 0; i < 256 * 192; i++) {
        preview_buffer[i] = border_color;
    }
}

void renderUpdatePreview(void) {
    if (preview_buffer == NULL || composite_buffer == NULL) return;
    memcpy(preview_buffer, composite_buffer, 256 * 192 * sizeof(uint16_t));
}

static void clearUndoStep(UndoStep* step) {
    if (step->type == UNDO_STRUCTURE) {
        for (int l = 0; l < MAX_LAYERS; l++) {
            if (step->struct_layer_pixels[l] != NULL) {
                free(step->struct_layer_pixels[l]);
                step->struct_layer_pixels[l] = NULL;
            }
        }
    }
    step->type = UNDO_STROKE;
}

void renderInitUndoStack(void) {
    for (int i = 0; i < MAX_UNDO_STEPS; i++) {
        clearUndoStep(&undo_stack[i]);
        clearUndoStep(&redo_stack[i]);
        
        if (undo_stack[i].stroke_pixels == NULL) {
            undo_stack[i].stroke_pixels = malloc(256 * 192 * sizeof(uint16_t));
        }
        for (int l = 0; l < MAX_LAYERS; l++) {
            undo_stack[i].struct_layer_pixels[l] = NULL;
        }

        if (redo_stack[i].stroke_pixels == NULL) {
            redo_stack[i].stroke_pixels = malloc(256 * 192 * sizeof(uint16_t));
        }
        for (int l = 0; l < MAX_LAYERS; l++) {
            redo_stack[i].struct_layer_pixels[l] = NULL;
        }
    }
    undo_count = 0;
    redo_count = 0;
}

void renderSaveUndoState(void) {
    if (drawing_buffer == NULL) return;
    
    // Clear redo stack on new action
    for (int i = 0; i < redo_count; i++) {
        clearUndoStep(&redo_stack[i]);
    }
    redo_count = 0;
    
    // Push current to undo stack
    if (undo_count == MAX_UNDO_STEPS) {
        clearUndoStep(&undo_stack[0]);
        uint16_t* oldest_stroke_pixels = undo_stack[0].stroke_pixels;
        
        for (int i = 0; i < MAX_UNDO_STEPS - 1; i++) {
            undo_stack[i] = undo_stack[i + 1];
        }
        
        undo_stack[MAX_UNDO_STEPS - 1].type = UNDO_STROKE;
        undo_stack[MAX_UNDO_STEPS - 1].stroke_pixels = oldest_stroke_pixels;
        for (int l = 0; l < MAX_LAYERS; l++) {
            undo_stack[MAX_UNDO_STEPS - 1].struct_layer_pixels[l] = NULL;
        }
        
        undo_count = MAX_UNDO_STEPS - 1;
    }
    
    UndoStep* step = &undo_stack[undo_count];
    clearUndoStep(step);
    step->type = UNDO_STROKE;
    step->stroke_layer_idx = active_layer_idx;
    memcpy(step->stroke_pixels, drawing_buffer, 256 * 192 * sizeof(uint16_t));
    
    undo_count++;
}

void renderSaveUndoStructureState(void) {
    // Clear redo stack on new action
    for (int i = 0; i < redo_count; i++) {
        clearUndoStep(&redo_stack[i]);
    }
    redo_count = 0;
    
    // Push current to undo stack
    if (undo_count == MAX_UNDO_STEPS) {
        clearUndoStep(&undo_stack[0]);
        uint16_t* oldest_stroke_pixels = undo_stack[0].stroke_pixels;
        
        for (int i = 0; i < MAX_UNDO_STEPS - 1; i++) {
            undo_stack[i] = undo_stack[i + 1];
        }
        
        undo_stack[MAX_UNDO_STEPS - 1].type = UNDO_STROKE;
        undo_stack[MAX_UNDO_STEPS - 1].stroke_pixels = oldest_stroke_pixels;
        for (int l = 0; l < MAX_LAYERS; l++) {
            undo_stack[MAX_UNDO_STEPS - 1].struct_layer_pixels[l] = NULL;
        }
        
        undo_count = MAX_UNDO_STEPS - 1;
    }
    
    UndoStep* step = &undo_stack[undo_count];
    clearUndoStep(step);
    
    step->type = UNDO_STRUCTURE;
    step->struct_layers_count = layers_count;
    step->struct_active_layer_idx = active_layer_idx;
    for (int l = 0; l < MAX_LAYERS; l++) {
        step->struct_layers_visible[l] = layers_visible[l];
        strcpy(step->struct_layer_names[l], layer_names[l]);
        step->struct_layers_opacity[l] = layers_opacity[l];
        
        if (layers[l] != NULL) {
            step->struct_layer_pixels[l] = malloc(256 * 192 * sizeof(uint16_t));
            if (step->struct_layer_pixels[l] != NULL) {
                memcpy(step->struct_layer_pixels[l], layers[l], 256 * 192 * sizeof(uint16_t));
            }
        } else {
            step->struct_layer_pixels[l] = NULL;
        }
    }
    
    undo_count++;
}

void renderUndo(void) {
    if (undo_count <= 0) return;
    
    // Push current to redo stack
    if (redo_count == MAX_UNDO_STEPS) {
        clearUndoStep(&redo_stack[0]);
        uint16_t* oldest_stroke_pixels = redo_stack[0].stroke_pixels;
        
        for (int i = 0; i < MAX_UNDO_STEPS - 1; i++) {
            redo_stack[i] = redo_stack[i + 1];
        }
        
        redo_stack[MAX_UNDO_STEPS - 1].type = UNDO_STROKE;
        redo_stack[MAX_UNDO_STEPS - 1].stroke_pixels = oldest_stroke_pixels;
        for (int l = 0; l < MAX_LAYERS; l++) {
            redo_stack[MAX_UNDO_STEPS - 1].struct_layer_pixels[l] = NULL;
        }
        
        redo_count = MAX_UNDO_STEPS - 1;
    }
    
    UndoStep* u_step = &undo_stack[undo_count - 1];
    UndoStep* r_step = &redo_stack[redo_count];
    clearUndoStep(r_step);
    
    if (u_step->type == UNDO_STROKE) {
        r_step->type = UNDO_STROKE;
        r_step->stroke_layer_idx = u_step->stroke_layer_idx;
        int target_layer = u_step->stroke_layer_idx;
        if (target_layer < layers_count && layers[target_layer] != NULL) {
            memcpy(r_step->stroke_pixels, layers[target_layer], 256 * 192 * sizeof(uint16_t));
        }
        
        // Restore stroke from undo
        if (target_layer < layers_count && layers[target_layer] != NULL) {
            memcpy(layers[target_layer], u_step->stroke_pixels, 256 * 192 * sizeof(uint16_t));
        }
        active_layer_idx = target_layer;
        drawing_buffer = layers[active_layer_idx];
    } else {
        // Restore structure: Save current to redo first
        r_step->type = UNDO_STRUCTURE;
        r_step->struct_layers_count = layers_count;
        r_step->struct_active_layer_idx = active_layer_idx;
        for (int l = 0; l < MAX_LAYERS; l++) {
            r_step->struct_layers_visible[l] = layers_visible[l];
            strcpy(r_step->struct_layer_names[l], layer_names[l]);
            r_step->struct_layers_opacity[l] = layers_opacity[l];
            
            if (layers[l] != NULL) {
                r_step->struct_layer_pixels[l] = malloc(256 * 192 * sizeof(uint16_t));
                if (r_step->struct_layer_pixels[l] != NULL) {
                    memcpy(r_step->struct_layer_pixels[l], layers[l], 256 * 192 * sizeof(uint16_t));
                }
            } else {
                r_step->struct_layer_pixels[l] = NULL;
            }
        }
        
        // Restore structure from undo
        for (int l = 0; l < MAX_LAYERS; l++) {
            if (layers[l] != NULL) {
                free(layers[l]);
                layers[l] = NULL;
            }
        }
        
        layers_count = u_step->struct_layers_count;
        active_layer_idx = u_step->struct_active_layer_idx;
        for (int l = 0; l < MAX_LAYERS; l++) {
            layers_visible[l] = u_step->struct_layers_visible[l];
            strcpy(layer_names[l], u_step->struct_layer_names[l]);
            layers_opacity[l] = u_step->struct_layers_opacity[l];
            
            if (u_step->struct_layer_pixels[l] != NULL) {
                layers[l] = malloc(256 * 192 * sizeof(uint16_t));
                if (layers[l] != NULL) {
                    memcpy(layers[l], u_step->struct_layer_pixels[l], 256 * 192 * sizeof(uint16_t));
                }
            } else {
                layers[l] = NULL;
            }
        }
        drawing_buffer = layers[active_layer_idx];
    }
    
    redo_count++;
    
    clearUndoStep(u_step);
    undo_count--;
    
    renderComposeCanvas();
    renderUpdatePreview();
}

void renderRedo(void) {
    if (redo_count <= 0) return;
    
    // Push current to undo stack
    if (undo_count == MAX_UNDO_STEPS) {
        clearUndoStep(&undo_stack[0]);
        uint16_t* oldest_stroke_pixels = undo_stack[0].stroke_pixels;
        
        for (int i = 0; i < MAX_UNDO_STEPS - 1; i++) {
            undo_stack[i] = undo_stack[i + 1];
        }
        
        undo_stack[MAX_UNDO_STEPS - 1].type = UNDO_STROKE;
        undo_stack[MAX_UNDO_STEPS - 1].stroke_pixels = oldest_stroke_pixels;
        for (int l = 0; l < MAX_LAYERS; l++) {
            undo_stack[MAX_UNDO_STEPS - 1].struct_layer_pixels[l] = NULL;
        }
        
        undo_count = MAX_UNDO_STEPS - 1;
    }
    
    UndoStep* r_step = &redo_stack[redo_count - 1];
    UndoStep* u_step = &undo_stack[undo_count];
    clearUndoStep(u_step);
    
    if (r_step->type == UNDO_STROKE) {
        u_step->type = UNDO_STROKE;
        u_step->stroke_layer_idx = r_step->stroke_layer_idx;
        int target_layer = r_step->stroke_layer_idx;
        if (target_layer < layers_count && layers[target_layer] != NULL) {
            memcpy(u_step->stroke_pixels, layers[target_layer], 256 * 192 * sizeof(uint16_t));
        }
        
        // Restore stroke from redo
        if (target_layer < layers_count && layers[target_layer] != NULL) {
            memcpy(layers[target_layer], r_step->stroke_pixels, 256 * 192 * sizeof(uint16_t));
        }
        active_layer_idx = target_layer;
        drawing_buffer = layers[active_layer_idx];
    } else {
        // Redoing structure: save current structure to undo first
        u_step->type = UNDO_STRUCTURE;
        u_step->struct_layers_count = layers_count;
        u_step->struct_active_layer_idx = active_layer_idx;
        for (int l = 0; l < MAX_LAYERS; l++) {
            u_step->struct_layers_visible[l] = layers_visible[l];
            strcpy(u_step->struct_layer_names[l], layer_names[l]);
            u_step->struct_layers_opacity[l] = layers_opacity[l];
            
            if (layers[l] != NULL) {
                u_step->struct_layer_pixels[l] = malloc(256 * 192 * sizeof(uint16_t));
                if (u_step->struct_layer_pixels[l] != NULL) {
                    memcpy(u_step->struct_layer_pixels[l], layers[l], 256 * 192 * sizeof(uint16_t));
                }
            } else {
                u_step->struct_layer_pixels[l] = NULL;
            }
        }
        
        // Restore structure from redo
        for (int l = 0; l < MAX_LAYERS; l++) {
            if (layers[l] != NULL) {
                free(layers[l]);
                layers[l] = NULL;
            }
        }
        
        layers_count = r_step->struct_layers_count;
        active_layer_idx = r_step->struct_active_layer_idx;
        for (int l = 0; l < MAX_LAYERS; l++) {
            layers_visible[l] = r_step->struct_layers_visible[l];
            strcpy(layer_names[l], r_step->struct_layer_names[l]);
            layers_opacity[l] = r_step->struct_layers_opacity[l];
            
            if (r_step->struct_layer_pixels[l] != NULL) {
                layers[l] = malloc(256 * 192 * sizeof(uint16_t));
                if (layers[l] != NULL) {
                    memcpy(layers[l], r_step->struct_layer_pixels[l], 256 * 192 * sizeof(uint16_t));
                }
            } else {
                layers[l] = NULL;
            }
        }
        drawing_buffer = layers[active_layer_idx];
    }
    
    undo_count++;
    
    clearUndoStep(r_step);
    redo_count--;
    
    renderComposeCanvas();
    renderUpdatePreview();
}
