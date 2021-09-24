//#include <Arduino.h>
#include <Adafruit_Fingerprint.h>

#define TIMER_INTERRUPT_DEBUG 0
#define TIMERINTERRUPT_LOGLEVEL 0
#include <ESP8266TimerInterrupt.h>

ESP8266Timer ITimer;

enum MODES
{
    BLINK,
    FADE,
    HOLD
};

enum COLORS
{
    BLACK,
    WHITE,
    RED,
    GREEN,
    BLUE
};

struct color_struct
{
    uint8_t c[3];
};

void operator*(color_struct &a, float &mult)
{
    a.c[0] = (uint8_t)(a.c[0] * mult);
    a.c[1] = (uint8_t)(a.c[1] * mult);
    a.c[2] = (uint8_t)(a.c[2] * mult);
}
void operator-=(color_struct &a, const int8_t &dimm)
{
    a.c[0] = ((int)a.c[0] - dimm) <= 0 ? 0 : a.c[0] - dimm;
    a.c[1] = ((int)a.c[1] - dimm) <= 0 ? 0 : a.c[1] - dimm;
    a.c[2] = ((int)a.c[2] - dimm) <= 0 ? 0 : a.c[2] - dimm;
    //                    ((int)a.c[2] - dimm) <= 0 ? 0 : a.c[2] - dimm};
}
bool operator==(const color_struct &a, const int &b) {
    if (a.c[0] == b && a.c[1] == b && a.c[2] == b)
        return true;
    return false;
}

struct led_status
{
    color_struct color;
    MODES mode;
    bool invert;
} RGB{{255, 255, 255}, HOLD, false};

color_struct translate_colors(COLORS c)
{
    switch (c)
    {
    case BLACK:
        return {0, 0, 0};
    case WHITE:
        return {255, 255, 255};
    case RED:
        return {255, 0, 0};
    case GREEN:
        return {0, 255, 0};
    case BLUE:
        return {0, 0, 255};
    default:
        return {0, 0, 0};
    }
}

color_struct invert_colors(color_struct c)
{
    return {255 - c.c[0], 255 - c.c[1], 255 - c.c[2]};
}

void set_led(color_struct color)
{
    if (RGB.invert)
        color = invert_colors(color);
        
    //PRINT("[set_led] ");
    //PRINT(color.c[0]);
    //PRINT(" ");
    //PRINT(color.c[1]);
    //PRINT(" ");
    //PRINTLN(color.c[2]);
    analogWrite(LED_RED,   color.c[0]);
    analogWrite(LED_GREEN, color.c[1]);
    analogWrite(LED_BLUE,  color.c[2]);
}

void IRAM_ATTR rgb_led_handler()
{
    //PRINTLN("[HOHO] interrupt! ");
    static bool status = false;
    if (RGB.mode != BLINK)
        status = true;
    switch (RGB.mode)
    {
    case BLINK:
        if (status)
            set_led(RGB.color);
        else
            set_led(translate_colors(BLACK));
        status = !status;
        break;
    case FADE:
        set_led(RGB.color);
        RGB.color -= 1;
        //PRINT("po ");
        //PRINTLN(RGB.color.c[0]);
        
        if (RGB.color == 0)
        {
            //TODO FADE is not working properly. Interrupt crash
            ITimer.detachInterrupt();
        }
        break;
    case HOLD:
        static bool first_time = true;
        if (!first_time){
            set_led(translate_colors(BLACK));
            ITimer.detachInterrupt();
            first_time = true;
            break;
        }
        set_led(RGB.color);
        first_time = false;
        break;
    default:
        break;
    }
}

void update_led_status(MODES mode, COLORS color, uint32_t time = 2000, bool invert = false)
{
    //RGB.color = invert ? invert_colors(translate_colors(color)) : translate_colors(color);
    RGB.color = translate_colors(color);
    //PRINT("update_led_status RGB: ");
    //PRINT(RGB.color.c[0]);
    //PRINT(RGB.color.c[1]);
    //PRINTLN(RGB.color.c[2]);
    RGB.mode = mode;
    ITimer.restartTimer();
    if (mode == FADE)
        ITimer.attachInterruptInterval(time * 1000 / 256, rgb_led_handler);
    else if (mode == BLINK) {
        ITimer.attachInterruptInterval(time * 400, rgb_led_handler);
    }
    else
        ITimer.attachInterruptInterval(time * 1000, rgb_led_handler);
}
