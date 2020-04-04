# GENajam v0.3
Front end editor for GenMDM by Littlescale

A physical front end interface for GenMDM for Mega Drive. This hardware device will allow you to access all FM settings for the 6 channels offered by GenMDM as well as load TFI files from SD card and assign them per channel. Currently supports channels 1-6 for FM. PCM mode is not supported. Saving is not supported at the moment.

This open source program is written for the Arduino Leonardo or MEGA 2560 rev 3. The pinout is the same on both devices.

Currently uses the LCD Keypad Shield by DFRobot, a standard mini SD card reader and Fourty Seven Effects midi input and output. It will has 4 potentiometers to control your 4 operators.

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

# Instructions

Load the TFI samples under the TFI folder in this GITHUB to the root of an SD card formatted for Arduino. You can currently load 32 files maximum. The INO file is currently set up for an NTSC tuning mode. To change this to PAL tuning mode, edit line 175:

```
175 MIDI.sendControlChange(83,65,1); // set GENMDM to NTSC
```
or
```
175 MIDI.sendControlChange(83,0,1); // set GENMDM to PAL
```

## MONO | Preset Mode

This mode allows you to navigate up and down between the MIDI channels 1 to 6 and forward and backwards through your TFI files from the SD card. Loading a new TFI file completely overwrites the FM settings on that channel.

## MONO | FM Edit

Once a TFI setting has been chosen, enter this mode to edit the raw FM parameters. Up and down navigates the MIDI channels 1 to 6 and left and right scrolls through the parameters. Potentimeters 1-4 control the Operaters 1-4 respectively.

## POLY | Preset Mode

Switching to this mode allows you to load a TFI file to all 6 channels at once and features high and low note priority polyphony. Notes are stolen at random from the center of the chord. When entering this mode, at first, no file will be loaded. Once left or right has been pushed, the TFI file loaded will overwrite all FM settings on all channels.

## POLY | FM Edit

Once a TFI setting has been chosen, enter this mode to edit the raw FM parameters. Left and right scrolls through the parameters. Potentimeters 1-4 control the Operaters 1-4 respectively and affects all 6 channels at once.

# Current state

version 0.3 - Have updated the project to Leonardo for expanded dynamic memory. The program is now fully functional, allowing you to load TFI files to each channel in MONO mode or to all 6 channels at once in POLY mode. You can then go into FM edit mode and start editing the FM patch.

version 0.2 - Can load up to 32 TFI instrument files off the root on an SD card and you can select instruments for channels 1-6 individually. Can switch modes. FM edit mode doesn't work yet. Poly preset mode will load the TFI file to all 6 channels. When in this mode, first enter the mode and then press left or right. This will initialise the loading of the TFI file and overwrite all channels with the TFI on the screen.

version 0.1 - Can load up to 64 TFI instrument files off the root on an SD card and you can select instruments for channels 1-6 individually.

Build Roadmap:
1. Read from SD Card
2. Assign programs to channels
3. Add screen and navigation 
4. Add polyphony
5. Add editor mode
6. Add pages for all editable elements <===
7. Add save state
8. Add save new patch
