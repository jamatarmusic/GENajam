// GENajam v0.2
// --------------------
// This is a front end for Litte-scale's GENMDM module for Mega Drive

#include <MIDI.h>  // Midi Library
#include <SPI.h>
#include "SdFat.h"
#include "FreeStack.h"
#include <LiquidCrystal.h>

//-------------------------------------------
//LCD pin to Arduino
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

uint8_t lcd_key = 0;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

byte emojichannel[] = {
  B01110,
  B01000,
  B01110,
  B00000,
  B01010,
  B01110,
  B01010,
  B00000
};

byte emojiprogram[] = {
  B11100,
  B10100,
  B11100,
  B10000,
  B00111,
  B00101,
  B00110,
  B00101
};


byte emojipoly[] = {
  B11100,
  B10100,
  B11100,
  B10000,
  B00101,
  B10101,
  B10010,
  B11010
};


//debouncing

unsigned long buttonpushed = 0;      // when a button is pushed, mark what millis it was pushed
const uint16_t debouncedelay = 200;   //the debounce time which user sets prior to run
unsigned long messagestart = 0; // when a message starts, mark what millis it displayed at
const uint16_t messagedelay = 1000; // how long to display messages for
uint8_t refreshscreen = 0; // trigger refresh of screen but only once

//-------------------------------------------

// SD card chip select pin.
const uint8_t SD_CS_PIN = 3;

SdFat sd;
SdFile tfifile;
SdFile dirFile;

const uint8_t MaxNumberOfChars = 17; // only show 16 characters coz LCD

// Number of files found.
uint16_t n = 0;

// How many files to handle
const uint16_t nMax = 32;

// Position of file's directory entry.
uint16_t dirIndex[nMax];

//----------------------------------------------

// Create an instance of the library with default name, serial port and settings
MIDI_CREATE_DEFAULT_INSTANCE();

// start position of each channel file cursor
int tfifilenumber[6] = {0, 0, 0, 0, 0, 0};

//set the initial midi channel
int tfichannel=1;

// switch between settings
uint8_t mode=1;
// 1) mono presets
// 2) mono fm settings
// 3) poly presets
// 4) poly fm settings

// polyphony settings
uint8_t polynote[6] = {0, 0, 0, 0, 0, 0};
bool polyon[6] = {0, 0, 0, 0, 0, 0};
uint8_t noteson = 0;
uint8_t lowestnote = 0; // don't steal the lowest note

// fm parameter screen navigation
uint8_t fmscreen=1;

// global variable for storing FM settings for each channel
uint8_t fmsettings[6][50];

uint8_t potPin1 = A1;
uint8_t potPin2 = A2;
uint8_t potPin3 = A3;
uint8_t potPin4 = A4;

uint16_t currentpotvalue;
uint16_t prevpotvalue[4];
uint8_t cc;

void setup()
{
    
    //setup the input pots
    pinMode(potPin1, INPUT);
    pinMode(potPin2, INPUT);
    pinMode(potPin3, INPUT);
    pinMode(potPin4, INPUT);
    
    lcd.begin(16, 2);

    lcd.createChar(0, emojichannel);
    lcd.createChar(1, emojiprogram);
    lcd.createChar(2, emojipoly);


    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("JamaGEN START!");
    lcd.setCursor(0,1);
    lcd.print("version 0.1");
  
  
  // Serial.begin(9600); // open the serial port at 9600 bps for debug
  // while (!Serial) {}
  // delay(1000);

    if (!sd.begin(SD_CS_PIN, SD_SCK_MHZ(50))) {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("ERROR:");
      lcd.setCursor(0,1);
      lcd.print("CANNOT FIND SD");
      sd.initErrorHalt();
    }

  MIDI.begin(MIDI_CHANNEL_OMNI); // read all channels
  MIDI.turnThruOff(); // turn off soft midi thru

  MIDI.setHandleNoteOn(MyHandleNoteOn);
  MIDI.setHandleNoteOff(MyHandleNoteOff); 
  
  MIDI.sendControlChange(83,65,1); // set GENMDM to NTSC

 // List files in root directory and get max file number
  if (!dirFile.open("/", O_RDONLY)) {
      lcd.setCursor(0,0);
      lcd.print("ERROR:");
      lcd.setCursor(0,1);
      lcd.print("NO ROOT DIR");
      sd.errorHalt("open root failed");
  }
  while (n < nMax && tfifile.openNext(&dirFile, O_RDONLY)) {

    // Skip directories and hidden files.
    if (!tfifile.isSubDir() && !tfifile.isHidden()) {

      // Save dirIndex of file in directory.
      dirIndex[n] = tfifile.dirIndex();

      // Count the files
      n++;
    }
    tfifile.close();
  }

  delay(700);

  //initialise all channels

  tfichannel=6;
  tfiselect();
  tfichannel=5;
  tfiselect();
  tfichannel=4;
  tfiselect();
  tfichannel=3;
  tfiselect();
  tfichannel=2;
  tfiselect();
  tfichannel=1;
  tfiselect();

  //test tone all channnels

/*
    MIDI.sendNoteOn(60, 127, 1);
    MIDI.sendNoteOn(62, 127, 2);
    MIDI.sendNoteOn(63, 127, 3);
    MIDI.sendNoteOn(65, 127, 4);
    MIDI.sendNoteOn(67, 127, 5);
    MIDI.sendNoteOn(68, 127, 6);
    delay(1000);
    MIDI.sendNoteOff(60, 127, 1);
    MIDI.sendNoteOff(62, 127, 2);
    MIDI.sendNoteOff(63, 127, 3);
    MIDI.sendNoteOff(65, 127, 4);
    MIDI.sendNoteOff(67, 127, 5);
    MIDI.sendNoteOff(68, 127, 6); 
 */   
  
} // void setup

