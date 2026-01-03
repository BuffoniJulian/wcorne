# Custom Static Image for Right Display

Replace the gem animation with your own static image on the peripheral (right) display.

## Quick Start

### Step 1: Convert Your Image

1. Prepare a **black & white PNG** image (max **69×68 pixels**)
2. Go to [LVGL Image Converter](https://lvgl.io/tools/imageconverter)
3. Select the **LVGL v8** tab
4. Upload your image
5. Set **Color format**: `CF_INDEXED_1_BIT`
6. Set **Output format**: `C array`
7. Click **Convert** and download

### Step 2: Copy the Image Data

Open the downloaded `.c` file. You'll see something like:

```c
const uint8_t my_image_map[] = {
    0xff, 0xff, 0xff, 0xff, /*Color of index 0*/  <-- SKIP these 8 bytes
    0x00, 0x00, 0x00, 0xff, /*Color of index 1*/  <-- SKIP these 8 bytes
    0x00, 0x00, 0x00, 0x00, 0x00, ...  <-- COPY from here
};
```

**Copy only the pixel data** (everything after the first 8 color palette bytes).

### Step 3: Paste Into static_image.c

Open `boards/shields/nice_view_gem/assets/static_img.c` and:

1. Update the dimensions at the top:
   ```c
   #define STATIC_IMG_WIDTH  69   // your image width
   #define STATIC_IMG_HEIGHT 68   // your image height
   ```

2. Replace the `STATIC_IMG_MAP` with your pixel data:
   ```c
   #define STATIC_IMG_MAP \
       0x00, 0x00, 0x00, ...  // paste your data here
   ```

### Step 4: Enable Static Image Mode

In `config/corne.conf`, set:

```ini
CONFIG_NICE_VIEW_GEM_ANIMATION=n
```

### Step 5: Build and Flash

Build and flash your firmware to both keyboard halves.

---

## Config Options

In `corne.conf`:

| Setting | Value | Description |
|---------|-------|-------------|
| `CONFIG_NICE_VIEW_GEM_ANIMATION=y` | Animation | Shows the gem crystal animation |
| `CONFIG_NICE_VIEW_GEM_ANIMATION=n` | Static | Shows your custom image from `static_img.c` |
| `CONFIG_NICE_VIEW_GEM_ANIMATION_MS` | milliseconds | Animation speed (only when animation enabled) |

---

## Display Specs

- **Resolution**: 160×68 pixels total
- **Image area**: ~69×68 pixels (right side)
- **Color depth**: 1-bit (black and white only)

---

## Tips

- Use pure black and white images (no grayscale)
- Simple line art works best
- The default example shows a diamond shape - replace it with your own!
- If image appears inverted, try `CONFIG_NICE_VIEW_WIDGET_INVERTED=y`

---

## Troubleshooting

| Issue | Solution |
|-------|----------|
| Image looks wrong | Make sure you skipped the first 8 palette bytes |
| Build error | Check the dimensions match your image |
| Image cropped | Resize to 69×68 or smaller |
| Colors inverted | Toggle `CONFIG_NICE_VIEW_WIDGET_INVERTED` |
