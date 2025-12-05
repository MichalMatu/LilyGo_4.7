/**
 * @file      my_project.ino
 * @brief     Mój projekt na LilyGo T5 4.7" E-Paper S3
 * @date      2024-12-05
 * 
 * @note      Arduino Setting
 *            Tools ->
 *                  Board:"ESP32S3 Dev Module"
 *                  USB CDC On Boot:"Enable"
 *                  USB DFU On Boot:"Disable"
 *                  Flash Size : "16MB(128Mb)"
 *                  Flash Mode"QIO 80MHz
 *                  Partition Scheme:"16M Flash(3M APP/9.9MB FATFS)"
 *                  PSRAM:"OPI PSRAM"
 *                  Upload Mode:"UART0/Hardware CDC"
 *                  USB Mode:"Hardware CDC and JTAG"
 */

#ifndef BOARD_HAS_PSRAM
#error "Please enable PSRAM, Arduino IDE -> tools -> PSRAM -> OPI !!!"
#endif

#include <Arduino.h>
#include "epd_driver.h"
#include "firasans.h"
#include "utilities.h"

// Framebuffer dla ekranu (960x540, 4-bit grayscale = 2 piksele na bajt)
uint8_t *framebuffer = NULL;

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("=== Mój Projekt LilyGo EPD47 ===");

    // Alokacja pamięci na framebuffer (w PSRAM)
    framebuffer = (uint8_t *)ps_calloc(sizeof(uint8_t), EPD_WIDTH * EPD_HEIGHT / 2);
    if (!framebuffer) {
        Serial.println("Błąd alokacji pamięci!");
        while (1);
    }
    memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);  // Wyczyść na biało

    // Inicjalizacja e-paper
    epd_init();

    // Włącz zasilanie ekranu
    epd_poweron();
    
    // Wyczyść ekran
    epd_clear();

    // === TUTAJ TWÓJ KOD ===
    
    // Przykład: wyświetl tekst
    int32_t cursor_x = 50;
    int32_t cursor_y = 100;
    
    writeln((GFXfont *)&FiraSans, "Witaj w moim projekcie!", &cursor_x, &cursor_y, NULL);
    
    cursor_x = 50;
    cursor_y += 60;
    writeln((GFXfont *)&FiraSans, "LilyGo T5 4.7\" E-Paper S3", &cursor_x, &cursor_y, NULL);

    cursor_x = 50;
    cursor_y += 60;
    char buf[64];
    snprintf(buf, sizeof(buf), "Rozdzielczość: %dx%d", EPD_WIDTH, EPD_HEIGHT);
    writeln((GFXfont *)&FiraSans, buf, &cursor_x, &cursor_y, NULL);

    // Wyłącz zasilanie ekranu (oszczędność energii)
    epd_poweroff();
    
    Serial.println("Setup zakończony!");
}

void loop()
{
    // Twoja główna pętla
    delay(1000);
}
