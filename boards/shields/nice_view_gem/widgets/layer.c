#include <zephyr/kernel.h>
#include "layer.h"
#include "../assets/custom_fonts.h"

void draw_layer_status(lv_obj_t *canvas, const struct status_state *state) {
    lv_draw_label_dsc_t label_dsc;
    init_label_dsc(&label_dsc, LVGL_FOREGROUND, &pixel_operator_mono, LV_TEXT_ALIGN_CENTER);

    char text[12] = {};

    if (state->layer_label == NULL) {
        sprintf(text, "LAYER %i", state->layer_index);
    } else {
        strncpy(text, state->layer_label, 11);
        to_uppercase(text);
    }

    // Draw near top of bottom canvas (same as nice_view)
    lv_canvas_draw_text(canvas, 0, 5, 68, &label_dsc, text);
}