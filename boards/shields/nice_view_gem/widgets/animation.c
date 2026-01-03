#include <stdlib.h>
#include <zephyr/kernel.h>
#include "animation.h"

#if IS_ENABLED(CONFIG_NICE_VIEW_GEM_ANIMATION)
/* Animation mode: use crystal frames */
LV_IMG_DECLARE(crystal_01);
LV_IMG_DECLARE(crystal_02);
LV_IMG_DECLARE(crystal_03);
LV_IMG_DECLARE(crystal_04);
LV_IMG_DECLARE(crystal_05);
LV_IMG_DECLARE(crystal_06);
LV_IMG_DECLARE(crystal_07);
LV_IMG_DECLARE(crystal_08);
LV_IMG_DECLARE(crystal_09);
LV_IMG_DECLARE(crystal_10);
LV_IMG_DECLARE(crystal_11);
LV_IMG_DECLARE(crystal_12);
LV_IMG_DECLARE(crystal_13);
LV_IMG_DECLARE(crystal_14);
LV_IMG_DECLARE(crystal_15);
LV_IMG_DECLARE(crystal_16);

const lv_img_dsc_t *anim_imgs[] = {
    &crystal_01, &crystal_02, &crystal_03, &crystal_04, &crystal_05, &crystal_06,
    &crystal_07, &crystal_08, &crystal_09, &crystal_10, &crystal_11, &crystal_12,
    &crystal_13, &crystal_14, &crystal_15, &crystal_16,
};
#else
/* Static image mode: use custom image from assets/static_img.c */
LV_IMG_DECLARE(static_img);
#endif

// Store reference to animation object for stop/resume
static lv_obj_t *anim_obj = NULL;
static bool anim_running = false;

void draw_animation(lv_obj_t *canvas) {
#if IS_ENABLED(CONFIG_NICE_VIEW_GEM_ANIMATION)
    lv_obj_t *art = lv_animimg_create(canvas);
    lv_obj_center(art);

    lv_animimg_set_src(art, (const void **)anim_imgs, 16);
    lv_animimg_set_duration(art, CONFIG_NICE_VIEW_GEM_ANIMATION_MS);
    lv_animimg_set_repeat_count(art, LV_ANIM_REPEAT_INFINITE);
    lv_animimg_start(art);
    anim_obj = art;
    anim_running = true;
#else
    /* Static image mode - larger image covering middle + bottom areas */
    lv_obj_t *art = lv_img_create(canvas);
    lv_img_set_src(art, &static_img);
    anim_obj = art;
    anim_running = false;
    /* Position below top status bar: X=0, Y=36 (36px down for battery/signal area) */
    lv_obj_align(art, LV_ALIGN_TOP_LEFT, 0, 36);
    return;
#endif

    /* Animation uses original positioning (36px from left edge) */
    lv_obj_align(art, LV_ALIGN_TOP_LEFT, 36, 0);
}

void stop_animation(void) {
#if IS_ENABLED(CONFIG_NICE_VIEW_GEM_ANIMATION)
    if (anim_obj != NULL && anim_running) {
        // Delete all animations on this object to allow sleep
        lv_anim_del(anim_obj, NULL);
        anim_running = false;
    }
#endif
}

void resume_animation(void) {
#if IS_ENABLED(CONFIG_NICE_VIEW_GEM_ANIMATION)
    if (anim_obj != NULL && !anim_running) {
        lv_animimg_start(anim_obj);
        anim_running = true;
    }
#endif
}

void toggle_animation(void) {
#if IS_ENABLED(CONFIG_NICE_VIEW_GEM_ANIMATION)
    if (anim_running) {
        stop_animation();
    } else {
        resume_animation();
    }
#endif
}

bool is_animation_running(void) {
    return anim_running;
}
