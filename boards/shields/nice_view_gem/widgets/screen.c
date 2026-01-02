#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/event_manager.h>
#include <zmk/events/activity_state_changed.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/events/endpoint_changed.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/activity.h>
#include <zmk/battery.h>
#include <zmk/ble.h>
#include <zmk/display.h>
#include <zmk/endpoints.h>
#include <zmk/keymap.h>
#include <zmk/usb.h>

#include "animation.h"
#include "battery.h"
#include "layer.h"
#include "output.h"
#include "pomodoro.h"
#include "profile_viewer.h"
#include "screen.h"
#include "screen_selector.h"

#define NUM_SCREENS 3

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);
static int current_screen = 0;
static lv_obj_t *gem_container = NULL;

/**
 * Draw buffers
 **/

static void draw_top(lv_obj_t *widget, lv_color_t cbuf[], const struct status_state *state) {
    lv_obj_t *canvas = lv_obj_get_child(widget, 0);
    fill_background(canvas);

    // Draw widgets
    draw_output_status(canvas, state);
    draw_battery_status(canvas, state);

    // Rotate for horizontal display
    rotate_canvas(canvas, cbuf);
}

static void draw_middle(lv_obj_t *widget, lv_color_t cbuf[], const struct status_state *state) {
    lv_obj_t *canvas = lv_obj_get_child(widget, 1);
    
    // Always redraw the canvas background first
    fill_background(canvas);
    
    // Hide gem container by default
    if (gem_container != NULL) {
        lv_obj_add_flag(gem_container, LV_OBJ_FLAG_HIDDEN);
    }
    
    switch (current_screen) {
    case 0:
        // Screen 1: Profile viewer
        draw_profile_viewer_status(canvas, state);
        break;
    case 1:
        // Screen 2: Pomodoro timer
        pomodoro_tick();  // Update timer
        draw_pomodoro(canvas);
        break;
    case 2:
        // Screen 3: Gem animation
        if (gem_container != NULL) {
            lv_obj_clear_flag(gem_container, LV_OBJ_FLAG_HIDDEN);
            lv_obj_invalidate(gem_container);
        }
        break;
    }
    
    rotate_canvas(canvas, cbuf);
}

static void draw_bottom(lv_obj_t *widget, lv_color_t cbuf[], const struct status_state *state) {
    lv_obj_t *canvas = lv_obj_get_child(widget, 2);
    fill_background(canvas);

    // Draw layer status and screen selector
    draw_layer_status(canvas, state);
    draw_screen_selector(canvas, current_screen);

    // Rotate for horizontal display
    rotate_canvas(canvas, cbuf);
}

/**
 * Battery status
 **/

static void set_battery_status(struct zmk_widget_screen *widget,
                               struct battery_status_state state) {
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
    widget->state.charging = state.usb_present;
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */
    widget->state.battery = state.level;

    draw_top(widget->obj, widget->cbuf, &widget->state);
}

static void battery_status_update_cb(struct battery_status_state state) {
    struct zmk_widget_screen *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_battery_status(widget, state); }
}

static struct battery_status_state battery_status_get_state(const zmk_event_t *eh) {
    const struct zmk_battery_state_changed *ev = as_zmk_battery_state_changed(eh);

    return (struct battery_status_state){
        .level = (ev != NULL) ? ev->state_of_charge : zmk_battery_state_of_charge(),
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
        .usb_present = zmk_usb_is_powered(),
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */
    };
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_battery_status, struct battery_status_state,
                            battery_status_update_cb, battery_status_get_state);

ZMK_SUBSCRIPTION(widget_battery_status, zmk_battery_state_changed);
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
ZMK_SUBSCRIPTION(widget_battery_status, zmk_usb_conn_state_changed);
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */

/**
 * Layer status
 **/

static void set_layer_status(struct zmk_widget_screen *widget, struct layer_status_state state) {
    widget->state.layer_index = state.index;
    widget->state.layer_label = state.label;

    draw_bottom(widget->obj, widget->cbuf3, &widget->state);
}

static void layer_status_update_cb(struct layer_status_state state) {
    struct zmk_widget_screen *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_layer_status(widget, state); }
}

static struct layer_status_state layer_status_get_state(const zmk_event_t *eh) {
    uint8_t index = zmk_keymap_highest_layer_active();
    return (struct layer_status_state){
        .index = index, 
        .label = zmk_keymap_layer_name(zmk_keymap_layer_index_to_id(index))
    };
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_layer_status, struct layer_status_state, layer_status_update_cb,
                            layer_status_get_state)

ZMK_SUBSCRIPTION(widget_layer_status, zmk_layer_state_changed);

/**
 * Output status
 **/

static void set_output_status(struct zmk_widget_screen *widget,
                              const struct output_status_state *state) {
    widget->state.selected_endpoint = state->selected_endpoint;
    widget->state.active_profile_index = state->active_profile_index;
    widget->state.active_profile_connected = state->active_profile_connected;
    widget->state.active_profile_bonded = state->active_profile_bonded;
    for (int i = 0; i < 5; ++i) {
        widget->state.profiles_connected[i] = state->profiles_connected[i];
        widget->state.profiles_bonded[i] = state->profiles_bonded[i];
    }

    draw_top(widget->obj, widget->cbuf, &widget->state);
    draw_middle(widget->obj, widget->cbuf2, &widget->state);
    draw_bottom(widget->obj, widget->cbuf3, &widget->state);
}

