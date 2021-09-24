//#include <Arduino.h>
#include <Adafruit_Fingerprint.h>

#include "defines.hpp"
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
    PRINTLN("[setup] Establishing connection to fingerprint sensor...");
    finger.begin(57600);
    delay(200);         //TODO: WAITING LIBRARY
    if (finger.verifyPassword()) { 
        update_led_status(HOLD, GREEN, 300);
        PRINTLN("[setup] Sensor connected!");
        delay(350);
    } else {
        PRINTLN("[setup] FINGERPRINT SENSOR CONNECTION ERROR");
        for (;;) {
            update_led_status(HOLD, RED, 2000);
            delay(2500);
        }
    }
}

void loop()
{
    if (!digitalRead(BUTTON_PIN)) {
      PRINTLN("[loop] key inserted");
      for (;;) {
          add_fingerprint();
      }
      PRINTLN("[loop] fingerprint added");
    } else {
      PRINTLN("[loop] key not inserted");
      scan_fingerprint();
      delay(400);
    }
}

int get_finger() {
  PRINTLN("[get_finger] searching for empty slot");
  for (int i = 50; i < 128; ++i) {
      uint8_t p = finger.loadModel(i);
      switch (p) {
        case FINGERPRINT_OK:
          break;
        case FINGERPRINT_PACKETRECIEVEERR:
          break;
        default:
          PRINT("[get_finger] Empty slot found #");
          PRINTLN(i);
          return i;
      }
    }
    PRINTLN("[get_finger] Error empty slot has not been found");
    return 0;
}

void add_fingerprint() {
    update_led_status(HOLD, BLUE, 1000);
    delay(900);
    int index = get_finger();
    if(try_add_new_finger(index))
    {
      PRINTLN("[add_fingerprint] resseting via function pointer");
      resetFunc();
    }
}

bool try_add_new_finger(const int &id) {
  int p = -1;
  update_led_status(BLINK, WHITE, 1000000);
  PRINTLN("[try_add_new_finger] Put finger on sensor");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      update_led_status(HOLD, GREEN, 2000);
      PRINTLN("[try_add_new_finger] Finger found");
      break;
    default:
    return false;
      break;
    }
  }
  delay(2000);
  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) {
    PRINTLN("[try_add_new_finger] image2Tz failed");
    return false;
  }

  p = 0;
  update_led_status(BLINK, WHITE, 1000000);
  PRINTLN("[try_add_new_finger] Put finger again");
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }

  p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    if(p == FINGERPRINT_OK)
        update_led_status(HOLD, GREEN, 2000);
  }
  PRINTLN("[try_add_new_finger] Finger found");
  delay(2000);

  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) {
    PRINTLN("[try_add_new_finger] image2Tz failed");
    return false;
  }

  p = finger.createModel();
  if (p != FINGERPRINT_OK)
    return false;
  PRINTLN("[try_add_new_finger] Model created");
  p = finger.storeModel(id);
  if (p != FINGERPRINT_OK)
    return false;
  PRINTLN("[try_add_new_finger] Finger model stored");
  return true;
}

void scan_fingerprint() {
  uint8_t p = finger.getImage();
  PRINTLN("[scan_fingerprint] Checking if finger on sensor");
  if (p != FINGERPRINT_OK)
    return;
  PRINTLN("[scan_fingerprint] Finger found");
  p = finger.image2Tz();
  if (p != FINGERPRINT_OK){
    PRINTLN("[scan_fingerprint] image2Tz failed");
    return;
  }

  p = finger.fingerSearch();
  if (p != FINGERPRINT_OK){
    PRINTLN("[scan_fingerprint] finger has not been found in database");
    return;
  }
  PRINTLN("[scan_fingerprint] finger has been found in database");
  PRINTLN("[scan_fingerprint] Relay open");
  digitalWrite(BUTTON_PIN, LOW);
  delay(2000);
  PRINTLN("[scan_fingerprint] Relya closed");
  digitalWrite(BUTTON_PIN, HIGH);
}