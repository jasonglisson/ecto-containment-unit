
/*** Libraries ***/
#include <AltSoftSerial.h>
#include <wavTrigger.h>
#include <Chrono.h>
#include <LightChrono.h>

// ==================================================================
// The following defines are used to control which serial class is
//  used. Uncomment only the one you wish to use. If all of them are
//  commented out, the library will use Hardware Serial
#define __WT_USE_ALTSOFTSERIAL__
// ==================================================================

/*** Variables ***/
#define RED_LED_PIN 2
#define RED_BUTTON_PIN 3
#define YELLOW_LED_PIN 4
#define YELLOW_BUTTON_PIN 5
#define GREEN_LED_PIN 6
#define GREEN_BUTTON_PIN 7

#define redLED 13
#define yellowLED 12
#define greenLED 11

int trapTrigger = 14;
int trapSmoker = 15;

int lever = 10;
int cutOffLever = 11;
int cutOffSmoker = 17;

int topGreenLED = 12;
int topRedLED = 13;

int meterNeedle = 16;

int gRateOffset = 0;         // WAV Trigger sample-rate offset

// Generic print flag
int printFlag;

byte redlastButtonState = LOW;
byte redLEDState = LOW;
unsigned long reddebounceDuration = 50; // millis
unsigned long redlastTimeButtonStateChanged = 0;

byte yellowlastButtonState = LOW;
byte yellowLEDState = LOW;
int yellowBTNReady;
unsigned long yellowdebounceDuration = 50; // millis
unsigned long yellowlastTimeButtonStateChanged = 0;

/*** Wav Trigger Setup ***/
int serial1 = 9; // RX
int serial2 = 8; // TX
AltSoftSerial mp3Serial(serial1, serial2); // RX, TX
wavTrigger wTrig; // Our WAV Trigger object

void setup() {

  // Serial monitor
  Serial.begin(9600);
  
  // LED and Button Prep
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(RED_BUTTON_PIN, INPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(YELLOW_BUTTON_PIN, INPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(GREEN_BUTTON_PIN, INPUT);

  pinMode(redLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  
  // If the Arduino is powering the WAV Trigger, we should wait for the WAV
  //  Trigger to finish reset before trying to send commands.
  delay(1000);

  // WAV Trigger startup at 57600
  wTrig.start();
  delay(10);
 
  // Send a stop-all command and reset the sample-rate offset, in case we have
  //  reset while the WAV Trigger was already playing.
  wTrig.stopAllTracks();
  wTrig.samplerateOffset(0);            // Reset our sample rate offset
  wTrig.masterGain(0);                  // Reset the master gain to 0dB

  wTrig.trackGain(1, -40);              // Preset Track 1 gain to -40dB
  wTrig.trackPlayPoly(1);               // Start Track 1
  wTrig.trackFade(1, 0, 3000, false);   // Fade Track 1 up to 0db over 3 secs
  wTrig.update();                       // Wait 2 secs
  wTrig.trackFade(1, -40, 3000, true);  // Fade Track 1 down to -40dB over 3 secs and stop
  
  wTrig.trackGain(2, -40);              // Preset Track 2 gain to -40dB
  wTrig.trackPlayPoly(1);               // Start Containment Unit constant hum
  wTrig.trackFade(2, 0, 2000, 0);       // Fade Track 2 up to 0dB over 2 secs
  wTrig.trackLoop(2, 1);                // Start hum loop
  
}

void loop() {

  /*** System Status ***/
  static enum {
    READY,
    TRAP_INSERT,    
    RED_BTN_PREP,
    RED_BTN_PUSH,
    YELLOW_BTN_PREP,
    YELLOW_BTN_PUSH,
    GREEN_BTN_PREP,
    GREEN_BTN_PUSH,
    CUT_OFF_LEVER
  } state = READY;

 switch (state){    
  case READY:
    if (printFlag == 0x00){
      Serial.print("ECU Ready State");
      printFlag = 0x01;
    }
    
    if(trapTrigger == HIGH) {
      state = TRAP_INSERT;
    }
  
    if (millis() - redlastTimeButtonStateChanged > reddebounceDuration) {
      byte redButtonState = digitalRead(RED_BUTTON_PIN);
      if (redButtonState != redlastButtonState) {
        redlastTimeButtonStateChanged = millis();
        redlastButtonState = redButtonState;
        if (redButtonState == LOW) {
          redLEDState = (redLEDState == HIGH) ? LOW: HIGH;
          digitalWrite(RED_LED_PIN, redLEDState);
          digitalWrite(redLED, redLEDState);
          Serial.println("Red Button Pressed");
        }
      }
    }
 
    if (redLEDState == HIGH) {
      Serial.println("Going to RED_BTN_PREP");
      state = RED_BTN_PREP;     
    }
    
    break;
//  case TRAP_INSERT:
//    wTrig.trackGain(4, -40);              // Preset Track 4 gain to -40dB
//    wTrig.trackPlayPoly(4);               // Start trap insert sound
//    if (redBtn == LOW) {
//      
//    } else {
//      state = READY;
//    }
//    break;     
  case RED_BTN_PREP:
    if (redLEDState == HIGH) {
      if (printFlag == 0x01){
        Serial.println("Going to RED_BTN_PUSH");
        printFlag = 0x00;
      }
      state = RED_BTN_PUSH;
    }
    break;
  case RED_BTN_PUSH:
    if (redLEDState == HIGH) {
      if (printFlag == 0x00){
        Serial.println("Going to YELLOW_BTN_PREP");
        printFlag = 0x01;
      }
      state = YELLOW_BTN_PREP;
    }
    break;
  case YELLOW_BTN_PREP:
    if (redLEDState == HIGH) {
      if (printFlag == 0x01){
        Serial.println("Going to YELLOW_BTN_PUSH");
        printFlag = 0x00;
      }
      state = YELLOW_BTN_PUSH;
    }
    break;
  case YELLOW_BTN_PUSH:
    if (redLEDState == HIGH) {
      if (printFlag == 0x00){
        Serial.println("Turn on yellow light!");
        printFlag = 0x01;
      }
      if (millis() - yellowlastTimeButtonStateChanged > yellowdebounceDuration) {
        byte yellowButtonState = digitalRead(YELLOW_BUTTON_PIN);
        if (yellowButtonState != yellowlastButtonState) {
          yellowlastTimeButtonStateChanged = millis();
          yellowlastButtonState = yellowButtonState;
          if (yellowButtonState == LOW) {
            yellowLEDState = (yellowLEDState == HIGH) ? LOW: HIGH;
            digitalWrite(YELLOW_LED_PIN, yellowLEDState);
            Serial.println("YELLOW Button Pressed");
            Serial.println(yellowLEDState);
          }
        }
      }  
    }

    if (yellowLEDState == HIGH) {
      Serial.println("Going to GREEN_BTN_PREP");
      state = GREEN_BTN_PREP;     
    }
    
    break;
  case GREEN_BTN_PREP:
    if (yellowLEDState == HIGH) {
      if (printFlag == 0x01){
        Serial.println("Ready for lever to be pulled");
        printFlag = 0x00;
      }
    }
    break;
//  case GREEN_BTN_PUSH:
//    if (RED_BUTTON_PIN == LOW) {
//
//    } else if (cutOffLever == LOW) {
//      state = CUT_OFF_LEVER;
//    } else {
//      state = READY;
//    }
//    break;   
//  case CUT_OFF_LEVER:
//    if (cutOffLever == HIGH) {
//
//    } else if (cutOffLever == LOW) {
//      state = CUT_OFF_LEVER;
//    } else {
//      state = READY;
//    }
//    break;
 }
}
