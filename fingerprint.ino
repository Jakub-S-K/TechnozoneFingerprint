//#include <Arduino.h>
#include <Adafruit_Fingerprint.h>

#define SERIAL_PINS 5,4
#define LED_RED 13
#define LED_GREEN 12
#define LED_BLUE 14
#define BUTTON_PIN 16

#define DEBUG

#ifdef DEBUG
#define PRINT(X) Serial.print(X)
#define PRINTLN(X) Serial.println(X)
#else
#define PRINT(X) 
#define PRINTLN(X) 
#endif

#include "led_interrupt.hpp"

SoftwareSerial mySerial(SERIAL_PINS);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

int get_finger();
void scan_fingerprint();
bool try_add_new_finger(const int &id);
void(* resetFunc) (void) = 0;

void setup()
{
    delay(1000);
    Serial.begin(115200);
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    delay(200);
    update_led_status(HOLD, RED, 200);
    //update_led_status(FADE, WHITE, 400);
    finger.begin(57600);
    delay(200);         //TODO: WAITING LIBRARY
    if (finger.verifyPassword()) { 
        update_led_status(HOLD, GREEN, 300);
        delay(350);
    } else {
        for (;;) {
            PRINTLN("FINGERPRINT SENSOR CONNECTION ERROR");
            update_led_status(HOLD, RED, 2000);
            delay(2500);
        }
    }
}

void loop()
{
    if (!digitalRead(BUTTON_PIN)) {
        for (;;) {
            add_fingerprint();
        }
    } else {
      scan_fingerprint();
      delay(400);
    }
}

int get_finger() {
  PRINT("FREE FINGERPRINT SLOT");
  for (int i = 50; i < 128; ++i) {
      uint8_t p = finger.loadModel(i);
      switch (p) {
        case FINGERPRINT_OK:
          break;
        case FINGERPRINT_PACKETRECIEVEERR:
          break;
        default:
          return i;
      }
    }
    return 0;
}

void add_fingerprint() {
    update_led_status(HOLD, BLUE, 1000);
    delay(900);
    int index = get_finger();
    if(try_add_new_finger(index))
        resetFunc();
}

bool try_add_new_finger(const int &id) {
  int p = -1;
  update_led_status(BLINK, WHITE, 1000000);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      update_led_status(HOLD, GREEN, 2000);
      break;
    default:
    return false;
      break;
    }
  }
  delay(2000);
  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK)
    return false;

  p = 0;
  update_led_status(BLINK, WHITE, 1000000);
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }

  p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    if(p == FINGERPRINT_OK)
        update_led_status(HOLD, GREEN, 2000);
  }
  delay(2000);

  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK)
    return false;

  p = finger.createModel();
  if (p != FINGERPRINT_OK)
    return false;
    
  p = finger.storeModel(id);
  if (p != FINGERPRINT_OK)
    return false;
  
  return true;
}

void scan_fingerprint() {
  uint8_t p = finger.getImage();

  if (p != FINGERPRINT_OK)
    return;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)
    return;

  p = finger.fingerSearch();
  if (p != FINGERPRINT_OK)
    return;

  digitalWrite(BUTTON_PIN, LOW);
  delay(2000);
  digitalWrite(BUTTON_PIN, HIGH);
}