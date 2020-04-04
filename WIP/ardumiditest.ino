
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
uint8_t polynote[6] = {0, 0, 0, 0, 0, 0};
bool polyon[6] = {0, 0, 0, 0, 0, 0};
uint8_t noteson = 0;
uint8_t lowestnote = 0; // don't steal the lowest note

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
               
  MIDI.setHandleNoteOn(MyHandleNoteOn); // This is important!! This command
  // tells the Midi Library which function you want to call when a NOTE ON command
  // is received. In this case it's "MyHandleNoteOn".
  MIDI.setHandleNoteOff(MyHandleNoteOff); // This command tells the Midi Library 
  // to call "MyHandleNoteOff" when a NOTE OFF command is received.

}

void loop()
{

MIDI.read();

lcd.setCursor(14,1);
if (noteson<10) lcd.print("0"); MIDI.read();
lcd.print(noteson);

if (noteson<7) 
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

/*
velocity = velocity*2;
if (velocity > 127) velocity = 127;
*/

lcd.setCursor(13,0);
if (velocity<100) lcd.print("0");
if (velocity<10) lcd.print("0");
lcd.print(velocity);

if (mode == 3) // if we're in poly mode
{  
  
  noteson++; // add one to the notes currently held down

  if (noteson < 7) // if less than 7 notes are playing
  {
    
    for (int i = 0; i <= 5; i++) // voice scan to check for voices that are free
    {
      if (polyon[i]==0) // if the voice isn't on
      {
        polyon[i]=1; // turn voice on
        MIDI.sendNoteOn(pitch, velocity, i+1);
        polynote[i] = pitch; // save the pitch of the note against the voice number
        break;  
      }
    }

  }
  else // time to note steal if there are more than 6 notes playing
  {  
    int randchannel = random(5); // fuck if for now we're doing random off polyphony
    int highestnote = 0; // use the highest note instead
    int highestchannel = 0;
    for (int i = 0; i <= 5; i++) // but first check for off notes
    {
      if (lowestnote<polynote[i]) lowestnote=polynote[i];
      if (polynote[i]>highestnote) { highestnote=polynote[i]; highestchannel=i; }
      if (polyon[i]==0) randchannel=i;
    }
    int randnote = polynote[randchannel];
    if (randnote<lowestnote) // if the random note chosen was the lowest note
    {
      randchannel = highestchannel;
    }
    polyon[randchannel]=1; // turn it on just in case
    
    MIDI.sendNoteOff(randnote, velocity, randchannel+1); // turn off that old note
    MIDI.sendNoteOn(pitch, velocity, randchannel+1); // play the new note at that channel
    polynote[randchannel] = pitch; // save the new pitch of the note against the voice number
    // no need to turn the voice indicator on
  }

} // if mode 3
else // otherwise, just revert to midi thru
{
  MIDI.sendNoteOn(pitch, velocity, channel);  
}
} // void note on


void MyHandleNoteOff(byte channel, byte pitch, byte velocity) {
// note: channel here is useless, as it's getting the channel from the keyboard 

if (mode == 3) // if we're in poly mode
{    

  if (noteson!=0) noteson--; // take one from the notes being played but don't let it go negative

  for (int i = 0; i <= 5; i++) // we know the note but not the channel
  {
    if (pitch==polynote[i]) // if the pitch matches the note that was triggered off...
    {
      polyon[i]=0; // turn voice off
      MIDI.sendNoteOff(pitch, velocity, i+1); // turn that voice off against it's channel
      polynote[i] = 0; // clear the pitch on that channel
      break;  
    }
  } 

} // if mode 3
else // otherwise, just revert to midi thru
{
  MIDI.sendNoteOff(pitch, velocity, channel);  
}
} // void note off
