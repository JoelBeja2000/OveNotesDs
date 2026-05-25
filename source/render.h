#ifndef RENDER_H
#define RENDER_H

#include <nds.h>
#include <stdint.h>
#include <stdbool.h>

// Redefine RGB15 to include the alpha bit (bit 15) so that pixels render as opaque in BgType_Bmp16
#undef RGB15
#define RGB15(r,g,b) (BIT(15) | ((r)|((g)<<5)|((b)<<10)))

// Framebuffer pointers (exported)
extern uint16_t* canvas_buffer;
extern uint16_t* preview_buffer;
extern uint16_t* wizard_buffer;
extern uint16_t* drawing_buffer;
extern bool toolbar_hidden;

extern bool bg_modifiable;
extern int bg_angle;
extern int angle_target;
extern int bg_color_p_idx;
extern int bg_color_s_idx;
extern const uint16_t bg_primary_palette[4];
extern const uint16_t bg_secondary_palette[4];

extern int perspective_mode;
extern int perspective_points[4][2];
extern int perspective_step;
extern int nib_angle;

void renderInitCanvas(void);
void renderInitPreview(void);
void renderUpdatePreview(void);
void renderComposeCanvas(void);

void renderSetPixel(int x, int y, uint16_t color);
void renderSetCanvasPixel(int x, int y, uint16_t color);

uint16_t blendRGB555_int(uint16_t src, uint16_t dst, int alpha_32);
void renderApplyBackgroundPattern(int pat_index);
void renderDrawBackgroundRegion(int y0, int y1);
void renderFloodFill(int start_x, int start_y, uint16_t fill_color);

void renderDrawChar(char c, int x, int y, uint16_t color, uint16_t bg_color);
void renderDrawText(const char* text, int x, int y, uint16_t color, uint16_t bg_color);
void renderDrawBrushPoint(int xc, int yc, uint16_t color, int size);
void renderDrawEraserPoint(int xc, int yc);
void renderDrawLine(int x0, int y0, int x1, int y1, uint16_t color, int size, bool eraser);

#endif // RENDER_H
