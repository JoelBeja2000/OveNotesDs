#ifndef RENDER_H
#define RENDER_H

#include <nds.h>
#include <stdint.h>
#include <stdbool.h>
#include "../ui/ui_types.h"

extern AppState g_app_state;

// Redefine RGB15 to include the alpha bit (bit 15) so that pixels render as opaque in BgType_Bmp16
#undef RGB15
#define RGB15(r,g,b) (BIT(15) | ((r)|((g)<<5)|((b)<<10)))

// Framebuffer pointers (exported)
extern uint16_t* canvas_buffer;
extern uint16_t* preview_buffer;
extern uint16_t* wizard_buffer;
extern uint16_t* drawing_buffer;
extern uint16_t* composite_buffer;
#define MAX_LAYERS 8
extern uint16_t* layers[MAX_LAYERS];

#ifndef NO_UI_COMPAT_MACROS
#include "../ui/ui_compat.h"
#endif

typedef enum {
    UNDO_STROKE,
    UNDO_STRUCTURE
} UndoType;

typedef struct {
    UndoType type;
    
    // For UNDO_STROKE:
    int stroke_layer_idx;
    uint16_t* stroke_pixels;
    
    // For UNDO_STRUCTURE:
    int struct_layers_count;
    int struct_active_layer_idx;
    bool struct_layers_visible[MAX_LAYERS];
    char struct_layer_names[MAX_LAYERS][16];
    uint8_t struct_layers_opacity[MAX_LAYERS];
    uint16_t* struct_layer_pixels[MAX_LAYERS];
} UndoStep;

#define MAX_UNDO_STEPS 5
extern UndoStep undo_stack[MAX_UNDO_STEPS];
extern UndoStep redo_stack[MAX_UNDO_STEPS];

void renderInitUndoStack(void);
void renderSaveUndoState(void);
void renderSaveUndoStructureState(void);
void renderUndo(void);
void renderRedo(void);
extern int undo_count;
extern int redo_count;

void renderAddLayer(void);
void renderDeleteLayer(int idx);
void renderMergeActiveLayerDown(void);
void renderMergeActiveLayerUp(void);

extern const uint16_t bg_primary_palette[4];
extern const uint16_t bg_secondary_palette[4];



void renderInitCanvas(void);
void renderInitPreview(void);
void renderUpdatePreview(void);
void renderComposeCanvas(void);
uint16_t renderGetComposedPixel(int x, int y);

void renderSetPixel(int x, int y, uint16_t color);
void renderSetCanvasPixel(int x, int y, uint16_t color);

uint16_t blendRGB555_int(uint16_t src, uint16_t dst, int alpha_32);
void renderApplyBackgroundPattern(int pat_index);
void renderDrawBackgroundRegion(int y0, int y1);
void renderFloodFill(int start_x, int start_y, uint16_t fill_color);
void renderOverlayPerspectiveGuides(void);

void renderDrawChar(char c, int x, int y, uint16_t color, uint16_t bg_color);
void renderDrawText(const char* text, int x, int y, uint16_t color, uint16_t bg_color);
void renderDrawCharOnBuffer(uint16_t* buffer, char c, int x, int y, uint16_t color, uint16_t bg_color);
void renderDrawTextOnBuffer(uint16_t* buffer, const char* text, int x, int y, uint16_t color, uint16_t bg_color);
void renderDrawBrushPoint(int xc, int yc, uint16_t color, int size);
void renderDrawEraserPoint(int xc, int yc);
void renderDrawLine(int x0, int y0, int x1, int y1, uint16_t color, int size, bool eraser);

uint16_t hsv_to_rgb15(int h, int s, int v);
void rgb15_to_hsv(uint16_t color, int* h, int* s, int* v);
void renderDrawHSMap(int x0, int y0, int w, int h);
void renderDrawBrightnessSlider(int x0, int y0, int w, int h, int ph, int ps);

void renderDrawRect(int x0, int y0, int x1, int y1, uint16_t color);
void renderDrawRectOutline(int x0, int y0, int x1, int y1, uint16_t color);
void renderDrawLineSimple(int x0, int y0, int x1, int y1, uint16_t color);
void renderDrawCircleOutline(int xc, int yc, int r, uint16_t color);
void renderDrawFilledCircle(int xm, int ym, int r, uint16_t color);
void renderDrawPatternPreview(int x0, int y0, int x1, int y1, int pat_idx, bool selected);
void renderDrawLockIcon(int x, int y, bool locked);

#endif // RENDER_H