void loop()
{

MIDI.read();

  lcd_key = read_LCD_buttons();

  modechangemessage(); // if the mode has just been changed, show message temporarily

  // set the mode depending on cycle of mode button
  
  switch(mode) {
  
  //======================================================= MODE 1
  case 1:
  {
      switch (lcd_key) // depending on which button was pushed, we perform an action
      {
      case btnRIGHT:
      {
      tfifilenumber[tfichannel-1] = tfifilenumber[tfichannel-1]+1;
      if(tfifilenumber[tfichannel-1]==(n)){ // if max files exceeded, loop back to start
      tfifilenumber[tfichannel-1]=0;
      }   
      tfiselect();
      break;
      }
      
      case btnLEFT:
      {
      tfifilenumber[tfichannel-1] = tfifilenumber[tfichannel-1]-1;
      if(tfifilenumber[tfichannel-1]==-1){ // if min files exceeded, loop back to end
      tfifilenumber[tfichannel-1]=n-1;
      }
      tfiselect();
      break;
      }
      
      case btnUP:
      {
      tfichannel=tfichannel+1;
      if(tfichannel==(7)){ // if max channels reached, loop around
      tfichannel=1;
      }
      channelselect();
      break;
      }
      
      case btnDOWN:
      {
      tfichannel=tfichannel-1;
      if(tfichannel==(0)){ // if max channels reached, loop around
      tfichannel=6;
      }
      channelselect();
      break;
      }
      
      case btnSELECT:
      {
      modechange();
      break;
      }
      
      } 
      
  break;  
  }
  
  
  //======================================================= MODE 2
  case 2:
  {
      
      switch (lcd_key) // depending on which button was pushed, we perform an action
      {
      case btnRIGHT:
      {
      fmscreen = fmscreen+1;
      if(fmscreen==14) fmscreen=1;   
      fmparamdisplay();
      break;
      }
      
      case btnLEFT:
      {
      fmscreen = fmscreen-1;
      if(fmscreen==0) fmscreen=13;   
      fmparamdisplay();
      break;
      }
      
      case btnUP:
      {
      tfichannel=tfichannel+1;
      if(tfichannel==(7)){ // if max channels reached, loop around
      tfichannel=1;
      }
      fmparamdisplay();
      break;
      }
      
      case btnDOWN:
      {
      tfichannel=tfichannel-1;
      if(tfichannel==(0)){ // if max channels reached, loop around
      tfichannel=6;
      }
      fmparamdisplay();
      break;
      }
      
      case btnSELECT:
      {
      modechange();
      break;
      }
      
      }

  operatorparamdisplay(); // check in with the pots to see if they've been moved, update the display and send CC
  
  break;  
  }


  //======================================================= MODE 3
  case 3:
  {
      switch (lcd_key) // depending on which button was pushed, we perform an action
      {
      case btnRIGHT:
      {
      tfichannel=1;
      tfifilenumber[tfichannel-1] = tfifilenumber[tfichannel-1]+1;
      if(tfifilenumber[tfichannel-1]==(n)){ // if max files exceeded, loop back to start
      tfifilenumber[tfichannel-1]=0;
      }
      for (int i = 1; i <= 5; i++) {
        tfifilenumber[i] = tfifilenumber[0];
      }
      for (int i = 1; i <= 6; i++) {
        tfichannel=i;
        tfiselect();
      }      
      break;
      }
      
      case btnLEFT:
      {
      tfichannel=1;
      tfifilenumber[tfichannel-1] = tfifilenumber[tfichannel-1]-1;
      if(tfifilenumber[tfichannel-1]==-1){ // if min files exceeded, loop back to end
      tfifilenumber[tfichannel-1]=n-1;
      }
      for (int i = 1; i <= 5; i++) {
        tfifilenumber[i] = tfifilenumber[0];
      }
      for (int i = 1; i <= 6; i++) {
        tfichannel=i;
        tfiselect();
      }    
      break;
      }
      
      case btnSELECT:
      {  
      modechange();
      break;
      }
      
      } 
      
  break;  
  }

  //======================================================= MODE 4
  case 4:
  {
      
      switch (lcd_key) // depending on which button was pushed, we perform an action
      {
      case btnRIGHT:
      {
      fmscreen = fmscreen+1;
      if(fmscreen==14) fmscreen=1;   
      fmparamdisplay();
      break;
      }
      
      case btnLEFT:
      {
      fmscreen = fmscreen-1;
      if(fmscreen==0) fmscreen=13;   
      fmparamdisplay();
      break;
      }
      
      case btnUP:
      {
      tfichannel=tfichannel+1;
      if(tfichannel==(7)){ // if max channels reached, loop around
      tfichannel=1;
      }
      fmparamdisplay();
      break;
      }
      
      case btnDOWN:
      {
      tfichannel=tfichannel-1;
      if(tfichannel==(0)){ // if max channels reached, loop around
      tfichannel=6;
      }
      fmparamdisplay();
      break;
      }
      
      case btnSELECT:
      {
      modechange();
      break;
      }
      
      }

  operatorparamdisplay(); // check in with the pots to see if they've been moved, update the display and send CC
  
  break;  
  }
  
  } // end mode check

  
} // void loop


