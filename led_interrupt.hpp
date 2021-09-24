//#include <Arduino.h>
#include <Adafruit_Fingerprint.h>

#include "defines.hpp"

#include <ESP8266TimerInterrupt.h>

ESP8266Timer ITimer;

enum MODES
{
    BLINK = 0,
    FADE,
    HOLD
};

enum COLORS
{
    BLACK = 0,
    WHITE,
    RED,
    GREEN,
    BLUE
};

union color_struct {
    struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };
    uint8_t rgb[3];
};

struct led_status
{
    color_struct color;
    MODES mode;
    bool color_inverted;
} led {{255, 255, 255}, HOLD, false};

void operator*(color_struct &a, float &mult);
void operator-=(color_struct &a, const int8_t &dimm);
bool operator==(const color_struct &a, const int8_t &b);

color_struct translate_colors(COLORS c);
color_struct invert_colors(color_struct c);

void set_led(color_struct color);
void IRAM_ATTR rgb_led_handler();
void update_led_status(MODES mode, COLORS color, uint32_t time = 2000, bool invert = false);