# Screen Switching for nice_view_gem Peripheral Display

## Overview

The nice_view_gem peripheral (right side) display now supports multiple screens that you can switch between using a key combination.

## Features

- **Screen 1**: Animated gem crystal with battery and signal status + screen selector dots
- **Screen 2**: Empty for now (can be customized with additional content)
- **Screen Selector**: Larger dots at the top showing which screen is active

## Usage

### Adding the Screen Cycle Behavior to Your Keymap

To switch screens, add the `&scr_cyc` behavior to your keymap. This behavior is only active on the peripheral (right) side.

Example in `corne.keymap`:

```dts
// In your keymap layer, add:
&scr_cyc  // This will cycle through screens when pressed
```

### Example Key Binding

You can bind it to any key or key combination. Here are some examples:

#### Option 1: Dedicated key
```dts
&scr_cyc  // Press this key to switch screens
```

#### Option 2: Layer + key combination
```dts
// On a function layer:
&trans  &trans  &trans  &trans  &trans  &trans    &trans  &trans  &trans  &trans  &trans  &scr_cyc
```

#### Option 3: Mod-tap or hold-tap
```dts
&mt LGUI SCR_CYC  // Hold for GUI, tap to cycle screens
```

## Customizing Screen 2

To add content to Screen 2, edit `boards/shields/nice_view_gem/widgets/screen_peripheral.c`:

Find the `draw_screen_2()` function and add your custom drawing code:

```c
static void draw_screen_2(lv_obj_t *widget) {
    if (animation_obj != NULL) {
        lv_obj_add_flag(animation_obj, LV_OBJ_FLAG_HIDDEN);
    }
    
    // Add your custom content here
    // Example: Draw text, images, or other widgets
}
```

## Adding More Screens

To add more than 2 screens:

1. Update `NUM_SCREENS` in `widgets/screen_selector.c`
2. Update the modulo in `zmk_widget_screen_cycle()` in `widgets/screen_peripheral.c`
3. Add new case statements in `update_screen_display()` for each screen
4. Create corresponding `draw_screen_X()` functions

## Technical Details

- The behavior is defined in `widgets/behavior_screen_cycle.c`
- The device tree binding is in `dts/bindings/behaviors/zmk,behavior-screen-cycle.yaml`
- The behavior is registered in `nice_view_gem_behaviors.dtsi`
- Screen state is maintained in `widgets/screen_peripheral.c`