int read_LCD_buttons() // function for reading the buttons
{
  uint16_t adc_key_in = 0;
  adc_key_in = analogRead(0);      // read the value from the sensor
  
  if (adc_key_in > 1000) {
  return btnNONE;
  }
  
    if ((millis() - buttonpushed) > debouncedelay) {    // only register a new button push if no button has been pushed in debouncedelay millis
    
      if (adc_key_in < 50) {
      buttonpushed = millis();  
      return btnRIGHT;
      }
      
      if (adc_key_in < 195) {
      buttonpushed = millis();  
      return btnUP;
      }
      
      if (adc_key_in < 380) {
      buttonpushed = millis();  
      return btnDOWN;
      }
      
      if (adc_key_in < 555) {
      buttonpushed = millis();  
      return btnLEFT;
      }
      
      if (adc_key_in < 790) {
      buttonpushed = millis();  
      return btnSELECT;
      }
    
    }

  return btnNONE;  // when all others fail, return this...

}

void modechange() // when the mode button is changed, cycle the modes
{
  mode++;
  if(mode > 3) mode = 1; // loop mode

  switch(mode){
    case 1:
    {
      messagestart = millis();
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("MONO Preset");
      refreshscreen=1;
      break;
    }
    
    case 2:
    {
      messagestart = millis();
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("MONO FM Edit");
      refreshscreen=1;
      break;
    }
    
    case 3:
    {
      messagestart = millis();
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("POLY Preset");
      refreshscreen=1;
      break;
    }

    case 4:
    {
      messagestart = millis();
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("POLY FM Edit");
      refreshscreen=1;
      break;
    }
    
  } // switch
}

void modechangemessage() // display temporary message
{
// if a mode switch message has been displayed for long enough, refresh the screen
  
if ((millis() - messagestart) > messagedelay && refreshscreen == 1) { 
  switch(mode){
    case 1:
    {
      channelselect(); // just reload the channel info for the current channel
      refreshscreen=0;
      break;
    }

    case 2:
    {
      fmparamdisplay();
      refreshscreen=0;
      break;
    }

    case 3:
    {
      channelselect(); // just reload the channel info for the current channel
      refreshscreen=0;
      break;
    }      
  } // end mode check
} // end message refresh
}

