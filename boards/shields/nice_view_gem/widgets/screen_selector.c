#include <zephyr/kernel.h>
#include "screen_selector.h"
#include "../assets/custom_fonts.h"

#define NUM_SCREENS 2

void draw_screen_selector(lv_obj_t *canvas, int current_screen) {
    lv_draw_rect_dsc_t rect_white_dsc;
    init_rect_dsc(&rect_white_dsc, LVGL_FOREGROUND);
    
    lv_draw_rect_dsc_t rect_black_dsc;
    init_rect_dsc(&rect_black_dsc, LVGL_BACKGROUND);

    // Draw larger dots for screen selection at the bottom
    int dot_size = 6;
    int dot_spacing = 12;
    int start_x = (68 - (NUM_SCREENS * dot_spacing - (dot_spacing - dot_size))) / 2;
    int y_pos = 10;  // Position from top of bottom canvas

    for (int i = 0; i < NUM_SCREENS; i++) {
        int x_pos = start_x + (i * dot_spacing);
        
        // Draw outer border
        lv_canvas_draw_rect(canvas, x_pos, y_pos, dot_size, dot_size, &rect_white_dsc);
        
        // Fill with background color if not selected
        if (i != current_screen) {
            lv_canvas_draw_rect(canvas, x_pos + 1, y_pos + 1, dot_size - 2, dot_size - 2, &rect_black_dsc);
        }
    }
}

