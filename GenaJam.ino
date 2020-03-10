#include <MIDI.h>  // Midi Library
#include <SPI.h>
#include "SdFat.h"
#include "FreeStack.h"
#include <LiquidCrystal.h>

//-------------------------------------------
//LCD pin to Arduino
const int pin_RS = 8; 
const int pin_EN = 9; 
const int pin_d4 = 4; 
const int pin_d5 = 5; 
const int pin_d6 = 6; 
const int pin_d7 = 7; 
const int pin_BL = 10; 
LiquidCrystal lcd( pin_RS,  pin_EN,  pin_d4,  pin_d5,  pin_d6,  pin_d7);

int lcd_key     = 0;
int adc_key_in  = 0;
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

byte emojimono[] = {
  B11111,
  B10101,
  B10101,
  B00000,
  B11000,
  B10100,
  B10101,
  B00000
};

//debouncing

unsigned long buttonpushed = 0;             // when a button is pushed, mark what millis it was pushed
unsigned long debouncedelay = 200;   //the debounce time which user sets prior to run

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
const uint16_t nMax = 64;

// Position of file's directory entry.
uint16_t dirIndex[nMax];

//----------------------------------------------

// Create an instance of the library with default name, serial port and settings
MIDI_CREATE_DEFAULT_INSTANCE();

// start position of each channel file cursor
int tfifilenumber[6] = {0, 0, 0, 0, 0, 0};

// variables for pulling values from TFI files and putting them in an array
int tfiarray[42];

//set the initial midi channel
int tfichannel=1;

// switch between settings
int mode=1;
// 1) preset mono
// 2) settings mono

// global variable for storing FM settings for each channel
int fmsettings[6][50];

void setup()
{
    
    lcd.begin(16, 2);

    lcd.createChar(0, emojichannel);
    lcd.createChar(1, emojiprogram);
    lcd.createChar(2, emojimono);


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

  MIDI.begin(MIDI_CHANNEL_OFF);
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
  tfiselect(0, 6);
  tfiselect(0, 5);
  tfiselect(0, 4);
  tfiselect(0, 3);
  tfiselect(0, 2);
  tfiselect(0, 1);

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

  lcd_key = read_LCD_buttons();


switch(mode) {

//======================================================= MODE 1
case 1:
{
    switch (lcd_key)               // depending on which button was pushed, we perform an action
    {
    case btnRIGHT:
    {
    tfifilenumber[tfichannel-1] = tfifilenumber[tfichannel-1]+1;
    if(tfifilenumber[tfichannel-1]==(n)){ // if max files exceeded, loop back to start
    tfifilenumber[tfichannel-1]=0;
    }   
    tfiselect(tfifilenumber[tfichannel-1], tfichannel);
    break;
    }
    
    case btnLEFT:
    {
    tfifilenumber[tfichannel-1] = tfifilenumber[tfichannel-1]-1;
    if(tfifilenumber[tfichannel-1]==-1){ // if min files exceeded, loop back to end
    tfifilenumber[tfichannel-1]=n-1;
    }
    tfiselect(tfifilenumber[tfichannel-1], tfichannel);
    break;
    }
    
    case btnUP:
    {
    tfichannel=tfichannel+1;
    if(tfichannel==(7)){ // if max channels reached, loop around
    tfichannel=1;
    }
    channelselect(tfifilenumber[tfichannel-1], tfichannel);
    break;
    }
    
    case btnDOWN:
    {
    tfichannel=tfichannel-1;
    if(tfichannel==(0)){ // if max channels reached, loop around
    tfichannel=6;
    }
    channelselect(tfifilenumber[tfichannel-1], tfichannel);
    break;
    }
    
    case btnSELECT:
    {
    mode = mode + 1;
    if(mode > 2){ // loop mode
    mode = 1;
    }
    break;
    }
    
    } 
    
break;  
}


//======================================================= MODE 2
case 2:
{

break;  
}

  
}

  
} // void loop


int read_LCD_buttons() // function for reading the buttons
{
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


void tfiselect(int tfifilenumberselect, int tfichannelselect) //load a tfi , send the midi, update screen
{
    if (!tfifile.open(&dirFile, dirIndex[tfifilenumberselect], O_RDONLY)) {
      lcd.setCursor(0,0);
      lcd.print("ERROR:");
      lcd.setCursor(0,1);
      lcd.print("CANNOT READ TFI");
      sd.errorHalt(F("open"));
    }

    // open TFI file and read/send contents =================

    int filetoarray = 0; // read the file from the start
    // read from the file until there's nothing else in it:
    while (tfifile.available()) {
      tfiarray[filetoarray]=tfifile.read(), DEC; // read the byte data, convert it to DEC
      filetoarray++;
    } 

    
    tfisend(tfiarray, tfichannelselect);

     //get filename
    char tfifilename[MaxNumberOfChars + 1];
    tfifile.getName(tfifilename, MaxNumberOfChars);
    tfifilename[MaxNumberOfChars]=0; //ensure  termination

    //show filename on screen
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.write(byte(2));
    lcd.print(" ");
    lcd.write(byte(0));
    lcd.print(" ");
    lcd.print(tfichannelselect);
    lcd.print(" ");
    lcd.write(byte(1));
    lcd.print(" ");
    lcd.print(tfifilenumberselect+1);
    lcd.print("/");
    lcd.print(n);
    lcd.setCursor(0,1);
    lcd.print(tfifilename);

   
    // TFI file closed ================
    
  tfifile.close();
  
}


void channelselect(int tfifilenumberselect, int tfichannelselect) //select a new channel, display current tfi on screen
{
    if (!tfifile.open(&dirFile, dirIndex[tfifilenumberselect], O_RDONLY)) {
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
    lcd.write(byte(2));
    lcd.print(" ");
    lcd.write(byte(0));
    lcd.print(" ");
    lcd.print(tfichannelselect);
    lcd.print(" ");
    lcd.write(byte(1));
    lcd.print(" ");
    lcd.print(tfifilenumberselect+1);
    lcd.print("/");
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