void tfiselect() //load a tfi , send the midi, update screen
{

    if (!tfifile.open(&dirFile, dirIndex[tfifilenumber[tfichannel-1]], O_RDONLY)) {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("ERROR:");
      lcd.setCursor(0,1);
      lcd.print("CANNOT READ TFI ");
      sd.errorHalt(F("open"));
    }

    // open TFI file and read/send contents =================

    // variables for pulling values from TFI files and putting them in an array
    int tfiarray[42];
    int filetoarray = 0; // read the file from the start
    // read from the file until there's nothing else in it:
    while (tfifile.available()) {
      tfiarray[filetoarray]=tfifile.read(), DEC; // read the byte data, convert it to DEC
      filetoarray++;
    } 

    
    tfisend(tfiarray, tfichannel);

     //get filename
    char tfifilename[MaxNumberOfChars + 1];
    tfifile.getName(tfifilename, MaxNumberOfChars);
    tfifilename[MaxNumberOfChars]=0; //ensure  termination
    
    //show filename on screen
    lcd.clear();
    lcd.setCursor(0,0);
    if (mode==3) {  
      lcd.write(byte(2));
      lcd.print(" ");     
    }
    else {
      lcd.write(byte(0));
      lcd.print(tfichannel);  
    }
    lcd.print(" ");
    lcd.write(byte(1));
    if ((tfifilenumber[tfichannel-1]+1)<100) lcd.print("0");
    if ((tfifilenumber[tfichannel-1]+1)<10) lcd.print("0");
    lcd.print(tfifilenumber[tfichannel-1]+1);
    lcd.print("/");
    if (n<100) lcd.print("0");
    if (n<10) lcd.print("0");
    lcd.print(n);
    lcd.setCursor(0,1);
    lcd.print(tfifilename);

   
    // TFI file closed ================
    
  tfifile.close();
  
}


void channelselect() //select a new channel, display current tfi on screen
{
    if (!tfifile.open(&dirFile, dirIndex[tfifilenumber[tfichannel-1]], O_RDONLY)) {
      lcd.setCursor(0,0);
      lcd.print("ERROR:");
      lcd.setCursor(0,1);
      lcd.print("CANNOT READ TFI");
    sd.errorHalt(F("open"));
  }

     //get filename
    char tfifilename[MaxNumberOfChars + 1];
    tfifile.getName(tfifilename, MaxNumberOfChars);
    tfifilename[MaxNumberOfChars]=0; //ensure  termination

    //show filename on screen
    lcd.clear();
    lcd.setCursor(0,0);
    if (mode==3) {  
      lcd.write(byte(2));
      lcd.print(" ");     
    }
    else {
      lcd.write(byte(0));
      lcd.print(tfichannel);  
    }
    lcd.print(" ");
    lcd.write(byte(1));
    if ((tfifilenumber[tfichannel-1]+1)<100) lcd.print("0");
    if ((tfifilenumber[tfichannel-1]+1)<10) lcd.print("0");
    lcd.print(tfifilenumber[tfichannel-1]+1);
    lcd.print("/");
    if (n<100) lcd.print("0");
    if (n<10) lcd.print("0");
    lcd.print(n);
    lcd.setCursor(0,1);
    lcd.print(tfifilename);
   
    // TFI file closed ================
    
  tfifile.close();
  
}

