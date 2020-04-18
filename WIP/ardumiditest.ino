
#include <MIDI.h>
#include <LiquidCrystal.h>

// Simple tutorial on how to receive and send MIDI messages.
// Here, when receiving any message on channel 4, the Arduino
// will blink a led and play back a note for 1 second.

MIDI_CREATE_DEFAULT_INSTANCE();

static const unsigned ledPin = 13;      // LED pin on Arduino Uno

// include the library code:


//LCD pin to Arduino
const uint8_t pin_RS = 8; 
const uint8_t pin_EN = 9; 
const uint8_t pin_d4 = 4; 
const uint8_t pin_d5 = 5; 
const uint8_t pin_d6 = 6; 
const uint8_t pin_d7 = 7; 
const uint8_t pin_BL = 10; 
LiquidCrystal lcd( pin_RS,  pin_EN,  pin_d4,  pin_d5,  pin_d6,  pin_d7);

uint8_t lcd_key     = 0;
uint16_t adc_key_in  = 0;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

int potPin1 = A1;
int potPin2 = A2;
int potPin3 = A3;
int potPin4 = A4;



uint16_t readvalue;
uint8_t cc;

int mode = 3;

// polyphony settings
uint8_t polynote[6] = {0, 0, 0, 0, 0, 0}; // what note is in each channel
bool polyon[6] = {0, 0, 0, 0, 0, 0}; // what channels have voices playing
bool sustainon[6] = {0, 0, 0, 0, 0, 0}; // what channels are sustained
bool noteheld[6] = {0, 0, 0, 0, 0, 0}; // what notes are currently held down
uint8_t noteson = 0; // how many notes are on
uint8_t sustainextra = 0; // how many notes are added in sustain
uint8_t lowestnote = 0; // don't steal the lowest note
bool sustain = 0; // is the pedal on


void setup()
{
// set up the LCD's number of columns and rows:
lcd.begin(16, 2);
lcd.clear();

pinMode(potPin1, INPUT);
pinMode(potPin2, INPUT);
pinMode(potPin3, INPUT);
pinMode(potPin4, INPUT);

    MIDI.begin(MIDI_CHANNEL_OMNI);
    MIDI.turnThruOff();    
               
  MIDI.setHandleNoteOn(MyHandleNoteOn);
  MIDI.setHandleNoteOff(MyHandleNoteOff);
  MIDI.setHandleControlChange(MyHandleCC);
  MIDI.setHandlePitchBend(MyHandlePitchbend);


}

void loop()
{

MIDI.read();

/*
lcd.setCursor(13,0);
if (lowestnote<100) lcd.print("0");
if (lowestnote<10) lcd.print("0");
lcd.print(lowestnote);
*/

lcd.setCursor(13,1);
if (noteson+sustainextra<100) lcd.print("0"); MIDI.read();
if (noteson+sustainextra<10) lcd.print("0"); MIDI.read();
lcd.print(noteson+sustainextra);

 lcd.setCursor(13,0);

if (sustain==0) lcd.print("   ");
if (sustain==1) lcd.print("SUS");


if (noteson+sustainextra<6) 
{
lcd.setCursor(12,0); MIDI.read();
lcd.print(" ");  
}
else
{
lcd.setCursor(12,0); MIDI.read();
lcd.print("*");    
}

lcd.setCursor(0,0);

for (int i = 0; i <= 2; i++)
{
  if (polynote[i]<10) lcd.print("0");
  lcd.print(polynote[i]);
  if (noteheld[i]==1)
  {
  lcd.print(".");  
  }
  else
  {
  lcd.print(" ");  
  }
  MIDI.read();  
}

lcd.setCursor(0,1);

for (int i = 3; i <= 5; i++)
{
  if (polynote[i]<10) lcd.print("0");
  lcd.print(polynote[i]);
  if (polyon[i]==1)
  {
  lcd.print(".");  
  }
  else
  {
  lcd.print(" ");  
  }
  MIDI.read();  
}

/*
adc_key_in = analogRead(0);      // read the value from the sensor
  
lcd.clear();

lcd.setCursor(0,0);

lcd.print(adc_key_in);



readvalue = analogRead(potPin1);
cc = readvalue>>3;
if (cc<100) lcd.print("0");
if (cc<10) lcd.print("0");
lcd.print(cc);
lcd.print(" ");

readvalue = analogRead(potPin2);
cc = readvalue>>3;
if (cc<100) lcd.print("0");
if (cc<10) lcd.print("0");
lcd.print(cc);
lcd.print(" ");

readvalue = analogRead(potPin3);
cc = readvalue>>3;
if (cc<100) lcd.print("0");
if (cc<10) lcd.print("0");
lcd.print(cc);
lcd.print(" ");

readvalue = analogRead(potPin4);
cc = readvalue>>3;
if (cc<100) lcd.print("0");
if (cc<10) lcd.print("0");
lcd.print(cc);
lcd.print(" ");
*/

} //loop


