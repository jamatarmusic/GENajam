# GENajam v1.10
Front end editor for GenMDM by Littlescale

Overview video: https://www.youtube.com/watch?v=uE3FbmMKl-U

Connection guide (and history): https://www.youtube.com/watch?v=WcB8032SWHI

To see an example of what this can do: https://www.youtube.com/watch?v=_nLA5kXui2M
*This is a live improv GENajam set played over some multi-tracked GENajam recorded material.*

A physical front end interface for GenMDM for Mega Drive built by Catskull Electronics. This hardware device will allow you to access all FM settings for the 6 channels offered by GenMDM as well as load and save TFI files from SD card and assign them per channel. Currently supports channels 1-6 for FM and has full 6 note Polyphony with last note priority and low note priority. There is also sustain pedal support and the modulation wheel now activates the LFO. This makes it extremely musical to play chords and pads. GENajam will also gives a very piano like velocity curve, allowing the instruments to be played with ease.

Note: PCM mode switching is not supported by Genajam but can be still enabled in MONO mode. There is a bug in GENMDM where Decay 2 only works on OP1.

GENajam is written in Arduino and runs on the MightyCore ATMega1284 (https://github.com/MCUdude/MightyCore). It uses the MIDI library by FortySevenEffects (https://github.com/FortySevenEffects/arduino_midi_library) and the SDfat library by Greiman (https://github.com/greiman/SdFat).

# Instructions

There is a folder of sample TFI files that you can test by loading them to the root of an SD card. Load the TFI samples under the TFI folder in this GITHUB to the root of an SD card formatted for Arduino. You can currently load a maximum of 999 files.

You can find more TFI files here: https://little-scale.blogspot.com/2013/02/all-sega-genesis-mega-drive-tfi.html

GENajam launches in POLY mode, giving you 6 notes of polyphony over channel 1. Press the **LEFT** and **RIGHT** buttons to scroll through the presets.

## Buttons and Functions

Note, these buttons refer to the v1.10 firmware.

**PRESET / FM EDIT** - This button swaps between preset mode and FM editing mode so you can quickly navigate to a patch and jump into editing it's parameters.

**LEFT AND RIGHT** - These buttons will navigate left and right through presets or FM parameters.

**MONO / POLY** - This button switches between monophonic or polyphonic mode. **Please note**, switching from MONO to POLY mode will **not** overwrite all channels with an instrument file **until you press LEFT or RIGHT**. This is a safety feature in case you have programmed all your channels in MONO mode and accidentally switch to POLY.

**UP AND DOWN - IN POLY** - While in POLY mode, these buttons will enter the **SAVE MENU** giving the option to push **up** to overwrite the current patch or **down** to save the patch as a new file. The new patch will be named "newpatch###.tfi" and will count up sequencially. To rename patches, insert your sd card into a computer. To cancel out of the **SAVE MENU**, push the left, right or preset/fm edit button.

**DELETE BUTTON** - This unmarked button will give you a prompt to delete a patch. This cannot be undone. Press again to confirm or cancel by pressing the left, right or preset/fm edit button.

**UP AND DOWN - IN MONO** - While in MONO mode in either preset mode or FM edit mode, you can navigate from channels 1 to 6 using these buttons.

**OP1, OP2, OP3 and OP4 KNOBS** - These knobs activate when you are in FM EDIT mode. The knobs will not activate unless you start to turn them, so you can scroll through FM parameters while retaining settings. For most parameters, you will have a value of 0 to 127.

**LFO ACTIVATE** - The LFO on and off is assigned to MOD WHEEL (CC ch 1). To change types of LFO, head to Page **13 - LFO/FM/AM** in FM editing. To turn on Amplitude Modulation, go to page **12 - Amp Mod** and set to **ON** on all the operators you want it on, then dial up AM on Page 13.

**STEREO SPREAD MODE** - There is a stereo spread poly mode on PAGE 01 of FM edit mode. For each note, left, right or center channel is picked randomly for each key press. This gives a very pleasing stereo spread to pads and epianos. First select a patch in POLY mode. Then press the **PRESET / FM EDIT** button to enter FM editing. Navigate to page **01:Alg FB Pan**. The OP1 knob will switch the spread icon from OFF to ON.

## Boot menu

New to v1.10, to access the boot menu, start with your Genajam unplugged. Hold down the **Preset / FM Edit** button and plug in the USB-C cable. Continue to hold down the button until "MIDI CH / REGION" appears on the screen. The settings will snap to whatever the knobs are set to. Choose your MIDI CHANNEL (for using POLY mode) using the OP1 knob and choose your REGION using the OP4 knob. Push the left, right or preset/fm edit button to save the settings to EEPROM. These settings will be remembered even after the unit is reset or turned off.

## MONO Mode

This mode allows you to navigate up and down between the MIDI channels 1 to 6 and forward and backwards through your TFI files from the SD card. Loading a new TFI file completely overwrites the FM settings on that channel.

Once a TFI setting has been chosen, enter this mode to edit the raw FM parameters. Up and down navigates the MIDI channels 1 to 6 and left and right scrolls through the parameters. Potentimeters 1-4 control the Operaters 1-4 respectively.

## POLY Mode

Switching to this mode allows you to load a TFI file to all 6 channels at once and features high and low note priority polyphony. Notes are stolen at random from the center of the chord. When entering this mode, at first, no file will be loaded. Once left or right has been pushed, the TFI file loaded will overwrite all FM settings on all channels.

Once a TFI setting has been chosen, enter this mode to edit the raw FM parameters. Left and right scrolls through the parameters. Potentimeters 1-4 control the Operaters 1-4 respectively and affects all 6 channels at once.

## FM editing tips

There is some in-depth reading into the FM parameters of the YM2612 in the Mega Drive on this page:
https://www.smspower.org/maxim/Documents/YM2612

However, if you are slightly familiar with FM editing, here are some quick tips on the implementation in GENajam:

**Operators and Algorithms** - GENajam lets you edit all 4 operators at once, giving you powerful simultanious access over things such as operator level, frequency multipliers, attack and release very quickly to sculpt your sound live. An important note about the operators in the Mega Drive is that usually OP4 is the **first** operator in the series, unlike a Yamaha Reface DX or Yamaha DX7. This means that when you head into editing, especially in the TL (total level) section, you want to start with OP4 as the first carrier and then work backwards. OP4 is usually the primary oscillator and reducing this operator to 0 will mute the sound.

For example, algorithm 0 will flow like this: OP1 -> OP2 -> OP3 -> OP4

A good way to think of this is that OP4 is the oscilator. Turn down OP1, 2 and 3 and turn up OP4 and you'll hear a pure sine wave. Bring in OP3 and you'll hear OP4 being modulated by OP3. Bring in OP2 and it will modulate OP3 and OP4. Bring in OP1 and you'll have a stack of modulation.

Check out the SMSPower page to see all the algorithms.

**Feedback** - Feedback is only enabled on OP1, so if that operator is turned down, you won't hear anything happening. I find that OP1 has to be at full 127 volume (TL) most of the time.

**Pan** - The YM2612 only supports hard left, hard right and center pan. Pan at values lower than 32 will simply mute.

**TL or Volume** - This is simply the volume of each operator (and the amount it modulates the operator before it). As I said before, make sure OP4 is cranked as it's usually the oscillator at the start of the chain.

**Multiple** - This is the multiplification factor on the frequency, or a kind of octave selector for the operator. The YM2612 has some nice pitched multipliers, so it's not just like a oscillator tuning amount, it gives you some nice tuning in musical steps.

**Rate Scaling** - This is how much envelope should be scaled as you move up the keyboard. In simple terms, you could have a sound with a slow attack, decay and release and as you play higher up the keyboard, this amount gets shorted.

**Detune** - Honestly, this is so subtle that you might turn this knob and be scratching your head, wondering if it does anything.

**Attack, Decay 1, Decay 2, Release, TL2** - The Yamaha envelope is kinda unique. Attack, Decay and Release are as expected. However, you get a second Decay to play with and TL2 should really be called "Sustain". If you check out the SMSpower page, you'll see a great diagram of the envelope. The values are actually variable angles at each part of the envelope and may behave differently than your standard subtractive synth. For example, a low attack value is actually slow and a high value is fast. Same goes for release and decay. TL2 is just a volume control instead of an angle and annoyingly, the value is subtractive, so a low value means not much drop in volume at sustain and a high amount means lots of drop in volume at sustain. Another un-fun fact is there is a hard-coded bug in GENMDM where Decay 2 across all operators route to OP1. That means Decay 2 is permenantly set to 64 for OP2, OP3 and OP4 and can't be changed.

**SSG-EG** - One of the most unique sections of the YM2612 is the SSG-EG section which basically lets you loop the envelope in interesting ways which almost makes EG section into an amplitude LFO. If means you could have a slow attack and fast release, have the envelope loop into a kind of sawtooth pad. Because the YM2612 is multi-timbral, you can have an looped fake LFO *per note* which is really cool and something that isn't in a lot of synths. There a bit more info here: https://plutiedev.com/ym2612-registers#reg-90

**Amplitude Modulation** - This is off by default on each patch. If you turn AMP MOD on, you'll notice a volume dip. The LFO has to be activate by moving your mod wheel or CC1 above 64.

**LFO/FM/AM** - This section lets you chose the speed of the LFO and the intensity on either frequency modulation (warbly cool stuff) or amplitude modulation (tremelo). The AM section will not do anything until it is switched to "ON" on page 12. If you want only amplitude modulation, make sure you turn down FM to zero.

**VOICES and Polyphony** - The YM2612 is multi-timbral. That means that while in POLY mode it loads all patches and settings across to all 6 channels at once, you can also switch over to MONO mode, set each channel differently, switch back to POLY mode (without pressing left and right) and play all 6 voices which different settings in POLY mode which is really cool. Try load different presets into channels 1 - 6 in MONO mode and then press POLY mode. I wrote the polyphony engine to have random note stealing and have low note priority. This means chords should sound really full if you hold something with the sustain pedal and play a melody over a held chord.

Thanks for checking out my project! You can check out my music at http://www.jamatar.com :)
