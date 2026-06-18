#include "display_epaper.h"

#include <Arduino.h>
#include <lvgl.h>
#include <cstring>

static uint8_t* epd_framebuffer = NULL;
static lv_disp_draw_buf_t disp_buf;
static lv_disp_drv_t disp_drv;
static lv_color_t* lvgl_draw_buf = NULL;

struct DisplayRefreshState {
    bool pending;
    unsigned long last_update_time;
};

static DisplayRefreshState display_refresh = {
    .pending = false,
    .last_update_time = 0
};

static const unsigned long UPDATE_INTERVAL_MS = 2000;

void request_screen_update(bool immediate) {
    display_refresh.pending = true;
    if (immediate) {
        display_refresh.last_update_time = millis() - UPDATE_INTERVAL_MS;
    }
}

static bool screen_update_due(unsigned long now) {
    if (!display_refresh.pending) return false;
    return now - display_refresh.last_update_time >= UPDATE_INTERVAL_MS;
}

static void mark_screen_updated(unsigned long now) {
    display_refresh.pending = false;
    display_refresh.last_update_time = now;
}

static inline void epd_set_pixel(int x, int y, uint8_t color) {
    if (x < 0 || x >= kDisplayHorRes || y < 0 || y >= kDisplayVerRes) return;

    uint8_t* buf_ptr = &epd_framebuffer[y * kDisplayHorRes / 2 + x / 2];
    if (x % 2) {
        *buf_ptr = (*buf_ptr & 0x0F) | (color & 0xF0);
    } else {
        *buf_ptr = (*buf_ptr & 0xF0) | (color >> 4);
    }
}

static void lvgl_flush_cb(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* color_p) {
    for (int32_t y = area->y1; y <= area->y2; y++) {
        for (int32_t x = area->x1; x <= area->x2; x++) {
            uint8_t gray8 = color_p->full;
            color_p++;
            epd_set_pixel(x, y, gray8);
        }
    }

    // App code schedules physical EPD refreshes to skip transient LVGL states.
    lv_disp_flush_ready(drv);
}

static void draw_full_screen_with_timing(const char* label) {
    unsigned long start = millis();

    epd_poweron();
    unsigned long after_poweron = millis();

    epd_clear();
    unsigned long after_clear = millis();

    Rect_t area = {
        .x = 0,
        .y = 0,
        .width = kDisplayHorRes,
        .height = kDisplayVerRes
    };
    epd_draw_grayscale_image(area, epd_framebuffer);
    unsigned long after_draw = millis();

    epd_poweroff();
    unsigned long after_poweroff = millis();

    Serial.printf("%s timing: poweron=%lu ms, clear=%lu ms, draw=%lu ms, poweroff=%lu ms, total=%lu ms\n",
                  label,
                  after_poweron - start,
                  after_clear - after_poweron,
                  after_draw - after_clear,
                  after_poweroff - after_draw,
                  after_poweroff - start);
}

void epd_display_init() {
    size_t fb_size = kDisplayHorRes * kDisplayVerRes / 2;
    epd_framebuffer = static_cast<uint8_t*>(ps_calloc(1, fb_size));
    if (!epd_framebuffer) {
        Serial.println("Failed to allocate EPD framebuffer!");
        while (1);
    }
    std::memset(epd_framebuffer, 0xFF, fb_size);
    Serial.printf("EPD framebuffer allocated: %u bytes\n", static_cast<unsigned>(fb_size));

    Serial.println("Initializing EPD...");
    epd_init();

    epd_poweron();
    epd_clear();
    epd_poweroff();
    Serial.println("EPD initialized and cleared!");
}

void epd_update_screen() {
    unsigned long now = millis();
    if (!screen_update_due(now)) return;

    Serial.println("Updating e-paper display...");

    lv_refr_now(NULL);
    draw_full_screen_with_timing("Update");
    mark_screen_updated(millis());

    Serial.println("Display updated!");
}

void epd_force_update() {
    Serial.println("Force updating e-paper display (with clear)...");

    draw_full_screen_with_timing("Force update");
    mark_screen_updated(millis());

    Serial.println("Display updated!");
}

void lvgl_display_init() {
    Serial.println("Initializing LVGL...");

    lv_init();

    size_t buf_size = kDisplayHorRes * kDisplayBufferLines;
    lvgl_draw_buf = static_cast<lv_color_t*>(ps_malloc(buf_size * sizeof(lv_color_t)));
    if (!lvgl_draw_buf) {
        Serial.println("Failed to allocate LVGL draw buffer!");
        while (1);
    }

    lv_disp_draw_buf_init(&disp_buf, lvgl_draw_buf, NULL, buf_size);

    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = kDisplayHorRes;
    disp_drv.ver_res = kDisplayVerRes;
    disp_drv.flush_cb = lvgl_flush_cb;
    disp_drv.draw_buf = &disp_buf;

    lv_disp_drv_register(&disp_drv);

    Serial.println("LVGL initialized!");
}
