#include <zephyr/kernel.h>
#include <stdio.h>

#include "wpm.h"
#include "../assets/custom_fonts.h"

#define GRAPH_X 12
#define GRAPH_Y 108
#define GRAPH_W 120
#define GRAPH_H 28
#define GRAPH_STEP 2  /* WPM_SAMPLES * GRAPH_STEP == GRAPH_W */
#define MIN_SCALE 20

static uint8_t samples[WPM_SAMPLES] = {0};
static uint8_t head = 0;

void wpm_push_sample(uint8_t value) {
    samples[head] = value;
    head = (head + 1) % WPM_SAMPLES;
}

void draw_wpm_status(lv_obj_t *canvas, const struct status_state *state) {
    uint8_t max = MIN_SCALE;
    for (int i = 0; i < WPM_SAMPLES; i++) {
        if (samples[i] > max) {
            max = samples[i];
        }
    }

    /* Baseline */
    lv_draw_line_dsc_t base_dsc;
    init_line_dsc(&base_dsc, LVGL_FOREGROUND, 1);
    lv_point_t base[2] = {
        {.x = GRAPH_X, .y = GRAPH_Y + GRAPH_H},
        {.x = GRAPH_X + GRAPH_W, .y = GRAPH_Y + GRAPH_H},
    };
    lv_canvas_draw_line(canvas, base, 2, &base_dsc);

    /* Line graph: oldest sample on the left, newest on the right */
    lv_draw_line_dsc_t line_dsc;
    init_line_dsc(&line_dsc, LVGL_FOREGROUND, 1);
    for (int i = 0; i < WPM_SAMPLES - 1; i++) {
        uint8_t a = samples[(head + i) % WPM_SAMPLES];
        uint8_t b = samples[(head + i + 1) % WPM_SAMPLES];
        lv_point_t pts[2];
        pts[0].x = GRAPH_X + i * GRAPH_STEP;
        pts[0].y = GRAPH_Y + GRAPH_H - (a * GRAPH_H / max);
        pts[1].x = GRAPH_X + (i + 1) * GRAPH_STEP;
        pts[1].y = GRAPH_Y + GRAPH_H - (b * GRAPH_H / max);
        lv_canvas_draw_line(canvas, pts, 2, &line_dsc);
    }

    /* Current WPM label */
    char buf[16];
    snprintf(buf, sizeof(buf), "WPM %u", (unsigned)state->wpm);
    lv_draw_label_dsc_t label_dsc;
    init_label_dsc(&label_dsc, LVGL_FOREGROUND, &quinquefive_8, LV_TEXT_ALIGN_CENTER);
    lv_canvas_draw_text(canvas, 0, 98, SCREEN_WIDTH, &label_dsc, buf);
}
