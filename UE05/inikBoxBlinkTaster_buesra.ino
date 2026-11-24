/*
  IniK-Box - Name auf dem Display, RGB-LED blinkt, Taster steuern den Takt
  Informatik im Kontext, WiSe 26/27

  Hardware: Carenuity C3 Mini auf der Platine IniK-Box v1.3
  In der Arduino IDE als Board "LOLIN C3 MINI" auswaehlen.

  Pin         Funktion
  ----------  ------------------------------------------
  GPIO 4      schwarzer Taster, gegen GND
  GPIO 5      roter Taster, gegen GND
  GPIO 7      RGB-LED (WS2812B) auf dem C3 Mini
  GPIO 8      SDA (I2C)
  GPIO 10     SCL (I2C)

  Benoetigte Bibliotheken (Bibliotheksverwalter der Arduino IDE):
  U8g2, Adafruit NeoPixel

  Der Aufbau im loop ist derselbe wie bei "Blink without Delay" aus
  Vorlesung 03: previousMillis merkt sich, wann zuletzt umgeschaltet
  wurde, interval ist die Wartezeit dazwischen.
*/

#include <Wire.h>
#include <U8g2lib.h>
#include <Adafruit_NeoPixel.h>

U8G2_SSD1306_64X48_ER_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE);

const int PIN_SCHW = 4;  // schwarz
const int PIN_ROT  = 5;  // rot
const int PIN_LED  = 7;  // RGB-LED

Adafruit_NeoPixel led(1, PIN_LED, NEO_GRB + NEO_KHZ800);

unsigned long previousMillis = 0;
unsigned long interval = 300;   // Millisekunden je Phase
const unsigned long MIN_INTERVAL = 40;    // noch schneller moeglich (vorher 50 ms)
const unsigned long MAX_INTERVAL = 1200;  // frueher am langsamen Ende aus (vorher 1500 ms)
const unsigned long STEP_SCHW = 75;   // vorher 50 ms (groesserer Sprung = schneller spuerbar)
const unsigned long STEP_ROT  = 150;  // vorher 100 ms
bool ledState = false;          // leuchtet die LED gerade?
bool blinkAktiv = true; // Zusatz: Blinken aktiv oder gestoppt?
int  farbe = 0;                 // 0 rot, 1 gruen, 2 blau

bool lastSchwState = HIGH;
bool lastRotState  = HIGH;
unsigned long lastDebounceMillis = 0;
const unsigned long DEBOUNCE_DELAY = 30;

void zeigeFarbe() {
  if (!ledState) {
    led.clear();
  } else if (farbe == 0) {
    led.setPixelColor(0, led.Color(255, 0, 0));
  } else if (farbe == 1) {
    led.setPixelColor(0, led.Color(0, 255, 0));
  } else {
    led.setPixelColor(0, led.Color(0, 0, 255));
  }
  led.show();
}

void zeigeNamenMittig(const char* name) {
  display.clearBuffer();
  display.setFont(u8g2_font_6x10_tf);

  int16_t textBreite = display.getUTF8Width(name);
  int16_t x = (64 - textBreite) / 2;
  if (x < 0) {
    x = 0;
  }
  int16_t y = 27;

  display.setCursor(x, y);
  display.print(name);
  display.sendBuffer();
}


void setup() {
  Wire.begin(8, 10);

  pinMode(PIN_SCHW, INPUT_PULLUP);
  pinMode(PIN_ROT,  INPUT_PULLUP);

  display.begin();
  zeigeNamenMittig("BUESRA");

  led.begin();
  led.setBrightness(40);   // volle Helligkeit blendet
  led.clear();
  led.show();
}

void loop() {
  unsigned long currentMillis = millis();

  // Taster einlesen (INPUT_PULLUP: LOW = gedrueckt)
  bool readingSchw = digitalRead(PIN_SCHW);
  bool readingRot  = digitalRead(PIN_ROT);

  if ((readingSchw != lastSchwState || readingRot != lastRotState) &&
      (currentMillis - lastDebounceMillis >= DEBOUNCE_DELAY)) {
    lastDebounceMillis = currentMillis;

    // Schwarzer Taster gedrueckt (Flanke HIGH -> LOW)
    if (lastSchwState == HIGH && readingSchw == LOW) {
      if (!blinkAktiv) {
        blinkAktiv = true;
        interval = 500;
        ledState = true;
        previousMillis = currentMillis;
        zeigeFarbe();
      } else {
        if (interval > MIN_INTERVAL + STEP_SCHW) {
          interval -= STEP_SCHW;
        } else {
          interval = MIN_INTERVAL;
        }
      }
    }

    // Roter Taster gedrueckt (Flanke HIGH -> LOW)
    if (lastRotState == HIGH && readingRot == LOW && blinkAktiv) {
      if (interval + STEP_ROT <= MAX_INTERVAL) {
        interval += STEP_ROT;
      } else {
        interval = MAX_INTERVAL;
        blinkAktiv = false;
        ledState = false;
        zeigeFarbe();
      }
    }

    lastSchwState = readingSchw;
    lastRotState  = readingRot;
  }

  // Nicht-blockierendes Blinken
  if (blinkAktiv && (currentMillis - previousMillis >= interval)) {
    previousMillis = currentMillis;

    ledState = !ledState;
    if (ledState) {
      farbe = (farbe + 1) % 3;
    }
    zeigeFarbe();
  }
}
