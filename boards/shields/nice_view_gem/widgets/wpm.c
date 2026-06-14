#include <zephyr/kernel.h>
#include <stdio.h>

#include "wpm.h"
#include "../assets/custom_fonts.h"

#define GRAPH_X 12
#define GRAPH_Y 96
#define GRAPH_W 120
#define GRAPH_H 34
#define GRAPH_STEP 2  /* WPM_SAMPLES * GRAPH_STEP == GRAPH_W */
#define MIN_SCALE 20
#define LINE_WIDTH 2

static uint8_t samples[WPM_SAMPLES] = {0};
static uint8_t head = 0;  /* index of the oldest sample / next write slot */

void wpm_push_sample(uint8_t value) {
    samples[head] = value;
    head = (head + 1) % WPM_SAMPLES;
}

void draw_wpm_status(lv_obj_t *canvas, const struct status_state *state) {
    /* Walk the ring buffer oldest -> newest and track the window peak. */
    uint8_t ordered[WPM_SAMPLES];
    uint8_t peak = 0;
    for (int i = 0; i < WPM_SAMPLES; i++) {
        uint8_t v = samples[(head + i) % WPM_SAMPLES];
        ordered[i] = v;
        if (v > peak) {
            peak = v;
        }
    }

    /* Light 3-tap smoothing so a single fast/slow second doesn't spike. */
    uint8_t smooth[WPM_SAMPLES];
    for (int i = 0; i < WPM_SAMPLES; i++) {
        int prev = ordered[i > 0 ? i - 1 : 0];
        int cur = ordered[i];
        int next = ordered[i < WPM_SAMPLES - 1 ? i + 1 : WPM_SAMPLES - 1];
        smooth[i] = (uint8_t)((prev + 2 * cur + next) / 4);
    }

    /* Scale to the peak with a little headroom so it never slams the ceiling. */
    uint16_t scale = peak + peak / 6;
    if (scale < MIN_SCALE) {
        scale = MIN_SCALE;
    }

    /* Baseline */
    lv_draw_line_dsc_t base_dsc;
    init_line_dsc(&base_dsc, LVGL_FOREGROUND, 1);
    lv_point_t base[2] = {
        {.x = GRAPH_X, .y = GRAPH_Y + GRAPH_H},
        {.x = GRAPH_X + GRAPH_W, .y = GRAPH_Y + GRAPH_H},
    };
    lv_canvas_draw_line(canvas, base, 2, &base_dsc);

    /* Thick line graph: oldest sample on the left, newest on the right */
    lv_draw_line_dsc_t line_dsc;
    init_line_dsc(&line_dsc, LVGL_FOREGROUND, LINE_WIDTH);
    for (int i = 0; i < WPM_SAMPLES - 1; i++) {
        lv_point_t pts[2];
        pts[0].x = GRAPH_X + i * GRAPH_STEP;
        pts[0].y = GRAPH_Y + GRAPH_H - (smooth[i] * GRAPH_H / scale);
        pts[1].x = GRAPH_X + (i + 1) * GRAPH_STEP;
        pts[1].y = GRAPH_Y + GRAPH_H - (smooth[i + 1] * GRAPH_H / scale);
        lv_canvas_draw_line(canvas, pts, 2, &line_dsc);
    }

    /* Compact live/peak label: "WPM <live>/<peak>" */
    char buf[20];
    snprintf(buf, sizeof(buf), "WPM %u/%u", (unsigned)state->wpm, (unsigned)peak);
    lv_draw_label_dsc_t label_dsc;
    init_label_dsc(&label_dsc, LVGL_FOREGROUND, &quinquefive_8, LV_TEXT_ALIGN_CENTER);
    lv_canvas_draw_text(canvas, 0, 82, SCREEN_WIDTH, &label_dsc, buf);
}
