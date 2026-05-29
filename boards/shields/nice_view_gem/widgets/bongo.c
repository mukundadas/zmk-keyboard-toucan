#include <zephyr/kernel.h>
#include <stdio.h>

#include "bongo.h"
#include "../assets/custom_fonts.h"

LV_IMG_DECLARE(bongo_idle);
LV_IMG_DECLARE(bongo_left);
LV_IMG_DECLARE(bongo_right);

/*
 * The panel is mounted rotated 90deg; the frame bitmaps are pre-rotated so the
 * cat reads upright on screen. Image footprint is 48 (canvas x) x 104 (canvas y).
 * Anchored so it sits across the free strip at the top of the display.
 */
#define BONGO_X 95
#define BONGO_Y 32

static uint8_t cur_wpm = 0;
static bool paw_right = false; /* which paw is currently down while typing */

void bongo_set_wpm(uint8_t wpm) { cur_wpm = wpm; }

bool bongo_is_typing(void) { return cur_wpm > 0; }

void bongo_tick(void) {
    if (cur_wpm > 0) {
        paw_right = !paw_right;
    }
}

uint32_t bongo_period_ms(void) {
    /* Faster typing -> faster taps. */
    int p = 250 - (int)cur_wpm * 2;
    if (p < 90) {
        p = 90;
    } else if (p > 250) {
        p = 250;
    }
    return (uint32_t)p;
}

void draw_bongo_status(lv_obj_t *canvas, const struct status_state *state) {
    const lv_img_dsc_t *frame;
    if (cur_wpm == 0) {
        frame = &bongo_idle;
    } else {
        frame = paw_right ? &bongo_right : &bongo_left;
    }

    lv_draw_img_dsc_t img_dsc;
    lv_draw_img_dsc_init(&img_dsc);
    lv_canvas_draw_img(canvas, BONGO_X, BONGO_Y, frame, &img_dsc);

    /* Live WPM number, kept left of the cat so it doesn't overlap. */
    char buf[12];
    snprintf(buf, sizeof(buf), "WPM %u", (unsigned)state->wpm);
    lv_draw_label_dsc_t label_dsc;
    init_label_dsc(&label_dsc, LVGL_FOREGROUND, &quinquefive_8, LV_TEXT_ALIGN_CENTER);
    lv_canvas_draw_text(canvas, 0, 100, 88, &label_dsc, buf);
}
