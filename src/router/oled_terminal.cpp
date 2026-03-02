#include "oled_terminal.h"

#include "Fonts/TomThumb.h"
#include "config.h"

OledTerminal::OledTerminal(const uint8_t width, const uint8_t height,
                           const uint8_t addr)
    : _height(height), _width(width), _addr(addr) {
    this->_rows = this->_height / CHAR_PIXEL_HEIGHT;
    this->_columns = this->_width / CHAR_PIXEL_WIDTH;

    this->_termBuffer = new char *[this->_rows];

    // Initialize term buffer as all nullptrs
    for (size_t i = 0; i < this->_rows; i++) {
        this->_termBuffer[i] = nullptr;
    }

    this->_termBufferIdx = SIZE_MAX;
}

uint8_t OledTerminal::begin() {
    if (this->_display != nullptr) {
        LOG(Serial.println("OledTerminal display is already initialized"));
    }
    Wire.begin();

    this->_display =
        new Adafruit_SSD1306(this->_width, this->_height, &Wire, -1);

    return this->_display->begin(SSD1306_SWITCHCAPVCC, this->_addr);
}

OledTerminal::~OledTerminal() {
    for (size_t i = 0; i < this->_rows; i++) {
        if (this->_termBuffer[i] != nullptr) {
            delete[] this->_termBuffer[i];
        }
    }
}

uint8_t OledTerminal::appendLine(const char *buffer) {
    if (this->_termBufferIdx == SIZE_MAX) {
        this->_termBufferIdx = 0;
    } else {
        this->_termBufferIdx = (this->_termBufferIdx + 1) % this->_rows;
    }

    if (this->_termBuffer[this->_termBufferIdx] == nullptr) {
        this->_termBuffer[this->_termBufferIdx] = new char[this->_columns];
    }

    strncpy(this->_termBuffer[this->_termBufferIdx], buffer, this->_columns);
    // Ensure null-terminator
    this->_termBuffer[this->_termBufferIdx][this->_columns - 1] = '\0';

    return 1;
}

uint8_t OledTerminal::draw() {
    this->_display->clearDisplay();

    this->_display->setFont(&TomThumb);
    this->_display->setTextSize(1);
    this->_display->setTextColor(SSD1306_WHITE);

    for (uint8_t i = 0; i < this->_rows; i++) {
        if (this->_termBuffer[i] == nullptr) continue;

        int8_t rowsFromBottom = (i > this->_termBufferIdx)
                                    ? (this->_rows - (i - this->_termBufferIdx))
                                    : (this->_termBufferIdx - i);
        uint8_t screenIndex =
            this->_height - CHAR_PIXEL_HEIGHT * (rowsFromBottom);

        this->_display->setCursor(0, screenIndex);
        this->_display->print(this->_termBuffer[i]);
    }

    this->_display->display();

    return 1;
}
