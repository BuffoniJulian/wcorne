#include <zephyr/kernel.h>
#include "profile_viewer.h"

static void draw_profile_circles(lv_obj_t *canvas, const struct status_state *state) {
    lv_draw_arc_dsc_t arc_dsc;
    init_arc_dsc(&arc_dsc, LVGL_FOREGROUND, 2);
    lv_draw_arc_dsc_t arc_dsc_filled;
    init_arc_dsc(&arc_dsc_filled, LVGL_FOREGROUND, 9);
    lv_draw_label_dsc_t label_dsc;
    init_label_dsc(&label_dsc, LVGL_FOREGROUND, &lv_font_montserrat_18, LV_TEXT_ALIGN_CENTER);
    lv_draw_label_dsc_t label_dsc_black;
    init_label_dsc(&label_dsc_black, LVGL_BACKGROUND, &lv_font_montserrat_18, LV_TEXT_ALIGN_CENTER);

    // Draw circles - positions for 5 profiles in a pattern (moved up to reduce gap)
    int circle_offsets[5][2] = {
        {13, 5}, {55, 5}, {34, 24}, {13, 43}, {55, 43},
    };

    for (int i = 0; i < 5; i++) {
        bool selected = i == state->active_profile_index;

        if (state->profiles_connected[i]) {
            lv_canvas_draw_arc(canvas, circle_offsets[i][0], circle_offsets[i][1], 13, 0, 360,
                            &arc_dsc);
        } else if (state->profiles_bonded[i]) {
            const int segments = 8;
            const int gap = 20;
            for (int j = 0; j < segments; ++j)
                lv_canvas_draw_arc(canvas, circle_offsets[i][0], circle_offsets[i][1], 13,
                                360. / segments * j + gap / 2.0,
                                360. / segments * (j + 1) - gap / 2.0, &arc_dsc);
        }

        if (selected) {
            lv_canvas_draw_arc(canvas, circle_offsets[i][0], circle_offsets[i][1], 9, 0, 359,
                            &arc_dsc_filled);
        }

        char label[2];
        snprintf(label, sizeof(label), "%d", i + 1);
        // Center the text in the circle - adjust x and y for proper centering
        lv_canvas_draw_text(canvas, circle_offsets[i][0] - 9, circle_offsets[i][1] - 9, 18,
                         (selected ? &label_dsc_black : &label_dsc), label);
    }
}

void draw_profile_viewer_status(lv_obj_t *canvas, const struct status_state *state) {
    draw_profile_circles(canvas, state);
}

