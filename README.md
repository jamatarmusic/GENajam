# genajam
Front end editor for GenMDM by Littlescale

A physical front end interface for GenMDM for Mega Drive. This hardware device will allow you to access all FM settings for the 6 channels offered by GenMDM as well as load TFI files from SD card and assign them per channel.

This open source program is written for the Arduino Uno or Nano. Once the software hits alpha with a physical prototype, I'll include a schematic so you can build your own!

Currently uses the LCD Keypad Shield by DFRobot and a custom midi input / micro SD card reader shield.

Uses the following Arduino libraries:

#include <MIDI.h> // Forty Seven Effects Midi Library

#include <SPI.h> // https://www.arduino.cc/en/reference/SPI

#include "SdFat.h" // https://github.com/greiman/SdFat

#include "FreeStack.h" // https://github.com/greiman/SdFat/blob/master/src/FreeStack.h

#include <LiquidCrystal.h> 


Build Roadmap:
1. Read from SD Card
2. Assign programs to channels
3. Add screen and navigation
4. Add editor mode
5. Add pages for all editable elements
6. Add save state
7. Add save new patch
