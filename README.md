# genajam
Front end editor for GenMDM by Littlescale

A physical front end interface for GenMDM for Mega Drive. This hardware device will allow you to access all FM settings for the 6 channels offered by GenMDM as well as load TFI files from SD card and assign them per channel.

This open source program is written for the Arduino Uno or Nano.

Currently uses the LCD Keypad Shield by DFRobot, a standard mini SD card reader and Fourty Seven Effects midi input and output. It will be expanded with 4 potentiometers for controlling operators 1-4 on each channel.

SD card CS pin goes to pin 3. Other pins as standard here:

https://create.arduino.cc/projecthub/electropeak/sd-card-module-with-arduino-how-to-read-write-data-37f390

Use this guide for the midi inputs and outputs:

https://www.notesandvolts.com/2015/02/midi-and-arduino-build-midi-input.html

https://www.notesandvolts.com/2015/03/midi-for-arduino-build-midi-output.html

For the LCD Keypad shield that I used:

https://wiki.dfrobot.com/Arduino_LCD_KeyPad_Shield__SKU__DFR0009_


Uses the following Arduino libraries:

#include <MIDI.h> // Forty Seven Effects Midi Library

#include <SPI.h> // https://www.arduino.cc/en/reference/SPI

#include "SdFat.h" // https://github.com/greiman/SdFat

#include "FreeStack.h" // https://github.com/greiman/SdFat/blob/master/src/FreeStack.h

#include <LiquidCrystal.h> // standard liquid crystal arduino library


There is a folder of sample TFI files that you can test by loading them to the root of an SD card.

Current state:

version 0.2 - Can load up to 32 TFI instrument files off the root on an SD card and you can select instruments for channels 1-6 individually. Can switch modes. FM edit mode doesn't work yet. Poly preset mode will load the TFI file to all 6 channels. When in this mode, first enter the mode and then press left or right. This will initialise the loading of the TFI file and overwrite all channels with the TFI on the screen.

version 0.1 - Can load up to 64 TFI instrument files off the root on an SD card and you can select instruments for channels 1-6 individually.

Build Roadmap:
1. Read from SD Card
2. Assign programs to channels
3. Add screen and navigation 
4. Add polyphony <===
5. Add editor mode
6. Add pages for all editable elements
7. Add save state
8. Add save new patch
