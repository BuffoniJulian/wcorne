/*
 * Copyright (c) 2025 The ZMK Contributors
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>
#include <math.h>
#include "pomodoro.h"
#include "screen.h"

// Default durations in seconds (configurable via Kconfig)
#ifndef CONFIG_NICE_VIEW_GEM_POMODORO_WORK_DURATION
#define CONFIG_NICE_VIEW_GEM_POMODORO_WORK_DURATION 25
#endif

#ifndef CONFIG_NICE_VIEW_GEM_POMODORO_SHORT_BREAK
#define CONFIG_NICE_VIEW_GEM_POMODORO_SHORT_BREAK 5
#endif

#ifndef CONFIG_NICE_VIEW_GEM_POMODORO_LONG_BREAK
#define CONFIG_NICE_VIEW_GEM_POMODORO_LONG_BREAK 15
#endif

#ifndef CONFIG_NICE_VIEW_GEM_POMODORO_LONG_BREAK_INTERVAL
#define CONFIG_NICE_VIEW_GEM_POMODORO_LONG_BREAK_INTERVAL 4
#endif

// Convert minutes to seconds
#define WORK_DURATION_SEC (CONFIG_NICE_VIEW_GEM_POMODORO_WORK_DURATION * 60)
#define SHORT_BREAK_SEC (CONFIG_NICE_VIEW_GEM_POMODORO_SHORT_BREAK * 60)
#define LONG_BREAK_SEC (CONFIG_NICE_VIEW_GEM_POMODORO_LONG_BREAK * 60)
#define LONG_BREAK_INTERVAL CONFIG_NICE_VIEW_GEM_POMODORO_LONG_BREAK_INTERVAL

// Timer state
static struct pomodoro_data pom_data = {
    .state = POM_IDLE,
    .paused_from = POM_IDLE,
    .elapsed_seconds = 0,
    .session_duration = WORK_DURATION_SEC,
    .work_sessions_completed = 0
};

static int64_t last_tick_time = 0;

// Periodic timer for display updates
static void pomodoro_timer_handler(struct k_work *work);
static void pomodoro_timer_expiry(struct k_timer *timer);

K_WORK_DEFINE(pomodoro_work, pomodoro_timer_handler);
K_TIMER_DEFINE(pomodoro_timer, pomodoro_timer_expiry, NULL);

static void pomodoro_timer_handler(struct k_work *work) {
    pomodoro_tick();
    zmk_widget_screen_refresh();
}

static void pomodoro_timer_expiry(struct k_timer *timer) {
    k_work_submit(&pomodoro_work);
}

static void start_pomodoro_timer(void) {
    k_timer_start(&pomodoro_timer, K_SECONDS(1), K_SECONDS(1));
}

static void stop_pomodoro_timer(void) {
    k_timer_stop(&pomodoro_timer);
}

// Circle drawing constants
#define OUTER_RADIUS 28
#define MAX_INNER_RADIUS 24
#define CIRCLE_CENTER_X 34
#define CIRCLE_CENTER_Y 38

void pomodoro_init(void) {
    pom_data.state = POM_IDLE;
    pom_data.elapsed_seconds = 0;
    pom_data.session_duration = WORK_DURATION_SEC;
    pom_data.work_sessions_completed = 0;
    last_tick_time = k_uptime_get();
}

void pomodoro_start_stop(void) {
    switch (pom_data.state) {
    case POM_IDLE:
        // Start work session
        pom_data.state = POM_RUNNING_WORK;
        pom_data.elapsed_seconds = 0;
        // Keep user-configured duration, don't reset to default
        last_tick_time = k_uptime_get();
        start_pomodoro_timer();
        break;
    case POM_RUNNING_WORK:
    case POM_RUNNING_SHORT_BREAK:
    case POM_RUNNING_LONG_BREAK:
        // Pause
        pom_data.paused_from = pom_data.state;
        pom_data.state = POM_PAUSED;
        stop_pomodoro_timer();
        break;
    case POM_PAUSED:
        // Resume
        pom_data.state = pom_data.paused_from;
        last_tick_time = k_uptime_get();
        start_pomodoro_timer();
        break;
    }
}

void pomodoro_reset(void) {
    stop_pomodoro_timer();
    pom_data.state = POM_IDLE;
    pom_data.elapsed_seconds = 0;
    pom_data.session_duration = WORK_DURATION_SEC;
    pom_data.work_sessions_completed = 0;
}

void pomodoro_add_time(void) {
    if (pom_data.state == POM_IDLE) {
        pom_data.session_duration += 300;  // Add 5 minutes
    }
}

void pomodoro_sub_time(void) {
    if (pom_data.state == POM_IDLE && pom_data.session_duration > 300) {
        pom_data.session_duration -= 300;  // Subtract 5 minutes (min 5 min)
    }
}

enum pomodoro_state pomodoro_get_state(void) {
    return pom_data.state;
}

uint32_t pomodoro_get_remaining_seconds(void) {
    if (pom_data.elapsed_seconds >= pom_data.session_duration) {
        return 0;
    }
    return pom_data.session_duration - pom_data.elapsed_seconds;
}

uint32_t pomodoro_get_session_duration(void) {
    return pom_data.session_duration;
}

uint8_t pomodoro_get_progress_step(void) {
    // Calculate which 5-minute step we're in (0-5 for 25 min session)
    uint32_t elapsed_minutes = pom_data.elapsed_seconds / 60;
    uint8_t step = elapsed_minutes / 5;
    uint8_t max_steps = pom_data.session_duration / 300;
    if (step > max_steps) step = max_steps;
    return step;
}

void pomodoro_tick(void) {
    if (pom_data.state != POM_RUNNING_WORK && 
        pom_data.state != POM_RUNNING_SHORT_BREAK &&
        pom_data.state != POM_RUNNING_LONG_BREAK) {
        return;
    }

    int64_t now = k_uptime_get();
    int64_t delta_ms = now - last_tick_time;
    
    if (delta_ms >= 1000) {
        pom_data.elapsed_seconds += delta_ms / 1000;
        last_tick_time = now - (delta_ms % 1000);
        
        // Check if session complete
        if (pom_data.elapsed_seconds >= pom_data.session_duration) {
            pom_data.elapsed_seconds = 0;
            
            if (pom_data.state == POM_RUNNING_WORK) {
                pom_data.work_sessions_completed++;
                
                // Determine break type
                if (pom_data.work_sessions_completed >= LONG_BREAK_INTERVAL) {
                    pom_data.state = POM_RUNNING_LONG_BREAK;
                    pom_data.session_duration = LONG_BREAK_SEC;
                    pom_data.work_sessions_completed = 0;
                } else {
                    pom_data.state = POM_RUNNING_SHORT_BREAK;
                    pom_data.session_duration = SHORT_BREAK_SEC;
                }
            } else {
                // Break complete, start new work session
                pom_data.state = POM_RUNNING_WORK;
                pom_data.session_duration = WORK_DURATION_SEC;
            }
        }
    }
}

// Draw circle outline
static void draw_circle_outline(lv_obj_t *canvas, int cx, int cy, int radius, 
                                 lv_draw_rect_dsc_t *dsc) {
    // Midpoint circle algorithm
    int x = radius;
    int y = 0;
    int err = 0;

    while (x >= y) {
        // Draw 8 octants
        lv_canvas_draw_rect(canvas, cx + x, cy + y, 1, 1, dsc);
        lv_canvas_draw_rect(canvas, cx + y, cy + x, 1, 1, dsc);
        lv_canvas_draw_rect(canvas, cx - y, cy + x, 1, 1, dsc);
        lv_canvas_draw_rect(canvas, cx - x, cy + y, 1, 1, dsc);
        lv_canvas_draw_rect(canvas, cx - x, cy - y, 1, 1, dsc);
        lv_canvas_draw_rect(canvas, cx - y, cy - x, 1, 1, dsc);
        lv_canvas_draw_rect(canvas, cx + y, cy - x, 1, 1, dsc);
        lv_canvas_draw_rect(canvas, cx + x, cy - y, 1, 1, dsc);

        y++;
        err += 1 + 2 * y;
        if (2 * (err - x) + 1 > 0) {
            x--;
            err += 1 - 2 * x;
        }
    }
}

// Draw a filled pie segment clockwise from 12 o'clock
// progress: 0.0 to 1.0 (0% to 100%)
static void draw_pie_segment(lv_obj_t *canvas, int cx, int cy, int radius,
                              float progress, lv_draw_rect_dsc_t *dsc) {
    if (progress <= 0.0f) return;
    if (progress > 1.0f) progress = 1.0f;
    
    // Calculate end angle in radians (clockwise from top)
    // Top is -PI/2, full circle is -PI/2 + 2*PI
    float end_angle = progress * 2.0f * 3.14159265f;
    
    // Draw filled pie by checking each pixel
    for (int py = -radius; py <= radius; py++) {
        for (int px = -radius; px <= radius; px++) {
            // Check if point is within circle
            if (px * px + py * py > radius * radius) continue;
            
            // Calculate angle of this point from center (clockwise from top)
            // atan2 gives angle from positive x-axis, we want from negative y-axis (top)
            float angle = atan2((float)px, (float)(-py));
            if (angle < 0) angle += 2.0f * 3.14159265f;
            
            // Check if angle is within the pie segment
            if (angle <= end_angle) {
                int draw_x = cx + px;
                int draw_y = cy + py;
                if (draw_x >= 0 && draw_x < BUFFER_SIZE && 
                    draw_y >= 0 && draw_y < BUFFER_SIZE) {
                    lv_canvas_draw_rect(canvas, draw_x, draw_y, 1, 1, dsc);
                }
            }
        }
    }
}

void draw_pomodoro(lv_obj_t *canvas) {
    lv_draw_rect_dsc_t fg_dsc;
    init_rect_dsc(&fg_dsc, LVGL_FOREGROUND);
    
    lv_draw_rect_dsc_t bg_dsc;
    init_rect_dsc(&bg_dsc, LVGL_BACKGROUND);
    
    lv_draw_label_dsc_t label_dsc;
    init_label_dsc(&label_dsc, LVGL_FOREGROUND, &lv_font_montserrat_14, LV_TEXT_ALIGN_CENTER);

    // Draw outer circle outline
    draw_circle_outline(canvas, CIRCLE_CENTER_X, CIRCLE_CENTER_Y, OUTER_RADIUS, &fg_dsc);
    draw_circle_outline(canvas, CIRCLE_CENTER_X, CIRCLE_CENTER_Y, OUTER_RADIUS - 1, &fg_dsc);
    
    // Calculate progress as fraction of elapsed time
    float progress = 0.0f;
    if (pom_data.session_duration > 0) {
        progress = (float)pom_data.elapsed_seconds / (float)pom_data.session_duration;
    }
    
    // Draw pie segment filling clockwise from top
    if (progress > 0.0f) {
        draw_pie_segment(canvas, CIRCLE_CENTER_X, CIRCLE_CENTER_Y, 
                         OUTER_RADIUS - 3, progress, &fg_dsc);
    }

    // Draw time remaining (MM:SS) above circle
    uint32_t remaining = pomodoro_get_remaining_seconds();
    uint32_t minutes = remaining / 60;
    uint32_t seconds = remaining % 60;
    
    char time_str[8];
    snprintf(time_str, sizeof(time_str), "%02u:%02u", minutes, seconds);
    lv_canvas_draw_text(canvas, 10, 2, 48, &label_dsc, time_str);
    
    // Draw state label below circle
    const char *state_str;
    switch (pom_data.state) {
    case POM_IDLE:
        state_str = "IDLE";
        break;
    case POM_RUNNING_WORK:
        state_str = "WORK";
        break;
    case POM_RUNNING_SHORT_BREAK:
        state_str = "BREAK";
        break;
    case POM_RUNNING_LONG_BREAK:
        state_str = "LONG";
        break;
    case POM_PAUSED:
        state_str = "PAUSE";
        break;
    default:
        state_str = "";
    }
    
    lv_draw_label_dsc_t small_label_dsc;
    init_label_dsc(&small_label_dsc, LVGL_FOREGROUND, &lv_font_montserrat_14, LV_TEXT_ALIGN_CENTER);
    lv_canvas_draw_text(canvas, 5, 54, 58, &small_label_dsc, state_str);
}
