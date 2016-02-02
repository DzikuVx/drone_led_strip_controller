#include <avr/sleep.h>
#include <WS2812.h>
#include <EEPROM.h>

#define adc_disable() (ADCSRA &= ~(1<<ADEN)) // disable ADC (before power-off)
#define adc_enable()  (ADCSRA |=  (1<<ADEN)) // re-enable ADC

#define LED_NUMBER 8
#define LED_PIN 3
#define SLEEP_TIME 75
#define BUTTON_PIN 2
#define COLORS 7
#define LONG_PUSH_THRESHOLD 10
#define MAX_CYCLE 256
#define MODES 7
#define EEPROM_MODE_ADDRESS 0
#define EEPROM_COLOR_ADDRESS 1

const byte colors[COLORS][3] = {
    {255, 0, 0},
    {0, 255, 0},
    {0, 0, 255},
    {255, 0, 255},
    {255, 255, 0},
    {0, 255, 255},
    {255, 255, 255}
  };

WS2812 LED(LED_NUMBER); // 1 LED

byte mode = 0;
byte pushStartCycle;
byte i;
byte colorIndex = 0;
byte cycle = 0;

cRGB currentColor;
cRGB off;

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

  mode = EEPROM.read(EEPROM_MODE_ADDRESS);
  if (mode >= MODES) {
    mode = 0;  
  }

  colorIndex = EEPROM.read(EEPROM_COLOR_ADDRESS);
  if (colorIndex >= COLORS) {
    colorIndex = 0;  
  }

  setColor();
}

void modeOff(byte currentCycle) {
  for (i = 0; i < LED_NUMBER; i++) {
    LED.set_crgb_at(i, off); // Set value at LED found at index 0
  }
}

void modeOn(byte currentCycle) {
  
  for (i = 0; i < LED_NUMBER; i++) {
    LED.set_crgb_at(i, currentColor); // Set value at LED found at index 0
  }
}

void modeSingleWander(byte currentCycle) {

  for (i = 0; i < LED_NUMBER; i++) {
    LED.set_crgb_at(i, off); 
  }

  LED.set_crgb_at(currentCycle % LED_NUMBER, currentColor); 
}

void modeBoldWander(byte currentCycle) {

  for (i = 0; i < LED_NUMBER; i++) {
    LED.set_crgb_at(i, off); 
  }

  LED.set_crgb_at(currentCycle % LED_NUMBER, currentColor);
  LED.set_crgb_at((currentCycle + 1) % LED_NUMBER, currentColor); 
}

void modeDoubleWander(byte currentCycle) {

  for (i = 0; i < LED_NUMBER; i++) {
    LED.set_crgb_at(i, off); 
  }

  LED.set_crgb_at(currentCycle % LED_NUMBER, currentColor); 
  LED.set_crgb_at((MAX_CYCLE - currentCycle) % LED_NUMBER, currentColor); 

}

void modeChase(byte currentCycle) {

  for (i = 0; i < LED_NUMBER; i++) {
    LED.set_crgb_at(i, off); 
  }

  LED.set_crgb_at(currentCycle % LED_NUMBER, currentColor); 
  LED.set_crgb_at((currentCycle + (LED_NUMBER / 2)) % LED_NUMBER, currentColor); 

}

void modeFlash(byte currentCycle) {

  cRGB state = off;

  if (currentCycle % 16 == 12 or currentCycle % 16 == 15) {
     state = currentColor;
  }

  for (i = 0; i < LED_NUMBER; i++) {
    LED.set_crgb_at(i, state); 
  }

}

byte previousPin = LOW;

void loop() {

  byte pin = digitalRead(BUTTON_PIN);

  if (pin == LOW and previousPin == HIGH) {
    pushStartCycle = cycle;
  }
  
  if (pin == HIGH and previousPin == LOW) {

    if (abs(cycle - pushStartCycle) > LONG_PUSH_THRESHOLD) {
      /*
       * Cycle color procedure
       */
      colorIndex++;
  
      if (colorIndex == COLORS) {
        colorIndex = 0;
      }
      EEPROM.write(EEPROM_COLOR_ADDRESS, colorIndex);
      setColor();
    } else if (abs(cycle - pushStartCycle) > 1) {
      mode++;

      if (mode == MODES) {
        mode = 0;
      }

      EEPROM.write(EEPROM_MODE_ADDRESS, mode);
    }
  }

  previousPin = pin;

  switch (mode) {

    case 0:
      modeOff(cycle);
      break;

    case 1:
      modeOn(cycle);
      break;

    case 2:
      modeSingleWander(cycle);
      break;

    case 3:
      modeBoldWander(cycle);
      break;
      
    case 4:
      modeDoubleWander(cycle);
      break;

    case 5:
      modeChase(cycle);
      break;

    case 6:
      modeFlash(cycle);
    
  }

  LED.sync();

  cycle++;
  if (cycle == MAX_CYCLE) {
    cycle = 0;
  }
  delay(SLEEP_TIME);
}
