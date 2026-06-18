#pragma once

#include "epd_driver.h"

static constexpr int kDisplayHorRes = EPD_WIDTH;
static constexpr int kDisplayVerRes = EPD_HEIGHT;
static constexpr int kDisplayBufferLines = 60;

void epd_display_init();
void lvgl_display_init();
void epd_update_screen();
void epd_force_update();
void request_screen_update(bool immediate = false);
