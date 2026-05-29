#pragma once

#include <lvgl.h>
#include "util.h"

/* Draw the bongo cat (current paw pose) plus the live WPM number. */
void draw_bongo_status(lv_obj_t *canvas, const struct status_state *state);

/* Feed the latest WPM; controls idle vs. tapping and tap speed. */
void bongo_set_wpm(uint8_t wpm);

/* True while typing (wpm > 0) — the animation timer only redraws then. */
bool bongo_is_typing(void);

/* Advance the paw pose one step (called from the animation timer). */
void bongo_tick(void);

/* Animation timer period (ms) for the current WPM. */
uint32_t bongo_period_ms(void);
