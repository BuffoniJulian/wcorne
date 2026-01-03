# Custom Static Image for Right Display

Replace the gem animation with your own static image on the peripheral (right) display.
The image covers the middle and bottom areas, leaving only the top bar (battery/signal) unchanged.

## Quick Start

### Step 1: Convert Your Image

1. Prepare a **black & white PNG** image at **68×124 pixels** (PORTRAIT - as seen on display)
2. Go to [image2cpp](https://javl.github.io/image2cpp/)
3. Upload your image
4. Configure settings:
   - **Canvas size**: `68 x 124`
   - **Background color**: `White`
   - **Invert image colors**: Check if needed
   - **Rotate image**: `90 degrees`
5. Set **Output options**:
   - **Code output format**: `Plain bytes`
   - **Draw mode**: `Horizontal - 1 bit per pixel`
6. Click **Generate code**

### Step 2: Copy the Output

After 90° rotation, you'll get **124×68** output (16 bytes per row × 68 rows):

```
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0,
...
```

**Copy all the bytes** (68 rows × 16 bytes = 1088 bytes).

### Step 3: Paste Into static_img.c

Open `boards/shields/nice_view_gem/assets/static_img.c` and:

1. Verify the dimensions (these are AFTER rotation):
   ```c
   #define STATIC_IMG_WIDTH  124
   #define STATIC_IMG_HEIGHT 68
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
       // PASTE YOUR BYTES HERE (16 bytes per row, 68 rows)
       0xff, 0xff, 0xff, ...
   };
   ```

3. Verify `data_size` is `1096` (8 palette + 1088 pixels)

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
How you see it on the display:
┌──────────┐
│Batt│ Sig │  ← Status bar (36px)
├──────────┤
│          │
│  YOUR    │
│  IMAGE   │  ← Your 68×124 image area
│ (68×124) │
│          │
└──────────┘

Create your image at 68×124 (portrait)
Rotate 90° in image2cpp → outputs 124×68 for the buffer
```

---

## image2cpp Settings Summary

| Setting | Value |
|---------|-------|
| **Your image size** | **68 × 124** (portrait, as seen on display) |
| Canvas size | 68 x 124 |
| Rotate image | **90 degrees** |
| Code output format | Plain bytes |
| Draw mode | Horizontal - 1 bit per pixel |
| **Output size** | 124 × 68 (after rotation) |

---

## Config Options

| Setting | Value | Description |
|---------|-------|-------------|
| `CONFIG_NICE_VIEW_GEM_ANIMATION=y` | Animation | Shows the gem crystal animation |
| `CONFIG_NICE_VIEW_GEM_ANIMATION=n` | Static | Shows your custom image |

---

## Tips

- Create your image at **68×124** (portrait) - exactly as you want it to appear
- The 90° rotation in image2cpp converts it to the buffer format
- Use pure black and white images (no grayscale)
- If image appears inverted, check "Invert image colors" in image2cpp

---

## Troubleshooting

| Issue | Solution |
|-------|----------|
| White/black rectangle | You haven't replaced the placeholder data |
| Image rotated wrong | Make sure rotation is **90 degrees** |
| Image size wrong | Create at 68×124, canvas 68×124, rotate 90° |
| Colors inverted | Toggle "Invert image colors" in image2cpp |
| Build error | Make sure data_size is 1096 |
