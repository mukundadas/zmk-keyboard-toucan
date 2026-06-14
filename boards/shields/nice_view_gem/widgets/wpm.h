#pragma once

#include <lvgl.h>
#include "util.h"

#define WPM_SAMPLES 60

void wpm_push_sample(uint8_t value);
void draw_wpm_status(lv_obj_t *canvas, const struct status_state *state);
