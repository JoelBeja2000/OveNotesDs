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
uint16_t* layer1_buffer = NULL;
uint16_t* layer2_buffer = NULL;
int active_layer = 1;
bool layer1_visible = true;
bool layer2_visible = true;
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
    uint16_t p1 = RGB15(31, 31, 31);
    uint16_t p2 = RGB15(31, 31, 31);
    
    if (layer1_buffer != NULL) p1 = layer1_buffer[y * 256 + x];
    if (layer2_buffer != NULL) p2 = layer2_buffer[y * 256 + x];
    
    if (layer1_buffer == NULL && layer2_buffer == NULL && drawing_buffer != NULL) {
        p1 = drawing_buffer[y * 256 + x];
    }
    
    bool pixel_drawn = false;
    uint16_t out_color = RGB15(31, 31, 31);
    
    if (layer2_visible && p2 != RGB15(31, 31, 31)) {
        out_color = p2;
        pixel_drawn = true;
    } else if (layer1_visible && p1 != RGB15(31, 31, 31)) {
        out_color = p1;
        pixel_drawn = true;
    }
    
    if (!pixel_drawn) {
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
    return out_color;
}

static void renderComposePixel(int x, int y) {
    if (canvas_buffer == NULL) return;
    int limit_y = toolbar_hidden ? 192 : 176;
    if (x < 0 || x >= 256 || y < 0 || y >= limit_y) return;

    canvas_buffer[y * 256 + x] = renderGetComposedPixel(x, y);
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

    while (1) {
        if (x0 >= 0 && x0 < 256 && y0 >= 0 && y0 < limit_y) {
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
    for (int dx = -2; dx <= 2; dx++) {
        int x = cx + dx;
        if (x >= 0 && x < 256 && cy >= 0 && cy < limit_y) {
            writePixel(x, cy, color);
        }
    }
    for (int dy = -2; dy <= 2; dy++) {
        int y = cy + dy;
        if (cx >= 0 && cx < 256 && y >= 0 && y < limit_y) {
            writePixel(cx, y, color);
        }
    }
}

void renderComposeCanvas(void) {
    if (canvas_buffer == NULL) return;
    
    int limit_y = toolbar_hidden ? 192 : 176;
    
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
            for (int x = 0; x < 256; x++) {
                dest_row[x] = renderGetComposedPixel(x, y);
            }
        }
    }

    // Overlay Perspective guides if enabled
    if (perspective_mode > 0) {
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
    if (layer1_buffer != NULL) {
        for (int i = 0; i < 256 * 192; i++) {
            layer1_buffer[i] = RGB15(31, 31, 31);
        }
    }
    if (layer2_buffer != NULL) {
        for (int i = 0; i < 256 * 192; i++) {
            layer2_buffer[i] = RGB15(31, 31, 31);
        }
    }
    renderComposeCanvas();
    uiDrawToolbar();
}

void renderInitPreview(void) {
    uint16_t border_color = RGB15(10, 10, 10);
    for (int i = 0; i < 128 * 128; i++) {
        preview_buffer[i] = border_color;
    }
}

void renderUpdatePreview(void) {
    for (int y = 0; y < 88; y++) {
        for (int x = 0; x < 128; x++) {
            uint16_t p = canvas_buffer[(y * 2) * 256 + (x * 2)];
            preview_buffer[(y + 20) * 128 + x] = p;
        }
    }
}
