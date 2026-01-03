/*
 * Copyright (c) 2025 The ZMK Contributors
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <lvgl.h>
#include "util.h"

// Pomodoro states
enum pomodoro_state {
    POM_IDLE,
    POM_RUNNING_WORK,
    POM_RUNNING_BREAK,
    POM_PAUSED
};

// Pomodoro timer data
struct pomodoro_data {
    enum pomodoro_state state;
    enum pomodoro_state paused_from;  // State before pause
    uint32_t elapsed_seconds;         // Time elapsed in current session
    uint32_t session_duration;        // Duration of current session in seconds
    uint32_t work_duration;           // User-configured work duration
    uint32_t break_duration;          // User-configured break duration
};

// Control functions
void pomodoro_start_stop(void);
void pomodoro_reset(void);
void pomodoro_add_time(void);       // Add 5 min to work time (when IDLE)
void pomodoro_sub_time(void);       // Sub 5 min from work (or 1 min from break if work at min)
void pomodoro_add_break_time(void); // Add 1 min to break time (when IDLE)

// State getters
enum pomodoro_state pomodoro_get_state(void);
uint32_t pomodoro_get_remaining_seconds(void);
uint32_t pomodoro_get_session_duration(void);
uint8_t pomodoro_get_progress_step(void);  // 0-5 for 5-min increments

// Drawing function
void draw_pomodoro(lv_obj_t *canvas);

// Timer management (called by screen refresh)
void pomodoro_tick(void);

// Initialize timer
void pomodoro_init(void);