void MyHandleNoteOn(byte channel, byte pitch, byte velocity) {

// thank impbox for this formula, a super nice velocity curve :D
velocity = (int)(pow((float)velocity / 127.0f, 0.2f) * 127.0f);

bool repeatnote=0;

if (mode==3 || mode==4) // if we're in poly mode
{

  // here's a fun feature of sustain, you can hit the same note twice
  // let's turn it off and on again here
  
  for (int i = 0; i <= 5; i++) // now scan the current note array for repeats
  {
    if (pitch==polynote[i]) // if the incoming note matches one in the array
    {
      MIDI.sendNoteOff(pitch, velocity, i+1); // turn off that old note
      MIDI.sendNoteOn(pitch, velocity, i+1); // play the new note at that channel
      repeatnote=1; // to bypass the rest
      break;
    }
  } 

  if (repeatnote==0)
  {
    if (sustain==0) {noteson++;} // add one to the notes currently held down 
    else {sustainextra++;} // keep track of extra notes added while sustain held 
    
    lowestnote = polynote[0]; // don't steal the lowest note
    
    for (int i = 0; i <= 5; i++) // now scan the current note array for the lowest note
    {
      if (polynote[i]<lowestnote && polynote[i]!=0) lowestnote=polynote[i];
    } 
  
    if (noteson+sustainextra < 6) // if less than 6 notes are playing
    {
      for (int i = 0; i <= 5; i++) // voice scan to check for voices that are free
      {
        if (polyon[i]==0) // if the voice isn't on
        {
          polyon[i]=1; // turn voice on
          MIDI.sendNoteOn(pitch, velocity, i+1);
          polynote[i] = pitch; // save the pitch of the note against the voice number
          noteheld[i] = 1; // the note is being held
          break;  
        }
      }
  
    }
    else // time to note steal if there are more than 6 notes playing
    {  
      long randchannel = random(0,6); // pick a random channel to switch
  
      if (polynote[randchannel]==lowestnote) // if in your randomness, you chose the lowest note
      {
        randchannel++; // next channel
        if (randchannel==6) randchannel=0; // loop around
      }
  
      // if there are notes being held, but they were turned off at some point, scan for empty voice slots
      for (int i = 0; i <= 5; i++)
      {
        if (polynote[i]==0) {
        randchannel=i; // fill the empty channel
        break;
        }     
      }
            
      MIDI.sendNoteOff(polynote[randchannel], velocity, randchannel+1); // turn off that old note
      MIDI.sendNoteOn(pitch, velocity, randchannel+1); // play the new note at that channel
      polynote[randchannel] = pitch; // save the new pitch of the note against the voice number
      polyon[randchannel]=1; // turn it on just in case
      noteheld[randchannel] = 1; // the key is currently held
    }  
  } // if repeatnote

} // if mode 3
else // otherwise, just revert to midi thru
{
  MIDI.sendNoteOn(pitch, velocity, channel);  
}
} // void note on


void MyHandleNoteOff(byte channel, byte pitch, byte velocity) {
// note: channel here is useless, as it's getting the channel from the keyboard 

if (mode==3 || mode==4) // if we're in poly mode
{    

  if (sustain==0) // if the sustain pedal isn't being held
  {
    if (noteson!=0) noteson--; // take one from the notes being played but don't let it go negative
  }

  for (int i = 0; i <= 5; i++) // we know the note but not the channel
  {
    if (pitch==polynote[i]) // if the pitch matches the note that was triggered off...
    {
      if (sustain==1) // if the sustain pedal is held
      {
        sustainon[i] = 1; // turn on sustain on that channel
        noteheld[i] = 0; // the key is no longer being held down
        break;   
      }
      else
      {
        polyon[i]=0; // turn voice off
        MIDI.sendNoteOff(pitch, velocity, i+1); // turn that voice off against it's channel
        polynote[i] = 0; // clear the pitch on that channel
        noteheld[i] = 0; // the key is no longer being held down
        break;   
      }
    }
  } 

} // if mode 3
else // otherwise, just revert to midi thru
{
  MIDI.sendNoteOff(pitch, velocity, channel);  
}
} // void note off


void MyHandleCC(byte channel, byte number, byte value) {
  if (number==64)
  {
    if (value==0)
    {
      sustain=0;
      for (int i = 0; i <= 5; i++) // scan for sustained channels
      {
        if (noteheld[i]==0) // if the key is not currently being pushed
        {
          if (noteson>=sustainextra)
          {
            noteson = noteson-sustainextra; // take away the extra sustain notes
          }
          else
          {
            noteson=0;
          }
          sustainextra=0; // reset the sustain extras
          sustainon[i]=0; // turn off sustain on that channel
          polyon[i]=0; // turn voice off
          MIDI.sendNoteOff(polynote[i], 0, i+1); // turn that voice off against it's channel
          polynote[i] = 0; // clear the pitch on that channel  
        }
      }
    }
    else
    {
      sustain=1;
    }
  }
  
} // void cc

void MyHandlePitchbend(byte channel, int bend)
{
  MIDI.sendPitchBend(bend, channel);
} // void pitch bend
