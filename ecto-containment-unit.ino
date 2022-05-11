/*** Libraries ***/
#include <DFPlayerMini_Fast.h>
#include <AltSoftSerial.h>

/*** Variables ***/
// These may change after I get the buttons
int redLED = 2;
int redBtn = 3;
int yellowLED = 4;
int yellowBtn = 5;
int greenLED = 6;
int greenBtn = 7;

int lever = 9;
int cutOffLever = 10;

int topGreenLED = 11;
int topRedLED = 12;

int meterNeedle = 16;

/*** DfPlayer Setup ***/
int serial1 = 14; // RX
int serial2 = 15; // TX
AltSoftSerial mp3Serial(serial1, serial2); // RX, TX
DFPlayerMini_Fast myMP3;


void setup() {
  
  // Dfplayer serial  
  delay(1000);
  mp3Serial.begin(9600);
  // Monitor serial
  Serial.begin(115200);
  
  if (!myMP3.begin(mp3Serial)) {
    Serial.println(F("Unable to begin:"));
    Serial.println(F("Please recheck the connections!"));
  } else {
    Serial.println(F("DFPlayer Mini online."));
  }

}

void loop() {

  /*** System Status ***/
  static enum {
    OFF,
    RED_BTN_PREP,
    RED_BTN_PUSH,
    YELLOW_BTN_PREP,
    YELLOW_BTN_PUSH,
    GREEN_BTN_PREP,
    GREEN_BTN_PUSH,
    RESET
  } state = OFF;

  switch (state){ 
    case OFF:
      if() {
        
      }
      break;
    case RED_BTN_PREP:
      if() {
        
      }
      break;
    case RED_BTN_PUSH:
      if() {
        
      }
      break;
    case YELLLOW_BTN_PREP:
      if() {
        
      }
      break;
    case YELLOW_BTN_PUSH:
      if() {
        
      }
      break;
    case GREEN_BTN_PREP:
      if() {
        
      }
      break;
    case GREEN_BTN_PUSH:
      if() {
        
      }
      break;
    case RESET:
      if() {
        
      }
      break;
}
