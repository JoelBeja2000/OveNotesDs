#ifndef UI_COMPAT_H
#define UI_COMPAT_H

// UI State compatibility macros
#define active_brush_size     g_app_state.draw.active_brush_size
#define eraser_size           g_app_state.draw.eraser_size
#define active_color_idx      g_app_state.draw.active_color_idx
#define drawing_mode          g_app_state.draw.drawing_mode
#define is_bucket             g_app_state.draw.is_bucket
#define bg_pattern_idx        g_app_state.draw.bg_pattern_idx
#define is_eraser             g_app_state.draw.is_eraser
#define palette_colors        g_app_state.draw.palette_colors

#define open_modal            g_app_state.ui.open_modal
#define preset_page           g_app_state.ui.preset_page
#define custom_page           g_app_state.ui.custom_page
#define selected_custom_slot  g_app_state.ui.selected_custom_slot
#define color_modal_tab       g_app_state.ui.color_modal_tab
#define bg_modal_tab          g_app_state.ui.bg_modal_tab

#define picker_x              g_app_state.ui.picker_x
#define picker_y              g_app_state.ui.picker_y
#define picker_h              g_app_state.ui.picker_h
#define picker_s              g_app_state.ui.picker_s
#define picker_v              g_app_state.ui.picker_v

#define app_theme_color       g_app_state.ui.app_theme_color
#define active_theme_idx      g_app_state.ui.active_theme_idx
#define current_lang          g_app_state.ui.current_lang
#define show_lang_modal       g_app_state.ui.show_lang_modal
#define show_help_modal       g_app_state.ui.show_help_modal
#define help_page             g_app_state.ui.help_page

// Render/Draw State compatibility macros
#define layers_count         g_app_state.draw.layers_count
#define active_layer_idx     g_app_state.draw.active_layer_idx
#define layers_visible       g_app_state.draw.layers_visible
#define layer_names          g_app_state.draw.layer_names
#define layers_opacity       g_app_state.draw.layers_opacity
#define layers_panel_open    g_app_state.ui.layers_panel_open
#define dragging_layer_idx   g_app_state.ui.dragging_layer_idx
#define toolbar_hidden       g_app_state.ui.toolbar_hidden

#define bg_modifiable        g_app_state.draw.bg_modifiable
#define bg_angle             g_app_state.draw.bg_angle
#define angle_target         g_app_state.draw.angle_target
#define bg_color_p_idx       g_app_state.draw.bg_color_p_idx
#define bg_color_s_idx       g_app_state.draw.bg_color_s_idx

#define perspective_mode     g_app_state.draw.perspective_mode
#define perspective_points   g_app_state.draw.perspective_points
#define perspective_step     g_app_state.draw.perspective_step
#define nib_angle            g_app_state.draw.nib_angle

#endif // UI_COMPAT_H
