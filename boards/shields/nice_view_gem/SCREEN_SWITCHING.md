# Screen Switching for nice_view_gem Display

## Overview

The nice_view_gem left (central) display supports screen cycling, with a screen selector indicator shown alongside the layer information.

## Left Display Layout

The left display shows:
- **Top**: Signal status + Battery percentage
- **Middle**: Bluetooth profile viewer (5 profiles)  
- **Bottom**: Layer name + Screen selector dots

## Right Display Layout

The right (peripheral) display shows:
- **Top**: Signal status + Battery percentage
- **Center**: Animated gem crystal

## Screen Cycling

Press `&scr_cyc` in your keymap to cycle through screens on the **left display**. Since behaviors run on the central (left) side in ZMK split keyboards, screen cycling works reliably.

### Adding to Your Keymap

Add the `&scr_cyc` behavior to any layer:

```dts
// Example: On a function layer
&scr_cyc  // Cycles through screens when pressed
```

### Example Key Binding

```dts
// In LAYER_3:
&trans  &trans  &trans  &trans  &trans  &trans    &trans  &scr_cyc  &trans  &trans  &trans  &trans
```

## Customizing Screens

### Adding More Screens

1. Update `NUM_SCREENS` in `widgets/screen_selector.c`
2. Update the modulo in `zmk_widget_screen_cycle()` in `widgets/screen.c`
3. Add different content based on `current_screen` value in the draw functions

### Screen 1 (Default)
Shows: Battery, Signal, Profiles, Layer, Screen Selector

### Screen 2
Currently shows the same content (can be customized to show different information)

## Technical Details

- Screen cycling behavior: `widgets/behavior_screen_cycle.c`
- Central display: `widgets/screen.c`
- Peripheral display: `widgets/screen_peripheral.c`  
- Screen selector widget: `widgets/screen_selector.c`
- Device tree binding: `dts/bindings/behaviors/zmk,behavior-screen-cycle.yaml`
