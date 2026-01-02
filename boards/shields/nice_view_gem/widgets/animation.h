#pragma once

#include <lvgl.h>
#include "util.h"

void draw_animation(lv_obj_t *canvas);
void stop_animation(void);
void resume_animation(void);
void toggle_animation(void);
bool is_animation_running(void);