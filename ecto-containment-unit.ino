/*** Libraries ***/
#include <Metro.h>
#include <AltSoftSerial.h>    // Arduino build environment requires this
#include <wavTrigger.h>
#include <ezButton.h>
#include <ServoTimer2.h>

// Audio connections are:
// D9 to WavTrigger RX
// D8 to WavTrigger TX

ServoTimer2 servo;
int servoPin = 16; // A2

/*** Variables ***/
// Buttons
#define RED_LED_PIN 2
#define RED_BUTTON_PIN 3
#define YELLOW_LED_PIN 4
#define YELLOW_BUTTON_PIN 5
#define GREEN_LED_PIN 6

// Trap
ezButton TRAP_TRIGGER(7);
#define TRAP_PUMP 12
#define TRAP_SMOKER 11

// Main Lever
#define MAIN_LEVER 15

// Cutoff Process
ezButton CUTOFF_LEVER(11);
#define VENT_SMOKER 17 // A3
#define CUTOFF_PUMP 18 // A4
#define VENT_FAN_LIGHT 19 // A5

// Top LEDS
#define TOP_GREEN_LED 14 // A0
#define TOP_RED_LED 13

// Needle Gauge Meter
#define NEEDLE_GAUGE 10
unsigned long NEEDLEpreviousMillis = 0;
unsigned long NEEDLEcurrentMillis;

// Generic trap flag
int trapFlag = 0x00;

/*** Wav Trigger Setup ***/
wavTrigger wTrig; // Our WAV Trigger object

// Variables will change:
int fanState = LOW;
int ledState = LOW;
int needleState = LOW;
int topRedLEDState = LOW;

// will store last time LED was updated
unsigned long previousMillis = 0;
unsigned long REDTOPpreviousMillis = 0;

// constants won't change:
const long interval = 10500;
unsigned long currentMillis;
unsigned long REDTOPcurrentMillis;
int count = 0;
bool shutdownstate = 0; 