void tfisend(int opnarray[42], int sendchannel)
{
  
      //send all TFI data to appropriate CCs
      
    MIDI.sendControlChange(14,opnarray[0]*16,sendchannel); //Algorithm 
    MIDI.sendControlChange(15,opnarray[1]*16,sendchannel); //Feedback
    
    MIDI.sendControlChange(20,opnarray[2]*8,sendchannel); //OP1 Multiplier
    MIDI.sendControlChange(21,opnarray[12]*8,sendchannel); //OP2 Multiplier
    MIDI.sendControlChange(22,opnarray[22]*8,sendchannel); //OP3 Multiplier
    MIDI.sendControlChange(23,opnarray[32]*8,sendchannel); //OP4 Multiplier

    MIDI.sendControlChange(24,opnarray[3]*32,sendchannel); //OP1 Detune
    MIDI.sendControlChange(25,opnarray[13]*32,sendchannel); //OP2 Detune
    MIDI.sendControlChange(26,opnarray[23]*32,sendchannel); //OP3 Detune
    MIDI.sendControlChange(27,opnarray[33]*32,sendchannel); //OP4 Detune

    MIDI.sendControlChange(16,127-opnarray[4],sendchannel); //OP1 Total Level
    MIDI.sendControlChange(17,127-opnarray[14],sendchannel); //OP2 Total Level
    MIDI.sendControlChange(18,127-opnarray[24],sendchannel); //OP3 Total Level
    MIDI.sendControlChange(19,127-opnarray[34],sendchannel); //OP4 Total Level

    MIDI.sendControlChange(39,opnarray[5]*32,sendchannel); //OP1 Rate Scaling
    MIDI.sendControlChange(40,opnarray[15]*32,sendchannel); //OP2 Rate Scaling
    MIDI.sendControlChange(41,opnarray[25]*32,sendchannel); //OP3 Rate Scaling
    MIDI.sendControlChange(42,opnarray[35]*32,sendchannel); //OP4 Rate Scaling

    MIDI.sendControlChange(43,opnarray[6]*4,sendchannel); //OP1 Attack Rate
    MIDI.sendControlChange(44,opnarray[16]*4,sendchannel); //OP2 Attack Rate
    MIDI.sendControlChange(45,opnarray[26]*4,sendchannel); //OP3 Attack Rate
    MIDI.sendControlChange(46,opnarray[36]*4,sendchannel); //OP4 Attack Rate

    MIDI.sendControlChange(47,opnarray[7]*4,sendchannel); //OP1 1st Decay Rate
    MIDI.sendControlChange(48,opnarray[17]*4,sendchannel); //OP2 1st Decay Rate
    MIDI.sendControlChange(49,opnarray[27]*4,sendchannel); //OP3 1st Decay Rate
    MIDI.sendControlChange(50,opnarray[37]*4,sendchannel); //OP4 1st Decay Rate

    MIDI.sendControlChange(51,opnarray[8]*8,sendchannel); //OP1 2nd Decay Rate
    MIDI.sendControlChange(52,opnarray[18]*8,sendchannel); //OP2 2nd Decay Rate
    MIDI.sendControlChange(53,opnarray[28]*8,sendchannel); //OP3 2nd Decay Rate
    MIDI.sendControlChange(54,opnarray[38]*8,sendchannel); //OP4 2nd Decay Rate

    MIDI.sendControlChange(59,opnarray[9]*8,sendchannel); //OP1 Release Rate
    MIDI.sendControlChange(60,opnarray[19]*8,sendchannel); //OP2 Release Rate
    MIDI.sendControlChange(61,opnarray[29]*8,sendchannel); //OP3 Release Rate
    MIDI.sendControlChange(62,opnarray[39]*8,sendchannel); //OP4 Release Rate

    MIDI.sendControlChange(55,opnarray[10]*8,sendchannel); //OP1 2nd Total Level
    MIDI.sendControlChange(56,opnarray[20]*8,sendchannel); //OP2 2nd Total Level
    MIDI.sendControlChange(57,opnarray[30]*8,sendchannel); //OP3 2nd Total Level
    MIDI.sendControlChange(58,opnarray[40]*8,sendchannel); //OP4 2nd Total Level
    
    MIDI.sendControlChange(90,opnarray[11]*8,sendchannel); //OP1 SSG-EG
    MIDI.sendControlChange(91,opnarray[21]*8,sendchannel); //OP2 SSG-EG   
    MIDI.sendControlChange(92,opnarray[31]*8,sendchannel); //OP3 SSG-EG  
    MIDI.sendControlChange(93,opnarray[41]*8,sendchannel); //OP4 SSG-EG

    MIDI.sendControlChange(75,0,sendchannel); //FM Level
    MIDI.sendControlChange(76,0,sendchannel); //AM Level
    MIDI.sendControlChange(75,127,sendchannel); //Stereo (centered)

    MIDI.sendControlChange(70,0,sendchannel); //OP1 Amplitude Modulation
    MIDI.sendControlChange(71,0,sendchannel); //OP2 Amplitude Modulation
    MIDI.sendControlChange(72,0,sendchannel); //OP3 Amplitude Modulation
    MIDI.sendControlChange(73,0,sendchannel); //OP4 Amplitude Modulation

    // Dump TFI settings into the global settings array
    
    for (int i = 0; i <= 41; i++) {
    fmsettings[sendchannel][i] = opnarray[i];
    }
    fmsettings[sendchannel][42] = 0; //FM Level
    fmsettings[sendchannel][43] = 0; //AM Level
    fmsettings[sendchannel][44] = 127; //Stereo (centered)
    fmsettings[sendchannel][45] = 0; //OP1 Amplitude Modulation
    fmsettings[sendchannel][46] = 0; //OP2 Amplitude Modulation
    fmsettings[sendchannel][47] = 0; //OP3 Amplitude Modulation
    fmsettings[sendchannel][48] = 0; //OP4 Amplitude Modulation
    fmsettings[sendchannel][49] = 0; //Patch is uneditted

    /*
    MIDI.sendNoteOn(60, 127, sendchannel);
    delay(200);           
    MIDI.sendNoteOff(60, 0, sendchannel);  
    */
    
}

