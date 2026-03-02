#ifndef OLEDTERMINAL_H
#define OLEDTERMINAL_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <Wire.h>

// Default font pixel size
// #define CHAR_PIXEL_HEIGHT 8
// #define CHAR_PIXEL_WIDTH 6

#define CHAR_PIXEL_HEIGHT 5
#define CHAR_PIXEL_WIDTH 3

class OledTerminal {
   private:
    Adafruit_SSD1306* _display = nullptr;
    uint8_t _height;
    uint8_t _width;
    uint8_t _addr;
    uint8_t _rows;
    uint8_t _columns;

    char** _termBuffer;
    size_t _termBufferIdx;

   public:
    OledTerminal(const uint8_t width, const uint8_t height, const uint8_t addr);
    ~OledTerminal();

    uint8_t begin();
    uint8_t appendLine(const char* buffer);
    uint8_t draw();
};

#endif