void setup() {

  // Serial monitor
  Serial.begin(9600);

  servo.attach(servoPin);
  servo.write(750);

  TRAP_TRIGGER.setDebounceTime(50);
  CUTOFF_LEVER.setDebounceTime(50);
  
  // LED and Button Prep
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(RED_BUTTON_PIN, INPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(YELLOW_BUTTON_PIN, INPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(MAIN_LEVER, INPUT);
  pinMode(NEEDLE_GAUGE, OUTPUT);
  
  digitalWrite(RED_LED_PIN, HIGH);
  digitalWrite(YELLOW_LED_PIN, HIGH);
  digitalWrite(GREEN_LED_PIN, HIGH);

  pinMode(TOP_GREEN_LED, OUTPUT);
  pinMode(TOP_RED_LED, OUTPUT);

  digitalWrite(TOP_GREEN_LED, HIGH);
  digitalWrite(TOP_RED_LED, HIGH);

  pinMode(VENT_FAN_LIGHT, OUTPUT);
  digitalWrite(VENT_FAN_LIGHT, LOW);
  pinMode(CUTOFF_PUMP, OUTPUT);
  digitalWrite(CUTOFF_PUMP, HIGH);

  pinMode(TRAP_PUMP, OUTPUT);
  digitalWrite(TRAP_PUMP, HIGH);

  // WAV Trigger startup at 57600
  wTrig.start();
  delay(10);
  
  // Send a stop-all command and reset the sample-rate offset, in case we have
  //  reset while the WAV Trigger was already playing.
  wTrig.stopAllTracks();
  wTrig.samplerateOffset(0);            // Reset our sample rate offset
  wTrig.masterGain(0);                  // Reset the master gain to 0dB
  
  wTrig.trackLoad(1);
  wTrig.trackPlayPoly(1);               // Start Track 1
  wTrig.trackLoad(2);
  wTrig.update();                       // Wait 2 secs
  delay(8000);
  wTrig.trackPlayPoly(2);               // Start Containment Unit constant hum
  wTrig.trackFade(1, -40, 2000, 0);       // Fade Track 1 down to -40dB over 2 secs
  wTrig.trackFade(2, 0, 2000, 0);       // Fade Track 2 up to 0dB over 2 secs
  wTrig.update();                       // Wait 2 secs
  wTrig.trackLoop(2, 1);                // Start hum loop
  Serial.println("ECU Ready");
}

void loop() {
  
  TRAP_TRIGGER.loop();
  CUTOFF_LEVER.loop();
  
  /*** System Status ***/
  static enum {
    READY,
    TRAP_INSERT,
    RED_BTN_PREP,
    RED_BTN_PUSH,
    YELLOW_BTN_PREP,
    YELLOW_BTN_PUSH,
    LEVER_PREP,
    LEVER_PUSH,
    TRAP_CLEAN,
    CUT_OFF_LEVER,
    ECU_OFF,
    STAND_BY
  } state = READY;

  if(CUTOFF_LEVER.isPressed()) {
    // Lever is pulled back up
  }

  if(CUTOFF_LEVER.isReleased()) {
    // Lever is pulled down
    shutDown();
    state = CUT_OFF_LEVER;
    Serial.println("SHUTDOWN START");
  }

  switch (state) {
    case READY:
      if (digitalRead(servoPin) == LOW) {
        servo.write(750);
      }
      wTrig.masterGain(0);
      
      digitalWrite(RED_LED_PIN, LOW);
      digitalWrite(YELLOW_LED_PIN, LOW);

      digitalWrite(NEEDLE_GAUGE, HIGH);

      digitalWrite(TOP_GREEN_LED, HIGH);
      digitalWrite(TOP_RED_LED, LOW);

      digitalWrite(GREEN_LED_PIN, HIGH);

      digitalWrite(CUTOFF_PUMP, LOW);
      digitalWrite(TRAP_PUMP, LOW);
      Serial.println(TRAP_PUMP);
      if (TRAP_TRIGGER.isReleased()) {
        digitalWrite(GREEN_LED_PIN, LOW);
        wTrig.trackLoad(4);
        wTrig.update(); 
        state = TRAP_INSERT;
      }

      break;
    // This the trap insert sound. Once the trap is inserted,
    // the system is ready to go.
    case TRAP_INSERT:
      if (TRAP_TRIGGER.getState() == HIGH) {

        wTrig.trackPlayPoly(4); // Start trap insert sound
        wTrig.update(); 
        digitalWrite(TOP_GREEN_LED, LOW);
        digitalWrite(TOP_RED_LED, HIGH);
        digitalWrite(TRAP_PUMP, HIGH);
        delay(3000);
        state = RED_BTN_PREP;
      }
      break;
    // Press the red button to start the ECU process  
    case RED_BTN_PREP:
      digitalWrite(TRAP_PUMP, LOW);
      if (digitalRead(RED_BUTTON_PIN) == LOW) {

      }
      if (CUTOFF_LEVER.isReleased()) { 
        state = CUT_OFF_LEVER;
      }
      if (digitalRead(RED_BUTTON_PIN) == HIGH) {
        wTrig.trackLoad(7);
        state = RED_BTN_PUSH;
      }
      break;
    // Once the red button is pressed, prep the yellow button 
    case RED_BTN_PUSH:
      if (digitalRead(RED_BUTTON_PIN) == HIGH) {
        digitalWrite(RED_LED_PIN, HIGH);
        wTrig.trackFade(2, -10, 1000, 0);
        wTrig.trackGain(7, 5);
        wTrig.trackPlayPoly(7);
        wTrig.trackLoad(8);
        wTrig.update(); 
        state = YELLOW_BTN_PREP;
      }
      break;
    // Continue on to press the yellow button
    case YELLOW_BTN_PREP:
      if (digitalRead(RED_BUTTON_PIN) == HIGH) {
        state = YELLOW_BTN_PUSH;
      }
      break;
    // Once the yellow button is pressed, 
    case YELLOW_BTN_PUSH:
      if (digitalRead(YELLOW_BUTTON_PIN) == HIGH) {
        wTrig.trackFade(2, -40, 500, 0);
        wTrig.trackFade(7, -40, 500, 0);
        wTrig.trackStop(7);
        wTrig.update(); 
        wTrig.trackGain(8, 10); 
        wTrig.trackPlayPoly(8);
        wTrig.update(); 
        digitalWrite(RED_LED_PIN, LOW);
        digitalWrite(YELLOW_LED_PIN, HIGH);
        state = LEVER_PREP;
      }
      break;
    // The lever is ready to be pulled down
    case LEVER_PREP:
      if (digitalRead(YELLOW_BUTTON_PIN) == HIGH) {
        state = LEVER_PUSH;
        wTrig.trackLoad(3);
      }
      break;
    // The lever has been pulled down
    case LEVER_PUSH:
      if (CUTOFF_LEVER.isReleased()) {
        state = CUT_OFF_LEVER;
      }
      if (digitalRead(MAIN_LEVER) == HIGH) {
        wTrig.trackStop(8);
        wTrig.trackStop(7);
        wTrig.update();
        state = TRAP_CLEAN;
      }

      break;
    // The trap is clean and the system has reset
    case TRAP_CLEAN:
      if (trapFlag == 0x00) {
        wTrig.trackFade(2, 0, 1000, 0);
        wTrig.trackGain(3, 10); 
        wTrig.trackPlayPoly(3);
        wTrig.update();
        trapFlag = 0x01;
      }

      // The Main lever must be pulled back up to reset the system
      if (digitalRead(MAIN_LEVER) == LOW) {
        digitalWrite(YELLOW_LED_PIN, LOW);
        digitalWrite(RED_LED_PIN, LOW);
//        digitalWrite(TRAP_TRIGGER, LOW);
        digitalWrite(TOP_GREEN_LED, HIGH);
        digitalWrite(GREEN_LED_PIN, HIGH);
        delay(10000);
        state = READY;
      }

      break;
    // This is the start of the shutdown process. You can
    // get to this step from anywhere in the process.
    case CUT_OFF_LEVER:
        if (shutdownstate == 0) {
          
          digitalWrite(GREEN_LED_PIN, LOW);
          digitalWrite(RED_LED_PIN, HIGH);
          digitalWrite(TOP_GREEN_LED, LOW);


          unsigned long NEEDLEcurrentMillis = millis();
      
          if (NEEDLEcurrentMillis - NEEDLEpreviousMillis >= 100) {
          // save the last time you blinked the LED
          NEEDLEpreviousMillis = NEEDLEcurrentMillis;
      
            // if the LED is off turn it on and vice-versa:
            if (needleState == LOW) {
              needleState = HIGH;
            } else {
              needleState = LOW;
            }
            digitalWrite(NEEDLE_GAUGE, needleState);
          }
      

          REDTOPcurrentMillis = millis();
          
          if (REDTOPcurrentMillis - REDTOPpreviousMillis >= 515) {
            REDTOPpreviousMillis = REDTOPcurrentMillis;
            if (topRedLEDState == LOW) {
              topRedLEDState = HIGH;
            } else {
              topRedLEDState = LOW;
            }
            digitalWrite(TOP_RED_LED, topRedLEDState); 
          }
          
          if (currentMillis - previousMillis >= interval) {
            count++;       
            previousMillis = currentMillis;
          }
        }
      switch (count) {
        case 0:
          // Run the vent PUMP
          digitalWrite(CUTOFF_PUMP, HIGH);
          currentMillis = millis();
          break;            
        case 1:
          currentMillis = millis();
          break;
        case 2:
          // Turn the PUMP off
          // Open the Vent
          if (digitalRead(servoPin) == LOW) {
            servo.write(1500);
          }
          // Turn the fan on
          Serial.println("SHUT DOWN 2");
          digitalWrite(VENT_FAN_LIGHT, HIGH);
          currentMillis = millis();
          break;
        case 3:
          Serial.println("SHUT DOWN 3");
          currentMillis = millis();
          
          // Turn the fan off but leave vent open
          digitalWrite(VENT_FAN_LIGHT, LOW);
          digitalWrite(CUTOFF_PUMP, LOW);
          wTrig.trackFade(11, -40, 1000, 0);
          break;
        case 4:
          Serial.println("SHUT DOWN 4");
          currentMillis = millis();
          shutdownstate = 1;
          break;
     }      
      if(shutdownstate == 1) {
        state = ECU_OFF;
        wTrig.trackLoad(1);          
        wTrig.trackLoad(2);               // Start Containment Unit constant hum
        wTrig.trackLoad(9);
        wTrig.trackLoad(10);
        wTrig.trackGain(1, 0);
        wTrig.trackGain(2, -40);
        wTrig.trackGain(9, -30);
        wTrig.trackGain(10, -30);
      }
      break;
    case ECU_OFF:
      Serial.println("ECU IS OFF");
      
      digitalWrite(TOP_RED_LED, LOW);
      digitalWrite(NEEDLE_GAUGE, LOW);

      currentMillis = millis();
             
      // Flash red button and play click
      if (currentMillis - previousMillis >= 600) {
        previousMillis = currentMillis;
        if (ledState == LOW) {
          ledState = HIGH;
          wTrig.trackPlayPoly(9);
        } else {
          ledState = LOW;
          wTrig.trackPlayPoly(10);
        }
        digitalWrite(RED_LED_PIN, ledState);
      }
      
      // If maincut off lever is back up, system will reset.
      if(CUTOFF_LEVER.isPressed()) {
        wTrig.trackPlayPoly(1);
        wTrig.update();
        wTrig.trackPlayPoly(2);
        wTrig.trackFade(2, 0, 5000, 0);       // Fade Track 2 up to 0dB over 7 secs
        wTrig.update();                       // Wait 2 secs
        wTrig.trackLoop(2, 1);
        shutdownstate = 0;
        count = 0;
        trapFlag = 0x00;
        state = READY;
      }
      break;
  }
}

void shutDown() {
  // Shutdown sequence sounds
  wTrig.stopAllTracks();
  wTrig.update();
  wTrig.trackLoad(5); 
  wTrig.trackLoad(11);
  wTrig.trackGain(5, 10);
  wTrig.trackGain(11, 10); 
  wTrig.trackPlayPoly(5);
  wTrig.trackPlayPoly(11);
}
