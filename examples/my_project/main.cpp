/**
 * @file      main.cpp
 * @brief     LilyGo T5 4.7" E-Paper S3 z LVGL 8.x
 * @date      2024-12-05
 */

#ifndef BOARD_HAS_PSRAM
#error "Please enable PSRAM, Arduino IDE -> tools -> PSRAM -> OPI !!!"
#endif

#include <Arduino.h>
#include <lvgl.h>
#include "epd_driver.h"
#include "utilities.h"

// ============================================================================
// CONFIGURATION
// ============================================================================

#define DISP_HOR_RES EPD_WIDTH   // 960
#define DISP_VER_RES EPD_HEIGHT  // 540
#define DISP_BUF_LINES 60        // Partial buffer height

// ============================================================================
// GLOBALS
// ============================================================================

static uint8_t* epd_framebuffer = NULL;
static lv_disp_draw_buf_t disp_buf;
static lv_disp_drv_t disp_drv;
static lv_color_t* lvgl_draw_buf = NULL;

// Flag to track if screen needs update
static volatile bool screen_dirty = false;
static unsigned long last_update_time = 0;
static const unsigned long UPDATE_INTERVAL_MS = 2000; // Min 2s between full updates

// ============================================================================
// LVGL DISPLAY DRIVER
// ============================================================================

/**
 * @brief Set a pixel in the EPD framebuffer (4-bit packed format)
 */
static inline void epd_set_pixel(int x, int y, uint8_t gray4) {
    if (x < 0 || x >= DISP_HOR_RES || y < 0 || y >= DISP_VER_RES) return;
    
    int idx = y * (DISP_HOR_RES / 2) + (x / 2);
    if (x % 2 == 0) {
        epd_framebuffer[idx] = (epd_framebuffer[idx] & 0xF0) | (gray4 & 0x0F);
    } else {
        epd_framebuffer[idx] = (epd_framebuffer[idx] & 0x0F) | ((gray4 & 0x0F) << 4);
    }
}

/**
 * @brief LVGL flush callback - transfers LVGL buffer to EPD framebuffer
 */
static void lvgl_flush_cb(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* color_p) {
    int32_t x, y;
    
    for (y = area->y1; y <= area->y2; y++) {
        for (x = area->x1; x <= area->x2; x++) {
            // LVGL 8.x with LV_COLOR_DEPTH=8: lv_color_t is 8-bit
            uint8_t gray8 = color_p->full;
            color_p++;
            
            // Convert 8-bit (0-255) to 4-bit (0-15)
            // EPD: 0=white, 15=black; LVGL: 0=black, 255=white
            uint8_t gray4 = 15 - (gray8 >> 4);
            epd_set_pixel(x, y, gray4);
        }
    }
    
    screen_dirty = true;
    lv_disp_flush_ready(drv);
}

// ============================================================================
// EPD UPDATE FUNCTIONS
// ============================================================================

/**
 * @brief Update the e-paper display (full refresh)
 */
void epd_update_screen() {
    if (!screen_dirty) return;
    
    unsigned long now = millis();
    if (now - last_update_time < UPDATE_INTERVAL_MS) {
        return;
    }
    
    Serial.println("Updating e-paper display...");
    
    epd_poweron();
    
    Rect_t area = {
        .x = 0,
        .y = 0,
        .width = DISP_HOR_RES,
        .height = DISP_VER_RES
    };
    epd_draw_grayscale_image(area, epd_framebuffer);
    
    epd_poweroff();
    
    screen_dirty = false;
    last_update_time = now;
    
    Serial.println("Display updated!");
}

/**
 * @brief Force immediate screen update
 */
void epd_force_update() {
    Serial.println("Force updating e-paper display...");
    
    epd_poweron();
    epd_clear();
    
    Rect_t area = {
        .x = 0,
        .y = 0,
        .width = DISP_HOR_RES,
        .height = DISP_VER_RES
    };
    epd_draw_grayscale_image(area, epd_framebuffer);
    
    epd_poweroff();
    
    screen_dirty = false;
    last_update_time = millis();
    
    Serial.println("Display updated!");
}

// ============================================================================
// LVGL INITIALIZATION
// ============================================================================