static void output_status_update_cb(struct output_status_state state) {
    struct zmk_widget_screen *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_output_status(widget, &state); }
}

static struct output_status_state output_status_get_state(const zmk_event_t *_eh) {
    struct output_status_state state = {
        .selected_endpoint = zmk_endpoints_selected(),
        .active_profile_index = zmk_ble_active_profile_index(),
        .active_profile_connected = zmk_ble_active_profile_is_connected(),
        .active_profile_bonded = !zmk_ble_active_profile_is_open(),
    };
    for (int i = 0; i < 5; ++i) {
        state.profiles_connected[i] = zmk_ble_profile_is_connected(i);
        state.profiles_bonded[i] = !zmk_ble_profile_is_open(i);
    }
    return state;
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_output_status, struct output_status_state,
                            output_status_update_cb, output_status_get_state)
ZMK_SUBSCRIPTION(widget_output_status, zmk_endpoint_changed);

#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
ZMK_SUBSCRIPTION(widget_output_status, zmk_usb_conn_state_changed);
#endif
#if defined(CONFIG_ZMK_BLE)
ZMK_SUBSCRIPTION(widget_output_status, zmk_ble_active_profile_changed);
#endif

/**
 * Activity state - stop animation on idle to allow sleep
 **/

static int activity_state_listener(const zmk_event_t *eh) {
    const struct zmk_activity_state_changed *ev = as_zmk_activity_state_changed(eh);
    if (ev == NULL) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    switch (ev->state) {
    case ZMK_ACTIVITY_ACTIVE:
        resume_animation();
        break;
    case ZMK_ACTIVITY_IDLE:
    case ZMK_ACTIVITY_SLEEP:
        stop_animation();
        break;
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(widget_activity_status, activity_state_listener);
ZMK_SUBSCRIPTION(widget_activity_status, zmk_activity_state_changed);

/**
 * Screen cycling
 **/

void zmk_widget_screen_cycle(void) {
    current_screen = (current_screen + 1) % NUM_SCREENS;
    struct zmk_widget_screen *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
        draw_middle(widget->obj, widget->cbuf2, &widget->state);
        draw_bottom(widget->obj, widget->cbuf3, &widget->state);
        // Force LVGL to refresh
        lv_obj_t *middle_canvas = lv_obj_get_child(widget->obj, 1);
        lv_obj_t *bottom_canvas = lv_obj_get_child(widget->obj, 2);
        lv_obj_invalidate(middle_canvas);
        lv_obj_invalidate(bottom_canvas);
    }
}

/**
 * Initialization
 **/

int zmk_widget_screen_init(struct zmk_widget_screen *widget, lv_obj_t *parent) {
    widget->obj = lv_obj_create(parent);
    lv_obj_set_size(widget->obj, SCREEN_HEIGHT, SCREEN_WIDTH);

    lv_obj_t *top = lv_canvas_create(widget->obj);
    lv_obj_align(top, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_canvas_set_buffer(top, widget->cbuf, BUFFER_SIZE, BUFFER_SIZE, LV_IMG_CF_TRUE_COLOR);

    lv_obj_t *middle = lv_canvas_create(widget->obj);
    lv_obj_align(middle, LV_ALIGN_TOP_LEFT, 46, 0);  // Profiles/gem area
    lv_canvas_set_buffer(middle, widget->cbuf2, BUFFER_SIZE, BUFFER_SIZE, LV_IMG_CF_TRUE_COLOR);

    lv_obj_t *bottom = lv_canvas_create(widget->obj);
    lv_obj_align(bottom, LV_ALIGN_TOP_LEFT, -22, 0);  // Layer + screen dots area
    lv_canvas_set_buffer(bottom, widget->cbuf3, BUFFER_SIZE, BUFFER_SIZE, LV_IMG_CF_TRUE_COLOR);

    // Create gem animation container for screen 2 (hidden by default)
    gem_container = lv_obj_create(widget->obj);
    lv_obj_set_size(gem_container, 68, 68);
    lv_obj_set_style_bg_opa(gem_container, LV_OBJ_FLAG_HIDDEN, 0);
    lv_obj_set_style_border_width(gem_container, 0, 0);
    lv_obj_set_style_pad_all(gem_container, 0, 0);
    lv_obj_align(gem_container, LV_ALIGN_TOP_LEFT, 46, 0);  // Same as middle canvas
    draw_animation(gem_container);
    lv_obj_add_flag(gem_container, LV_OBJ_FLAG_HIDDEN);

    sys_slist_append(&widgets, &widget->node);
    widget_battery_status_init();
    widget_layer_status_init();
    widget_output_status_init();
    pomodoro_init();

    // Initial draw of all sections
    draw_top(widget->obj, widget->cbuf, &widget->state);
    draw_middle(widget->obj, widget->cbuf2, &widget->state);
    draw_bottom(widget->obj, widget->cbuf3, &widget->state);

    return 0;
}

lv_obj_t *zmk_widget_screen_obj(struct zmk_widget_screen *widget) { return widget->obj; }
