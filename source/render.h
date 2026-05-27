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
extern uint16_t* composite_buffer;
#define MAX_LAYERS 8
extern uint16_t* layers[MAX_LAYERS];
extern int layers_count;
extern int active_layer_idx;
extern bool layers_visible[MAX_LAYERS];
extern bool layers_panel_open;
extern int dragging_layer_idx;
extern char layer_names[MAX_LAYERS][16];
extern uint8_t layers_opacity[MAX_LAYERS];

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
void renderDrawBrushPoint(int xc, int yc, uint16_t color, int size);
void renderDrawEraserPoint(int xc, int yc);
void renderDrawLine(int x0, int y0, int x1, int y1, uint16_t color, int size, bool eraser);

#endif // RENDER_H
