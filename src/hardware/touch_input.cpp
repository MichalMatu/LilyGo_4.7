#include "touch_input.h"

#include <Arduino.h>
#include <TouchDrvGT911.hpp>
#include <Wire.h>
#include <lvgl.h>

#include "hardware/display_epaper.h"
#include "utilities.h"

static TouchDrvGT911 touch;
static lv_indev_drv_t indev_drv;
static bool touch_available = false;

bool touch_is_available() {
    return touch_available;
}

static void lvgl_touch_cb(lv_indev_drv_t* drv, lv_indev_data_t* data) {
    (void)drv;

    if (!touch_available) {
        data->state = LV_INDEV_STATE_REL;
        return;
    }

    int16_t x, y;
    if (touch.getPoint(&x, &y)) {
        data->point.x = x;
        data->point.y = y;
        data->state = LV_INDEV_STATE_PR;
        Serial.printf("Touch: x=%d, y=%d\n", x, y);
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

void touch_init() {
    Serial.println("Initializing touch...");

    Wire.begin(BOARD_SDA, BOARD_SCL);

    pinMode(TOUCH_INT, OUTPUT);
    digitalWrite(TOUCH_INT, HIGH);
    delay(100);

    uint8_t touch_address = 0;
    Wire.beginTransmission(0x14);
    if (Wire.endTransmission() == 0) {
        touch_address = 0x14;
    }
    Wire.beginTransmission(0x5D);
    if (Wire.endTransmission() == 0) {
        touch_address = 0x5D;
    }

    if (touch_address == 0) {
        Serial.println("Touch GT911 not found!");
        touch_available = false;
        return;
    }

    Serial.printf("Touch found at address 0x%02X\n", touch_address);

    touch.setPins(-1, TOUCH_INT);
    if (!touch.begin(Wire, touch_address, BOARD_SDA, BOARD_SCL)) {
        Serial.println("Touch initialization failed!");
        touch_available = false;
        return;
    }

    touch.setMaxCoordinates(kDisplayHorRes, kDisplayVerRes);
    touch.setSwapXY(true);
    touch.setMirrorXY(false, true);

    touch_available = true;
    Serial.println("Touch initialized!");

    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = lvgl_touch_cb;
    lv_indev_drv_register(&indev_drv);

    Serial.println("Touch registered with LVGL!");
}
