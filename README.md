# Dronsie Strip - LED strips controller for quadcopter drones

Although some flight controller software, like [Cleanflight](https://github.com/cleanflight/cleanflight) (and hardware) allows to attach WS2812B based RGB LED strips, it is far from perfect. For example, LED Strip support might disable SoftSerial or Sonar support depending on used hardware. And if you want add some bling to drone not running Cleanflight, you are on your own.

This is why, ***Dronsie Strip*** was created. It is small, independent, WS2812B LED strips controller designed to fit on 250 class quadcopters. 

Features:
* Size: 23x13mm
* Weight: less than 5g 
* 5V input, can be taken directly from BEC
* Supports 8 RGB LED strips based on WS2812B
* 1 button operated. Single button allows to change pattern and color. Short press changes patters, long press changes color

How it works? One picture can be worth thousand words, so here is a small video

[![Dronsie Strip](http://img.youtube.com/vi/arccMxGvTm0/0.jpg)](http://www.youtube.com/watch?v=arccMxGvTm0 "Dronsie Strip")

# Notes
* Tested with [light_WS2812](https://github.com/cpldcpu/light_ws2812) 
* This code works with [ATtiny Classic](https://github.com/SpenceKonde/ATTinyCore) hardware definitions
* Tested with 8MHz internal oscillator