void lvgl_init() {
    Serial.println("Initializing LVGL...");
    
    lv_init();
    
    // Allocate draw buffer in PSRAM
    size_t buf_size = DISP_HOR_RES * DISP_BUF_LINES;
    lvgl_draw_buf = (lv_color_t*)ps_malloc(buf_size * sizeof(lv_color_t));
    if (!lvgl_draw_buf) {
        Serial.println("Failed to allocate LVGL draw buffer!");
        while(1);
    }
    
    lv_disp_draw_buf_init(&disp_buf, lvgl_draw_buf, NULL, buf_size);
    
    // Initialize display driver
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = DISP_HOR_RES;
    disp_drv.ver_res = DISP_VER_RES;
    disp_drv.flush_cb = lvgl_flush_cb;
    disp_drv.draw_buf = &disp_buf;
    
    lv_disp_drv_register(&disp_drv);
    
    Serial.println("LVGL initialized!");
}

// ============================================================================
// UI CREATION
// ============================================================================

void ui_create() {
    lv_obj_t* scr = lv_scr_act();
    
    // White background
    lv_obj_set_style_bg_color(scr, lv_color_white(), 0);
    
    // ---- Title ----
    lv_obj_t* title = lv_label_create(scr);
    lv_label_set_text(title, "LilyGo T5 4.7\" E-Paper + LVGL");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(title, lv_color_black(), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 30);
    
    // ---- Info ----
    lv_obj_t* info = lv_label_create(scr);
    lv_label_set_text_fmt(info, "Display: %dx%d pixels\n16 grayscale levels", 
                          DISP_HOR_RES, DISP_VER_RES);
    lv_obj_set_style_text_font(info, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(info, lv_color_black(), 0);
    lv_obj_align(info, LV_ALIGN_TOP_MID, 0, 100);
    
    // ---- Button ----
    lv_obj_t* btn = lv_btn_create(scr);
    lv_obj_set_size(btn, 200, 60);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(btn, lv_color_black(), 0);
    lv_obj_set_style_radius(btn, 10, 0);
    
    lv_obj_t* btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, "Hello LVGL!");
    lv_obj_set_style_text_color(btn_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(btn_label, &lv_font_montserrat_24, 0);
    lv_obj_center(btn_label);
    
    // ---- Progress bar ----
    lv_obj_t* bar = lv_bar_create(scr);
    lv_obj_set_size(bar, 400, 30);
    lv_obj_align(bar, LV_ALIGN_CENTER, 0, 100);
    lv_bar_set_value(bar, 70, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(bar, lv_color_make(200, 200, 200), LV_PART_MAIN);
    lv_obj_set_style_bg_color(bar, lv_color_black(), LV_PART_INDICATOR);
    
    // ---- Grayscale demo ----
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
    
    // ---- Footer ----
    lv_obj_t* footer = lv_label_create(scr);
    lv_label_set_text(footer, "ESP32-S3 | PSRAM: 8MB | Flash: 16MB");
    lv_obj_set_style_text_font(footer, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(footer, lv_color_make(100, 100, 100), 0);
    lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, -15);
}

// ============================================================================
// MAIN
// ============================================================================

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n========================================");
    Serial.println("  LilyGo T5 4.7\" E-Paper + LVGL Demo");
    Serial.println("========================================\n");
    
    // Allocate EPD framebuffer in PSRAM
    size_t fb_size = DISP_HOR_RES * DISP_VER_RES / 2;
    epd_framebuffer = (uint8_t*)ps_calloc(1, fb_size);
    if (!epd_framebuffer) {
        Serial.println("Failed to allocate EPD framebuffer!");
        while(1);
    }
    memset(epd_framebuffer, 0xFF, fb_size);
    Serial.printf("EPD framebuffer allocated: %d bytes\n", fb_size);
    
    // Initialize EPD
    Serial.println("Initializing EPD...");
    epd_init();
    epd_poweron();
    epd_clear();
    epd_poweroff();
    Serial.println("EPD initialized!");
    
    // Initialize LVGL
    lvgl_init();
    
    // Create UI
    Serial.println("Creating UI...");
    ui_create();
    
    // Render and update
    Serial.println("Rendering UI...");
    lv_timer_handler();
    lv_refr_now(NULL);
    
    epd_force_update();
    
    Serial.println("\nSetup complete!");
}

void loop() {
    lv_timer_handler();
    epd_update_screen();
    delay(10);
}
