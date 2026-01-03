# Custom Static Image for Right Display

Replace the gem animation with your own static image on the peripheral (right) display.
The image covers the middle and bottom areas, leaving only the top bar (battery/signal) unchanged.

## Quick Start

### Step 1: Convert Your Image

1. Prepare a **black & white PNG** image at **124×68 pixels** (landscape)
2. Go to [image2cpp](https://javl.github.io/image2cpp/)
3. Upload your image
4. Configure settings:
   - **Canvas size**: `124 x 68`
   - **Background color**: `White`
   - **Invert image colors**: Check if needed
   - **Rotate image**: `90 degrees`
5. Set **Output options**:
   - **Code output format**: `Plain bytes`
   - **Draw mode**: `Horizontal - 1 bit per pixel`
6. Click **Generate code**

### Step 2: Copy the Output

You'll get output like:

```
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0,
...
```

**Copy all the bytes** (should be 124 rows × 9 bytes = 1116 bytes).

### Step 3: Paste Into static_img.c

Open `boards/shields/nice_view_gem/assets/static_img.c` and:

1. Verify the dimensions (should already be correct):
   ```c
   #define STATIC_IMG_WIDTH  68
   #define STATIC_IMG_HEIGHT 124
   ```

2. Replace the pixel data inside `static_img_map[]` (after the color palette):
   ```c
   const ... uint8_t static_img_map[] = {
   #if CONFIG_NICE_VIEW_WIDGET_INVERTED
       0x00, 0x00, 0x00, 0xff,
       0xff, 0xff, 0xff, 0xff,
   #else
       0xff, 0xff, 0xff, 0xff,
       0x00, 0x00, 0x00, 0xff,
   #endif
       // PASTE YOUR BYTES HERE
       0xff, 0xff, 0xff, ...
   };
   ```

3. Verify `data_size` is `1124` (8 palette + 1116 pixels)

### Step 4: Enable Static Image Mode

In `config/corne.conf`, set:

```ini
CONFIG_NICE_VIEW_GEM_ANIMATION=n
```

### Step 5: Build and Flash

Build and flash your firmware to both keyboard halves.

---

## Display Layout

```
┌─────────────────────────────┐
│  Battery  │  Signal         │  ← Top bar (36px) - unchanged
├───────────┴─────────────────┤
│                             │
│                             │
│     YOUR CUSTOM IMAGE       │  ← Middle + Bottom (124px)
│       68 × 124 pixels       │
│                             │
│                             │
└─────────────────────────────┘
```

---

## Config Options

| Setting | Value | Description |
|---------|-------|-------------|
| `CONFIG_NICE_VIEW_GEM_ANIMATION=y` | Animation | Shows the gem crystal animation (smaller) |
| `CONFIG_NICE_VIEW_GEM_ANIMATION=n` | Static | Shows your custom image (larger, 68×124) |

---

## image2cpp Settings Summary

| Setting | Value |
|---------|-------|
| Canvas size | 124 x 68 |
| Background color | White |
| Rotate image | 90 degrees |
| Code output format | Plain bytes |
| Draw mode | Horizontal - 1 bit per pixel |

---

## Tips

- Create your image at **124×68** (landscape), it gets rotated to **68×124** (portrait)
- Use pure black and white images (no grayscale)
- Simple line art works best
- If image appears inverted, check "Invert image colors" in image2cpp or toggle `CONFIG_NICE_VIEW_WIDGET_INVERTED=y`

---

## Troubleshooting

| Issue | Solution |
|-------|----------|
| White/black rectangle | You haven't replaced the placeholder data |
| Image looks wrong | Check rotation is set to 90° |
| Image cropped | Check canvas size is 124×68 |
| Colors inverted | Toggle "Invert image colors" or `CONFIG_NICE_VIEW_WIDGET_INVERTED` |
| Build error | Make sure data_size is 1124 |
