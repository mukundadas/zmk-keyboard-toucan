#include <zephyr/kernel.h>
#include <stdio.h>
#include "output.h"
#include "../assets/custom_fonts.h"

#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)

/*
 * Friendly names for each BLE profile slot, shown next to the slot number.
 * ZMK has no API for the host's advertised name, so these are user-defined.
 * Index 0 == profile 1. Keep them short/uppercase (the pixel font is caps-only).
 * Edit these to match your own devices.
 */
static const char *profile_names[] = {
    "MAC",   /* slot 1 */
    "PC",    /* slot 2 */
    "IPAD",  /* slot 3 */
    "WORK",  /* slot 4 */
    "EXTRA", /* slot 5 */
};

/*
 * Bottom status band. Everything here is drawn plainly in canvas space so it
 * shares the same 90deg rotation as the layer / WPM text for a consistent look.
 * Layout runs along +x (the text reading direction): [icon] [slot] [name].
 */
#define BAND_Y 134      /* top of the strip                  */
#define ICON_W 16       /* icon footprint (x)                */
#define ICON_H 24       /* icon footprint (y)                */
#define ICON_LW 3       /* bold strokes so it reads clearly  */

/*
 * Bluetooth rune: vertical stem with two crossing diagonals forming the
 * right-hand diamonds. One 6-point polyline.
 */
static void draw_bt_icon(lv_obj_t *canvas, int bx) {
    lv_draw_line_dsc_t dsc;
    init_line_dsc(&dsc, LVGL_FOREGROUND, ICON_LW);

    int cx = bx + ICON_W / 2;
    int top = BAND_Y;
    int bot = BAND_Y + ICON_H;
    int q = ICON_H / 4;

    lv_point_t pts[6] = {
        {.x = bx, .y = top + q},          /* upper-left  */
        {.x = bx + ICON_W, .y = bot - q}, /* lower-right */
        {.x = cx, .y = bot},              /* bottom apex */
        {.x = cx, .y = top},              /* up the stem */
        {.x = bx + ICON_W, .y = top + q}, /* upper-right */
        {.x = bx, .y = bot - q},          /* lower-left  */
    };
    lv_canvas_draw_line(canvas, pts, 6, &dsc);
}

/*
 * USB trident: vertical stem, arrowhead at the top, a round and a square
 * branch, and a filled base.
 */
static void draw_usb_icon(lv_obj_t *canvas, int bx) {
    lv_draw_line_dsc_t dsc;
    init_line_dsc(&dsc, LVGL_FOREGROUND, ICON_LW);

    int cx = bx + ICON_W / 2;
    int top = BAND_Y;
    int bot = BAND_Y + ICON_H;

    lv_point_t stem[2] = {{.x = cx, .y = top + 1}, {.x = cx, .y = bot}};
    lv_canvas_draw_line(canvas, stem, 2, &dsc);

    lv_point_t arrow[3] = {
        {.x = cx - 4, .y = top + 6}, {.x = cx, .y = top}, {.x = cx + 4, .y = top + 6}};
    lv_canvas_draw_line(canvas, arrow, 3, &dsc);

    lv_point_t lbranch[2] = {{.x = cx, .y = top + 11}, {.x = bx, .y = top + 16}};
    lv_canvas_draw_line(canvas, lbranch, 2, &dsc);

    lv_point_t rbranch[2] = {{.x = cx, .y = top + 15}, {.x = bx + ICON_W, .y = top + 10}};
    lv_canvas_draw_line(canvas, rbranch, 2, &dsc);

    lv_draw_rect_dsc_t fill;
    init_rect_dsc(&fill, LVGL_FOREGROUND);
    lv_canvas_draw_rect(canvas, bx - 1, top + 14, 4, 4, &fill);            /* left dot  */
    lv_canvas_draw_rect(canvas, bx + ICON_W - 2, top + 8, 4, 4, &fill);    /* right tip */
    lv_canvas_draw_rect(canvas, cx - 3, bot - 2, 6, 4, &fill);             /* plug base */
}

/* A circle with the active BLE slot number (1-based) inside. */
static void draw_slot_circle(lv_obj_t *canvas, int x, int profile_index) {
    lv_draw_rect_dsc_t circ;
    lv_draw_rect_dsc_init(&circ);
    circ.bg_opa = LV_OPA_TRANSP;
    circ.border_color = LVGL_FOREGROUND;
    circ.border_width = 2;
    circ.radius = LV_RADIUS_CIRCLE;
    lv_canvas_draw_rect(canvas, x, BAND_Y + 3, 18, 18, &circ);

    char num[2];
    snprintf(num, sizeof(num), "%d", profile_index + 1);
    lv_draw_label_dsc_t label_dsc;
    init_label_dsc(&label_dsc, LVGL_FOREGROUND, &quinquefive_8, LV_TEXT_ALIGN_CENTER);
    lv_canvas_draw_text(canvas, x, BAND_Y + 9, 18, &label_dsc, num);
}

static void draw_name(lv_obj_t *canvas, int x, const char *name) {
    lv_draw_label_dsc_t label_dsc;
    init_label_dsc(&label_dsc, LVGL_FOREGROUND, &quinquefive_8, LV_TEXT_ALIGN_LEFT);
    lv_canvas_draw_text(canvas, x, BAND_Y + 9, SCREEN_WIDTH - x, &label_dsc, name);
}

void draw_output_status(lv_obj_t *canvas, const struct status_state *state) {
    switch (state->selected_endpoint.transport) {
    case ZMK_TRANSPORT_USB:
        draw_usb_icon(canvas, 8);
        draw_name(canvas, 34, "USB");
        break;

    case ZMK_TRANSPORT_BLE: {
        draw_bt_icon(canvas, 8);
        int idx = state->active_profile_index;
        draw_slot_circle(canvas, 32, idx);
        if (state->active_profile_bonded && state->active_profile_connected) {
            const char *name = (idx >= 0 && idx < (int)(sizeof(profile_names) /
                                                        sizeof(profile_names[0])))
                                   ? profile_names[idx]
                                   : "BT";
            draw_name(canvas, 56, name);
        } else if (state->active_profile_bonded) {
            draw_name(canvas, 56, "..."); /* bonded, not connected */
        } else {
            draw_name(canvas, 56, "OPEN"); /* waiting to pair */
        }
        break;
    }

    default:
        draw_name(canvas, 8, "OFF");
        break;
    }
}

#endif
