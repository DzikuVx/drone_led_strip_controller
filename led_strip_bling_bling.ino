#include <avr/sleep.h>
#include <WS2812.h>
#include <EEPROM.h>

#define adc_disable() (ADCSRA &= ~(1<<ADEN)) // disable ADC (before power-off)
#define adc_enable()  (ADCSRA |=  (1<<ADEN)) // re-enable ADC

#define LED_NUMBER 8
#define LED_MULTIPLIER 4
#define LED_PIN 4
#define SLEEP_TIME 75
#define BUTTON_PIN 3
#define COLORS 8
#define LONG_PUSH_THRESHOLD 10
#define MAX_CYCLE 256
#define MODES 8
#define EEPROM_MODE_ADDRESS 0
#define EEPROM_COLOR_ADDRESS 1

const byte colors[COLORS][3] = {
    {255, 0, 0},
    {0, 255, 0},
    {0, 0, 255},
    {255, 0, 255},
    {255, 255, 0},
    {0, 255, 255},
    {255, 255, 255},
    {0, 0, 0}
  };

WS2812 LED(LED_NUMBER * LED_MULTIPLIER);

byte i;
byte colorIndex = 0;

cRGB currentColor;
cRGB off;

cRGB output[LED_NUMBER] = {};

void setColor() {
  currentColor.r = colors[colorIndex][0];
  currentColor.g = colors[colorIndex][1];
  currentColor.b = colors[colorIndex][2];
}

void setup() {
  adc_disable();
  LED.setOutput(LED_PIN);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  off.r = 0;
  off.g = 0;
  off.b = 0;

  colorIndex = EEPROM.read(EEPROM_COLOR_ADDRESS);
  if (colorIndex >= COLORS) {
    colorIndex = 0;  
  }

  setColor();
}

byte controllState = LOW;
byte previousState = LOW;

void loop() {

  controllState = LOW;

  if (digitalRead(BUTTON_PIN) == LOW) {
    controllState = HIGH;
  }
  
  if (controllState == LOW and previousState == HIGH) {
    /*
     * Cycle color procedure
     */
    colorIndex++;

    if (colorIndex == COLORS) {
      colorIndex = 0;
    }
    EEPROM.write(EEPROM_COLOR_ADDRESS, colorIndex);
    setColor();
  }

  previousState = controllState;

  for (i = 0; i < LED_NUMBER; i++) {
    output[i] = currentColor;
  }

  for (byte strip = 0; strip < LED_MULTIPLIER; strip++) {
    for (byte i = 0; i < LED_NUMBER; i++) {
      LED.set_crgb_at(i + (strip * LED_NUMBER), output[i]); 
    }
  }

  LED.sync();

  delay(SLEEP_TIME);
}
