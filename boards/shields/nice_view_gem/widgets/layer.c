#include <zephyr/kernel.h>
#include <string.h>
#include "layer.h"

void draw_layer_status(lv_obj_t *canvas, const struct status_state *state) {
    lv_draw_label_dsc_t label_dsc;
    // Use bigger Montserrat 18 font for better visibility
    init_label_dsc(&label_dsc, LVGL_FOREGROUND, &lv_font_montserrat_18, LV_TEXT_ALIGN_CENTER);

    char text[12] = {};

    // Check for both NULL and empty string (same as nice_view)
    if (state->layer_label == NULL || strlen(state->layer_label) == 0) {
        sprintf(text, "LAYER %i", state->layer_index);
    } else {
        strncpy(text, state->layer_label, 11);
        text[11] = '\0';  // Ensure null termination
        to_uppercase(text);
    }

    // Draw centered in bottom canvas
    lv_canvas_draw_text(canvas, 0, 2, 68, &label_dsc, text);
}