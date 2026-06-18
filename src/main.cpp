/**
 * @file      main.cpp
 * @brief     LilyGo T5 4.7" E-Paper S3 with LVGL 8.x and touch
 * @date      2024-12-05
 */

#ifndef BOARD_HAS_PSRAM
#error "Please enable PSRAM, Arduino IDE -> tools -> PSRAM -> OPI !!!"
#endif

#include <Arduino.h>
#include <lvgl.h>

#include "hardware/display_epaper.h"
#include "hardware/touch_input.h"
#include "ui/app_ui.h"

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n========================================");
    Serial.println("  LilyGo T5 4.7\" E-Paper + LVGL + Touch");
    Serial.println("========================================\n");

    epd_display_init();
    lvgl_display_init();
    touch_init();

    Serial.println("Creating UI...");
    ui_create();

    request_screen_update(true);
    epd_update_screen();

    Serial.println("\nSetup complete! Touch the screen to interact.");
}

void loop() {
    lv_timer_handler();
    epd_update_screen();
    delay(10);
}
