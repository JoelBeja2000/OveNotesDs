#ifndef UI_TYPES_H
#define UI_TYPES_H

#include <nds.h>
#include <stdbool.h>

#define MAX_LAYERS 8

typedef struct {
    int active_brush_size;
    int eraser_size;
    int active_color_idx;
    int drawing_mode;
    bool is_bucket;
    bool is_eraser;
    uint16_t palette_colors[5];
    
    // Background & perspective parameters
    bool bg_modifiable;
    int bg_pattern_idx;
    int bg_angle;
    int bg_color_p_idx;
    int bg_color_s_idx;
    int angle_target; // 0 = brush, 1 = background
    int nib_angle;
    
    int perspective_mode;
    int perspective_points[4][2];
    int perspective_step;
    
    // Layer structure parameters
    int layers_count;
    int active_layer_idx;
    bool layers_visible[MAX_LAYERS];
    char layer_names[MAX_LAYERS][16];
    uint8_t layers_opacity[MAX_LAYERS];
} DrawState;

typedef struct {
    int open_modal;
    int preset_page;
    int custom_page;
    int selected_custom_slot;
    int color_modal_tab;
    int bg_modal_tab;
    
    int picker_x;
    int picker_y;
    int picker_h;
    int picker_s;
    int picker_v;
    
    uint16_t app_theme_color;
    int active_theme_idx;
    int current_lang; // 0 = Spanish, 1 = English
    bool show_lang_modal;
    
    bool layers_panel_open;
    int dragging_layer_idx;
    bool toolbar_hidden;
} UIState;

typedef struct {
    DrawState draw;
    UIState ui;
} AppState;

#endif // UI_TYPES_H
