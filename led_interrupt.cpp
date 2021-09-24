//#include <Arduino.h>
#include "led_interrupt.hpp"

void operator*(color_struct &a, float &mult)
{
    a.rgb[0] = (uint8_t)(a.rgb[0] * mult);
    a.rgb[1] = (uint8_t)(a.rgb[1] * mult);
    a.rgb[2] = (uint8_t)(a.rgb[2] * mult);
}
void operator-=(color_struct &a, const int8_t &dimm)
{
    a.rgb[0] = ((int)a.rgb[0] - dimm) <= 0 ? 0 : a.rgb[0] - dimm;
    a.rgb[1] = ((int)a.rgb[1] - dimm) <= 0 ? 0 : a.rgb[1] - dimm;
    a.rgb[2] = ((int)a.rgb[2] - dimm) <= 0 ? 0 : a.rgb[2] - dimm;
}
bool operator==(const color_struct &a, const int8_t &b) {
    if (a.rgb[0] == b && a.rgb[1] == b && a.rgb[2] == b)
        return true;
    return false;
}

color_struct translate_colors(COLORS color)
{
    switch (color)
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

color_struct invert_colors(color_struct color)
{
    return {255 - color.rgb[0], 255 - color.rgb[1], 255 - color.rgb[2]};
}

void set_led(color_struct color)
{
    if (led.color_inverted) 
    {
        color = invert_colors(color);
    }
        
    PRINT("[set_led] ");
    PRINT(color.rgb[0]);
    PRINT(" ");
    PRINT(color.rgb[1]);
    PRINT(" ");
    PRINTLN(color.rgb[2]);
    analogWrite(LED_RED,   color.rgb[0]);
    analogWrite(LED_GREEN, color.rgb[1]);
    analogWrite(LED_BLUE,  color.rgb[2]);
}

void IRAM_ATTR rgb_led_handler()
{
    //PRINTLN("[HOHO] interrupt! ");
    static bool status = false;
    if (led.mode != BLINK)
    {
        status = true;
    }
    switch (led.mode)
    {
    case BLINK:
        if (status)
            set_led(led.color);
        else
            set_led(translate_colors(BLACK));
        status = !status;
        break;
    case FADE:
        set_led(led.color);
        led.color -= 1;
        PRINT("po ");
        PRINTLN(led.color.rgb[0]);
        
        if (led.color == 0)
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
        set_led(led.color);
        first_time = false;
        break;
    default:
        break;
    }
}

void update_led_status(MODES mode, COLORS color, uint32_t time = 2000, bool invert = false)
{
    #ifdef EXTRA_INVERT
    //RGB.color = invert ? invert_colors(translate_colors(color)) : translate_colors(color);
    #endif // EXTRA_INVERT
    led.color = translate_colors(color);
    PRINT("update_led_status RGB: ");
    PRINT(led.color.rgb[0]);
    PRINT(led.color.rgb[1]);
    PRINTLN(led.color.rgb[2]);
    led.mode = mode;
    ITimer.restartTimer();
    if (mode == FADE)
        ITimer.attachInterruptInterval(time * 1000 / 256, rgb_led_handler);
    else if (mode == BLINK) {
        ITimer.attachInterruptInterval(time * 400, rgb_led_handler);
    }
    else
        ITimer.attachInterruptInterval(time * 1000, rgb_led_handler);
}
