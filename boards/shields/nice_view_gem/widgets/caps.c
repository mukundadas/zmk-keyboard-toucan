#include <zephyr/kernel.h>

#include "caps.h"
#include "../assets/custom_fonts.h"

/*
 * Caps Lock badge. Drawn centered at the top of the landscape canvas, between
 * the two battery icons, only while host Caps Lock is on. A filled white box
 * with black "CAPS" text reads like a little lit-up indicator.
 */
#define CAPS_X 60
#define CAPS_Y 4
#define CAPS_W 48
#define CAPS_H 15

void draw_caps_status(lv_obj_t *canvas, const struct status_state *state) {
    if (!state->caps_active) {
        return;
    }

    lv_draw_rect_dsc_t rect_dsc;
    init_rect_dsc(&rect_dsc, LVGL_FOREGROUND);
    rect_dsc.radius = 2;
    lv_canvas_draw_rect(canvas, CAPS_X, CAPS_Y, CAPS_W, CAPS_H, &rect_dsc);

    lv_draw_label_dsc_t label_dsc;
    init_label_dsc(&label_dsc, LVGL_BACKGROUND, &quinquefive_8, LV_TEXT_ALIGN_CENTER);
    lv_canvas_draw_text(canvas, CAPS_X, CAPS_Y + 4, CAPS_W, &label_dsc, "CAPS");
}
