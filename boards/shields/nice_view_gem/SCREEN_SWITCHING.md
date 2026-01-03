# Screen Switching for nice_view_gem **Display**

## Overview

The nice_view_gem left (central) display supports screen cycling in the middle section. Press `&scr_cyc` to switch between different content views.

## Left Display Layout

```
┌─────────────────────┐
│  SIG          BAT%  │  ← Top (always visible)
├─────────────────────┤
│                     │
│   SCREEN 1 or 2     │  ← Middle (cycles)
│        ○●           │  ← Screen selector
├─────────────────────┤
│      LAYER 0        │  ← Bottom (always visible)
└─────────────────────┘
```

### Screen 1 (Default)
- **Middle**: Bluetooth profile viewer (5 profiles) + Screen selector dots

### Screen 2
- **Middle**: Gem animation + Screen selector dots

## Right Display Layout

```
┌─────────────────────┐
│  SIG          BAT%  │
├─────────────────────┤
│                     │
│    GEM ANIMATION    │
│                     │
│                     │
└─────────────────────┘
```

The right (peripheral) display shows the gem animation with battery and signal status.

## Screen Cycling

Press `&scr_cyc` in your keymap to cycle through screens on the **left display**.

### Adding to Your Keymap

```dts
// Example: On a function layer
&scr_cyc  // Cycles between profile viewer and gem animation
```

### Example Key Binding

```dts
// In LAYER_3:
&trans  &trans  &trans  &trans  &trans  &trans    &trans  &scr_cyc  &trans  &trans  &trans  &trans
```

## Technical Details

- Screen cycling behavior: `widgets/behavior_screen_cycle.c`
- Central display: `widgets/screen.c`
- Peripheral display: `widgets/screen_peripheral.c`
- Screen selector widget: `widgets/screen_selector.c`
- Device tree binding: `dts/bindings/behaviors/zmk,behavior-screen-cycle.yaml`
