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
 * Bottom status band. The panel is mounted rotated 90deg, so the icons below
 * are drawn upright *on screen* via rp(): screen-right maps to canvas +y and
 * screen-up maps to canvas +x. Text still flows in the panel's native (rotated)
 * direction, matching the layer/WPM labels.
 */
#define BAND_Y 140

/* Map an icon's screen-local point (u = right, v = down) to a canvas point,
 * given the canvas coords (ax, ay) of the icon's on-screen top-left. */
static inline lv_point_t rp(int ax, int ay, int u, int v) {
    return (lv_point_t){.x = (lv_coord_t)(ax - v), .y = (lv_coord_t)(ay + u)};
}

/*
 * Bluetooth rune: vertical stem with two crossing diagonals forming the
 * right-hand diamonds. One 6-point polyline, drawn upright on screen.
 */
static void draw_bt_icon(lv_obj_t *canvas, int ax, int ay) {
    lv_draw_line_dsc_t dsc;
    init_line_dsc(&dsc, LVGL_FOREGROUND, 2);

    lv_point_t pts[6] = {
        rp(ax, ay, 1, 4),   /* upper-left  */
        rp(ax, ay, 11, 12), /* lower-right */
        rp(ax, ay, 6, 16),  /* bottom apex */
        rp(ax, ay, 6, 0),   /* up the stem */
        rp(ax, ay, 11, 4),  /* upper-right */
        rp(ax, ay, 1, 12),  /* lower-left  */
    };
    lv_canvas_draw_line(canvas, pts, 6, &dsc);
}

/*
 * USB trident: vertical stem, arrowhead on top, a round and a square branch,
 * and a filled base. Drawn upright on screen.
 */
static void draw_usb_icon(lv_obj_t *canvas, int ax, int ay) {
    lv_draw_line_dsc_t dsc;
    init_line_dsc(&dsc, LVGL_FOREGROUND, 2);

    lv_point_t stem[2] = {rp(ax, ay, 6, 1), rp(ax, ay, 6, 16)};
    lv_canvas_draw_line(canvas, stem, 2, &dsc);

    lv_point_t arrow[3] = {rp(ax, ay, 3, 4), rp(ax, ay, 6, 0), rp(ax, ay, 9, 4)};
    lv_canvas_draw_line(canvas, arrow, 3, &dsc);

    lv_point_t lbranch[2] = {rp(ax, ay, 6, 7), rp(ax, ay, 1, 11)};
    lv_canvas_draw_line(canvas, lbranch, 2, &dsc);

    lv_point_t rbranch[2] = {rp(ax, ay, 6, 10), rp(ax, ay, 11, 6)};
    lv_canvas_draw_line(canvas, rbranch, 2, &dsc);

    lv_draw_rect_dsc_t fill;
    init_rect_dsc(&fill, LVGL_FOREGROUND);
    lv_point_t ld = rp(ax, ay, 1, 11);  /* left dot  */
    lv_canvas_draw_rect(canvas, ld.x - 1, ld.y - 1, 3, 3, &fill);
    lv_point_t rt = rp(ax, ay, 11, 6);  /* right tip */
    lv_canvas_draw_rect(canvas, rt.x - 1, rt.y - 1, 3, 3, &fill);
    lv_point_t base = rp(ax, ay, 6, 16); /* plug base */
    lv_canvas_draw_rect(canvas, base.x - 2, base.y - 2, 4, 4, &fill);
}

/* A circle with the active BLE slot number (1-based) inside. */
static void draw_slot_circle(lv_obj_t *canvas, int x, int profile_index) {
    lv_draw_rect_dsc_t circ;
    lv_draw_rect_dsc_init(&circ);
    circ.bg_opa = LV_OPA_TRANSP;
    circ.border_color = LVGL_FOREGROUND;
    circ.border_width = 2;
    circ.radius = LV_RADIUS_CIRCLE;
    lv_canvas_draw_rect(canvas, x, BAND_Y, 16, 16, &circ);

    char num[2];
    snprintf(num, sizeof(num), "%d", profile_index + 1);
    lv_draw_label_dsc_t label_dsc;
    init_label_dsc(&label_dsc, LVGL_FOREGROUND, &quinquefive_8, LV_TEXT_ALIGN_CENTER);
    lv_canvas_draw_text(canvas, x, BAND_Y + 5, 16, &label_dsc, num);
}

static void draw_name(lv_obj_t *canvas, int x, const char *name) {
    lv_draw_label_dsc_t label_dsc;
    init_label_dsc(&label_dsc, LVGL_FOREGROUND, &quinquefive_8, LV_TEXT_ALIGN_LEFT);
    lv_canvas_draw_text(canvas, x, BAND_Y + 5, SCREEN_WIDTH - x, &label_dsc, name);
}

void draw_output_status(lv_obj_t *canvas, const struct status_state *state) {
    switch (state->selected_endpoint.transport) {
    case ZMK_TRANSPORT_USB:
        draw_usb_icon(canvas, 30, 142);
        draw_name(canvas, 34, "USB");
        break;

    case ZMK_TRANSPORT_BLE: {
        draw_bt_icon(canvas, 30, 142);
        int idx = state->active_profile_index;
        draw_slot_circle(canvas, 34, idx);
        if (state->active_profile_bonded && state->active_profile_connected) {
            const char *name = (idx >= 0 && idx < (int)(sizeof(profile_names) /
                                                        sizeof(profile_names[0])))
                                   ? profile_names[idx]
                                   : "BT";
            draw_name(canvas, 54, name);
        } else if (state->active_profile_bonded) {
            draw_name(canvas, 54, "..."); /* bonded, not connected */
        } else {
            draw_name(canvas, 54, "OPEN"); /* waiting to pair */
        }
        break;
    }

    default:
        draw_name(canvas, 8, "OFF");
        break;
    }
}

#endif
