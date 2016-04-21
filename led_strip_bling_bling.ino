#include <avr/sleep.h>
#include <WS2812.h>
#include <EEPROM.h>
#include <PinChangeInterrupt.h>

#define adc_disable() (ADCSRA &= ~(1<<ADEN)) // disable ADC (before power-off)
#define adc_enable()  (ADCSRA |=  (1<<ADEN)) // re-enable ADC

#define RC_PIN 2
#define LED_NUMBER 8
#define LED_MULTIPLIER 4
#define LED_PIN 4
#define SLEEP_TIME 30
#define BUTTON_PIN 3
#define COLORS 7
#define LONG_PUSH_THRESHOLD 20
#define MAX_CYCLE 256
#define MODES 9
#define EEPROM_MODE_ADDRESS 0
#define EEPROM_COLOR_ADDRESS 1
#define LPF_FACTOR 0.8

const byte colors[COLORS][3] = {
    {255, 0, 0},
    {0, 255, 0},
    {0, 0, 255},
    {255, 0, 255},
    {255, 255, 0},
    {0, 255, 255},
    {255, 255, 255}
  };

WS2812 LED(LED_NUMBER * LED_MULTIPLIER);

byte mode = 0;
byte pushStartCycle;
byte i;
byte colorIndex = 0;
byte cycle = 0;

cRGB currentColor;
cRGB off;
cRGB background;

cRGB output[LED_NUMBER] = {};

volatile unsigned long risingStart = 0;
volatile unsigned int channelLength = 0;

int smooth(int data, float filterVal, float smoothedVal){

  if (filterVal > 1){ // check to make sure params are within range
    filterVal = .99;
  }
  else if (filterVal <= 0){
    filterVal = 0;
  }

  smoothedVal = (data * (1 - filterVal)) + (smoothedVal  *  filterVal);

  return (int)smoothedVal;
}

void setColor() {
  currentColor.r = colors[colorIndex][0];
  currentColor.g = colors[colorIndex][1];
  currentColor.b = colors[colorIndex][2];

  background.r = colors[colorIndex][0] / 5;
  background.g = colors[colorIndex][1] / 5;
  background.b = colors[colorIndex][2] / 5;
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

  attachPinChangeInterrupt(RC_PIN, onChange, CHANGE);
}

void onChange(void) {
  uint8_t trigger = getPinChangeInterruptTrigger(RC_PIN);

  if(trigger == RISING) {
    risingStart = micros();
  } else if(trigger == FALLING) {
    unsigned int val = micros() - risingStart;
    channelLength = smooth(val, LPF_FACTOR, channelLength);
  }
}

void modeOff(byte currentCycle) {
  for (i = 0; i < LED_NUMBER; i++) {
    output[i] = off;
  }
}

void modeOn(byte currentCycle) {
  
  for (i = 0; i < LED_NUMBER; i++) {
    output[i] = currentColor;
  }
}

byte getDivider(byte denom, byte cycle) {
  return cycle / denom;
}

void modeSingleWander(byte currentCycle) {

  currentCycle = getDivider(2, currentCycle);

  for (i = 0; i < LED_NUMBER; i++) {
    output[i] = background; 
  }

  output[currentCycle % LED_NUMBER] = currentColor; 
}

void setOutput(byte led, cRGB color) {

//  if (led <= LED_NUMBER) {
    output[led] = color; 
//  }
  
}

void modeRapid(byte currentCycle) {

  for (i = 0; i < LED_NUMBER; i++) {
    output[i] = background; 
  }

  if (currentCycle == LED_NUMBER << 2) {
    cycle = 0;
    currentCycle = 0;
  }

  if (currentCycle < LED_NUMBER) {
    setOutput(currentCycle % LED_NUMBER, currentColor);

    if (currentCycle + 1 < LED_NUMBER) {
      setOutput((currentCycle + 1) % LED_NUMBER, currentColor);
    }
    if (currentCycle + 2 < LED_NUMBER) {
      setOutput((currentCycle + 2) % LED_NUMBER, currentColor);
    }
  } 
  
}

void modeBoldWander(byte currentCycle) {

  currentCycle = getDivider(2, currentCycle);

  for (i = 0; i < LED_NUMBER; i++) {
    output[i] = background; 
  }

  output[currentCycle % LED_NUMBER] = currentColor;
  output[(currentCycle + 1) % LED_NUMBER] = currentColor; 
}

void modeBolderWander(byte currentCycle) {

  currentCycle = getDivider(2, currentCycle);

  for (i = 0; i < LED_NUMBER; i++) {
    output[i] = off; 
  }

  output[currentCycle % LED_NUMBER] = currentColor;
  output[(currentCycle + 1) % LED_NUMBER] = currentColor; 
  output[(currentCycle + 2) % LED_NUMBER] = currentColor; 
  output[(currentCycle + 3) % LED_NUMBER] = currentColor; 
}

void modeDoubleWander(byte currentCycle) {

  currentCycle = getDivider(2, currentCycle);

  for (i = 0; i < LED_NUMBER; i++) {
    output[i] = off; 
  }

  output[currentCycle % LED_NUMBER] = currentColor; 
  output[(MAX_CYCLE - currentCycle) % LED_NUMBER] = currentColor; 

}

void modeChase(byte currentCycle) {

  currentCycle = getDivider(2, currentCycle);

  for (i = 0; i < LED_NUMBER; i++) {
    output[i] = off; 
  }

  output[currentCycle % LED_NUMBER] = currentColor; 
  output[(currentCycle + (LED_NUMBER / 2)) % LED_NUMBER] = currentColor; 

}

void modeFlash(byte currentCycle) {

  currentCycle = getDivider(2, currentCycle);

  cRGB state = off;

  if (currentCycle % 16 == 12 or currentCycle % 16 == 15) {
     state = currentColor;
  }

  for (i = 0; i < LED_NUMBER; i++) {
    output[i] = state; 
  }

}

byte previousPin = LOW;

byte controllState = LOW;
byte previousState = LOW;

void loop() {

  byte pin = digitalRead(BUTTON_PIN);

  controllState = LOW;

  if (channelLength > 1750 && channelLength < 2200 || digitalRead(BUTTON_PIN) == LOW) {
    controllState = HIGH;
  }

  if (controllState == HIGH and previousState == LOW) {
    pushStartCycle = cycle;
  }
  
  if (controllState == LOW and previousState == HIGH) {

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

  previousState = controllState;

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
      break;

    case 7:
      modeBolderWander(cycle);
      break;

    case 8:
      modeRapid(cycle);
      break;
    
  }

  for (byte strip = 0; strip < LED_MULTIPLIER; strip++) {
    for (byte i = 0; i < LED_NUMBER; i++) {
      LED.set_crgb_at(i + (strip * LED_NUMBER), output[i]); 
    }
  }

  LED.sync();

  cycle++;
  if (cycle == MAX_CYCLE) {
    cycle = 0;
  }
  delay(SLEEP_TIME);
}
