# LilyGo 4.7 Firmware

PlatformIO firmware project for the LilyGo T5 4.7" ESP32-S3 e-paper board.

The active application uses:

- LVGL 8.x for UI rendering
- GT911 touch input
- LilyGo/EPD47 e-paper driver kept as a local library

## Project Structure

```txt
.
├── include/
│   └── lv_conf.h              # LVGL project configuration
├── lib/
│   └── lilygo_epd/            # Local LilyGo EPD47 driver library
├── src/
│   ├── main.cpp               # Firmware entry point
│   ├── hardware/
│   │   ├── display_epaper.*   # LVGL display backend and e-paper refresh
│   │   └── touch_input.*      # GT911 touch input
│   └── ui/
│       └── app_ui.*           # LVGL UI and application state
└── reference/
    └── lilygo_examples/       # Original LilyGo examples kept for reference
```

Only `src/`, `include/`, and `lib/lilygo_epd/` participate in the firmware build.

## Build

```sh
pio run
```

## Upload

```sh
pio run -t upload
```

## Serial Monitor

```sh
pio device monitor
```

Default monitor speed is `115200`.

## Notes

- `platformio.ini` now builds the firmware from `src/` directly.
- The original LilyGo driver is isolated in `lib/lilygo_epd/` instead of being mixed with application code.
- The original sample sketches are not part of the build; they live under `reference/lilygo_examples/`.