void fmparamdisplay()
{

  switch(fmscreen) {

    // Algorthm, Feedback, Pan
    case 1:
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.write(byte(0));
      lcd.print(tfichannel);
      lcd.print(" ");
      lcd.print("01:Alg FB Pan");
      break;
    }

    // Total Level
    case 2:
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.write(byte(0));
      lcd.print(tfichannel);
      lcd.print(" ");
      lcd.print("02:TL");
      break;
    }

    // Multiple
    case 3:
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.write(byte(0));
      lcd.print(tfichannel);
      lcd.print(" ");
      lcd.print("03:Multiple");
      break;
    }

    // Rate Scaling
    case 4:
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.write(byte(0));
      lcd.print(tfichannel);
      lcd.print(" ");
      lcd.print("04:Rate Scale");
      break;
    }

    // Detune
    case 5:
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.write(byte(0));
      lcd.print(tfichannel);
      lcd.print(" ");
      lcd.print("05:Detune");
      break;
    }

    // Attack Rate
    case 6:
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.write(byte(0));
      lcd.print(tfichannel);
      lcd.print(" ");
      lcd.print("06:Attack");
      break;
    }

    // Decay Rate 1
    case 7:
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.write(byte(0));
      lcd.print(tfichannel);
      lcd.print(" ");
      lcd.print("07:Decay 1");
      break;
    }

    // Decay Rate 2
    case 8:
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.write(byte(0));
      lcd.print(tfichannel);
      lcd.print(" ");
      lcd.print("08:Decay 2");
      break;
    }

    // Total Level 2
    case 9:
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.write(byte(0));
      lcd.print(tfichannel);
      lcd.print(" ");
      lcd.print("09:TL 2");
      break;
    }

    // Release Rate
    case 10:
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.write(byte(0));
      lcd.print(tfichannel);
      lcd.print(" ");
      lcd.print("10:Release");
      break;
    }

    // SSG-EG
    case 11:
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.write(byte(0));
      lcd.print(tfichannel);
      lcd.print(" ");
      lcd.print("11:SSG-EG");
      break;
    }

    // Amp Mod
    case 12:
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.write(byte(0));
      lcd.print(tfichannel);
      lcd.print(" ");
      lcd.print("12:Amp Mod");
      break;
    }

    // AM and FM Level
    case 13:
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.write(byte(0));
      lcd.print(tfichannel);
      lcd.print(" ");
      lcd.print("13:AM FM Lvl");
      break;
    }   
     
  } // end switch  
  
}

void operatorparamdisplay()
{
      
      int difference;

      difference = prevpotvalue[0]-currentpotvalue;
      
      lcd.setCursor(1,1);
      currentpotvalue = analogRead(potPin1);
      cc = currentpotvalue>>3;
      if (cc<100) lcd.print("0");
      if (cc<10) lcd.print("0");
      lcd.print(cc);
      lcd.print(" ");
      
      currentpotvalue = analogRead(potPin2);
      cc = currentpotvalue>>3;
      if (cc<100) lcd.print("0");
      if (cc<10) lcd.print("0");
      lcd.print(cc);
      lcd.print(" ");
      
      currentpotvalue = analogRead(potPin3);
      cc = currentpotvalue>>3;
      if (cc<100) lcd.print("0");
      if (cc<10) lcd.print("0");
      lcd.print(cc);
      lcd.print(" ");
      
      currentpotvalue = analogRead(potPin4);
      cc = currentpotvalue>>3;
      if (cc<100) lcd.print("0");
      if (cc<10) lcd.print("0");
      lcd.print(cc);
      lcd.print(" ");
}

void MyHandleNoteOn(byte channel, byte pitch, byte velocity) {

velocity = velocity*(1.7);
if (velocity > 127) velocity = 127;

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
