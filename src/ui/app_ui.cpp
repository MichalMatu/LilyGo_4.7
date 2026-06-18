#include "app_ui.h"

#include <Arduino.h>
#include <lvgl.h>

#include "hardware/display_epaper.h"
#include "hardware/touch_input.h"

struct AppState {
    uint32_t clicks;
    int progress;
};

static AppState app_state = {
    .clicks = 0,
    .progress = 0
};

static lv_obj_t* counter_label = NULL;
static lv_obj_t* bar = NULL;

static void update_counter_ui() {
    lv_label_set_text_fmt(counter_label, "Clicks: %u", app_state.clicks);
    lv_bar_set_value(bar, app_state.progress, LV_ANIM_OFF);
}

static void register_button_click() {
    app_state.clicks++;
    app_state.progress = (app_state.clicks * 10) % 101;
    update_counter_ui();
}

static void btn_click_cb(lv_event_t* e) {
    (void)e;

    register_button_click();

    Serial.printf("Button clicked! Counter: %u\n", app_state.clicks);
    request_screen_update(true);
}

void ui_create() {
    lv_obj_t* scr = lv_scr_act();

    lv_obj_set_style_bg_color(scr, lv_color_white(), 0);

    lv_obj_t* title = lv_label_create(scr);
    lv_label_set_text(title, "LilyGo T5 4.7\" + LVGL + Touch");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(title, lv_color_black(), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 30);

    lv_obj_t* info = lv_label_create(scr);
    lv_label_set_text_fmt(info, "Display: %dx%d | Touch: %s",
                          kDisplayHorRes, kDisplayVerRes,
                          touch_is_available() ? "GT911" : "Not found");
    lv_obj_set_style_text_font(info, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(info, lv_color_black(), 0);
    lv_obj_align(info, LV_ALIGN_TOP_MID, 0, 90);

    counter_label = lv_label_create(scr);
    lv_label_set_text(counter_label, "Clicks: 0");
    lv_obj_set_style_text_font(counter_label, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(counter_label, lv_color_black(), 0);
    lv_obj_align(counter_label, LV_ALIGN_CENTER, 0, -60);

    lv_obj_t* btn = lv_btn_create(scr);
    lv_obj_set_size(btn, 250, 70);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 20);
    lv_obj_set_style_bg_color(btn, lv_color_black(), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btn, lv_color_make(80, 80, 80), LV_STATE_PRESSED);
    lv_obj_set_style_radius(btn, 15, 0);
    lv_obj_add_event_cb(btn, btn_click_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, "TAP ME!");
    lv_obj_set_style_text_color(btn_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(btn_label, &lv_font_montserrat_24, 0);
    lv_obj_center(btn_label);

    bar = lv_bar_create(scr);
    lv_obj_set_size(bar, 400, 30);
    lv_obj_align(bar, LV_ALIGN_CENTER, 0, 110);
    lv_obj_set_style_bg_color(bar, lv_color_make(200, 200, 200), LV_PART_MAIN);
    lv_obj_set_style_bg_color(bar, lv_color_black(), LV_PART_INDICATOR);
    update_counter_ui();

    lv_obj_t* gray_label = lv_label_create(scr);
    lv_label_set_text(gray_label, "Grayscale levels:");
    lv_obj_set_style_text_font(gray_label, &lv_font_montserrat_16, 0);
    lv_obj_align(gray_label, LV_ALIGN_BOTTOM_MID, 0, -120);

    int box_width = 50;
    int total_width = 8 * box_width + 7 * 5;
    int start_x = -total_width / 2 + box_width / 2;

    for (int i = 0; i < 8; i++) {
        lv_obj_t* box = lv_obj_create(scr);
        lv_obj_set_size(box, box_width, 40);
        lv_obj_align(box, LV_ALIGN_BOTTOM_MID, start_x + i * (box_width + 5), -60);
        lv_obj_set_style_radius(box, 5, 0);
        lv_obj_set_style_border_width(box, 1, 0);
        lv_obj_set_style_border_color(box, lv_color_black(), 0);

        uint8_t gray = 255 - (i * 36);
        lv_obj_set_style_bg_color(box, lv_color_make(gray, gray, gray), 0);
    }

    lv_obj_t* footer = lv_label_create(scr);
    lv_label_set_text(footer, "ESP32-S3 | PSRAM: 8MB | Flash: 16MB");
    lv_obj_set_style_text_font(footer, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(footer, lv_color_make(100, 100, 100), 0);
    lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, -15);
}
