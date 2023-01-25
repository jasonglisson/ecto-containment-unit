/*** Libraries ***/
#include <Metro.h>
#include <AltSoftSerial.h>    // Arduino build environment requires this
#include <wavTrigger.h>
#include <ezButton.h>

/*** Variables ***/
#define RED_LED_PIN 2
#define RED_BUTTON_PIN 3
#define YELLOW_LED_PIN 4
#define YELLOW_BUTTON_PIN 5
#define GREEN_LED_PIN 6

// Trap
ezButton TRAP_TRIGGER(7);
//#define TRAP_SMOKER 8

// Main Lever
#define MAIN_LEVER 10

// Cutoff Process
ezButton CUTOFF_LEVER(11);
#define CUTOFF_SMOKER 12
#define VENT_FAN 19
#define VENT_SERVO 21
#define VENT_LIGHT 22


// Top LEDS
#define TOP_GREEN_LED 13
#define TOP_RED_LED 14

// Needle Gauge Meter
#define NEEDLE_GAUGE 15

int gRateOffset = 0;         // WAV Trigger sample-rate offset

// Generic print flag
int printFlag;

/*** Wav Trigger Setup ***/
wavTrigger wTrig; // Our WAV Trigger object

// Variables will change:
int fanState = LOW;  // ledState used to set the LED

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;  // will store last time LED was updated

// constants won't change:
const long interval = 12000;  // interval at which to blink (milliseconds)

int count = 0; 

void setup() {

  // Serial monitor
  Serial.begin(9600);

  TRAP_TRIGGER.setDebounceTime(50);
  CUTOFF_LEVER.setDebounceTime(50);

  // LED and Button Prep
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(RED_BUTTON_PIN, INPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(YELLOW_BUTTON_PIN, INPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(MAIN_LEVER, INPUT);

//  pinMode(TRAP_TRIGGER, INPUT);

  digitalWrite(RED_LED_PIN, HIGH);
  digitalWrite(YELLOW_LED_PIN, HIGH);
  digitalWrite(GREEN_LED_PIN, HIGH);

  pinMode(TOP_GREEN_LED, OUTPUT);
  pinMode(TOP_RED_LED, OUTPUT);

  digitalWrite(TOP_GREEN_LED, HIGH);
  digitalWrite(TOP_RED_LED, HIGH);

  pinMode(VENT_FAN, OUTPUT);
  digitalWrite(VENT_FAN, LOW);

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
  delay(10000);
  wTrig.trackPlayPoly(2);               // Start Containment Unit constant hum
  wTrig.trackFade(2, 0, 2000, 0);       // Fade Track 2 up to 0dB over 2 secs
  wTrig.update();                       // Wait 2 secs
  wTrig.trackLoop(2, 1);                // Start hum loop
}

void loop() {
  TRAP_TRIGGER.loop();
  CUTOFF_LEVER.loop();

  if(CUTOFF_LEVER.isPressed())
    Serial.println("The button is pressed");

  if(CUTOFF_LEVER.isReleased())
    Serial.println("The button is released");

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
    TRAP_CLEAN,
    CUT_OFF_LEVER,
    ECU_OFF
  } state = READY;

  switch (state) {
    case READY:

      digitalWrite(RED_LED_PIN, LOW);
      digitalWrite(YELLOW_LED_PIN, LOW);

      digitalWrite(TOP_GREEN_LED, HIGH);
      digitalWrite(TOP_RED_LED, LOW);

      digitalWrite(GREEN_LED_PIN, HIGH);

      if (printFlag == 0x00) {
        Serial.print("Ecto Containment Unit Ready!");
        Serial.print("\r\n");
        Serial.print("\r\n");
        Serial.print("It's simple really. Loaded trap here.");
        Serial.print("\r\n");
        Serial.print("\r\n");
        Serial.print("Open, unlock the System.");
        Serial.print("\r\n");
        Serial.print("\r\n");
        //      Serial.print(digitalRead(TRAP_TRIGGER));
        //      Serial.print(digitalRead(RED_LED_PIN));
        //      Serial.print(digitalRead(YELLOW_LED_PIN));
        //      Serial.print(digitalRead(RED_BUTTON_PIN));
        printFlag = 0x01;
      }

      if (CUTOFF_LEVER.isReleased()) {
        shutDown();
        state = CUT_OFF_LEVER;
      }

      if (TRAP_TRIGGER.isReleased()) {
        Serial.print("Insert the trap.");
        Serial.print("\r\n");
        Serial.print("\r\n");
        digitalWrite(GREEN_LED_PIN, LOW);
        wTrig.trackLoad(4);
        wTrig.update(); 
        Serial.print("Release.");
        Serial.print("\r\n");
        Serial.print("\r\n");
        state = TRAP_INSERT;
      }

      break;
    case TRAP_INSERT:
      if (TRAP_TRIGGER.getState() == HIGH) {
        wTrig.trackPlayPoly(4);               // Start trap insert sound
        wTrig.update(); 
        Serial.print("Close...");
        Serial.print("\r\n");
        Serial.print("\r\n");
        digitalWrite(TOP_GREEN_LED, LOW);
        digitalWrite(TOP_RED_LED, HIGH);
        state = RED_BTN_PREP;
      }
      break;
    case RED_BTN_PREP:
      if (digitalRead(RED_BUTTON_PIN) == LOW) {
        if (printFlag == 0x01) {
          Serial.println("....Lock the System.");
          Serial.print("\r\n");
          Serial.print("\r\n");
          printFlag = 0x00;
        }
      }
      if (CUTOFF_LEVER.isReleased()) {
        shutDown();  
        state = CUT_OFF_LEVER;
      }
      if (digitalRead(RED_BUTTON_PIN) == HIGH) {
        wTrig.trackLoad(7);
        state = RED_BTN_PUSH;
      }
      break;
    case RED_BTN_PUSH:
      if (digitalRead(RED_BUTTON_PIN) == HIGH) {
        digitalWrite(RED_LED_PIN, HIGH);
        Serial.println("Set your entry grid.");
        wTrig.trackFade(2, -10, 1000, 0);
        wTrig.trackGain(7, 5);
        wTrig.trackPlayPoly(7);
        wTrig.trackLoad(8);
        wTrig.update(); 
        Serial.print("\r\n");
        Serial.print("\r\n");
        state = YELLOW_BTN_PREP;
      }
      break;
    case YELLOW_BTN_PREP:
      if (digitalRead(RED_BUTTON_PIN) == HIGH) {
        if (printFlag == 0x01) {
          printFlag = 0x00;
        }
        state = YELLOW_BTN_PUSH;
      }
      break;
    case YELLOW_BTN_PUSH:
      if (CUTOFF_LEVER.isReleased()) {
        shutDown();  
        state = CUT_OFF_LEVER;
      }
      if (digitalRead(YELLOW_BUTTON_PIN) == HIGH) {
        Serial.println("Neutralize your field");
        wTrig.trackFade(2, -40, 500, 0);
        wTrig.trackFade(7, -40, 500, 0);
        wTrig.trackStop(7);
        wTrig.update(); 
        wTrig.trackGain(8, 10); 
        wTrig.trackPlayPoly(8);
        wTrig.update(); 
        digitalWrite(RED_LED_PIN, LOW);
        digitalWrite(YELLOW_LED_PIN, HIGH);
        Serial.print("\r\n");
        Serial.print("\r\n");
        state = GREEN_BTN_PREP;
      }

      break;
    case GREEN_BTN_PREP:
      if (digitalRead(YELLOW_BUTTON_PIN) == HIGH) {
        state = GREEN_BTN_PUSH;
        wTrig.trackLoad(3);
      }
      break;
    case GREEN_BTN_PUSH:
      if (CUTOFF_LEVER.isReleased()) {
        shutDown();  
        state = CUT_OFF_LEVER;
      }
      if (digitalRead(MAIN_LEVER) == HIGH) {
        Serial.println("And....");
        Serial.print("\r\n");
        Serial.print("\r\n");
        wTrig.trackStop(8);
        wTrig.trackStop(7);
        wTrig.update();
        state = TRAP_CLEAN;
      }

      break;
    case TRAP_CLEAN:
      if (printFlag == 0x00) {
        wTrig.trackFade(2, 0, 1000, 0);
        wTrig.trackGain(3, 10); 
        wTrig.trackPlayPoly(3);
        wTrig.update();
        Serial.println("THE LIGHT IS GREEN, TRAP IS CLEAN!");
        Serial.print("\r\n");
        Serial.print("\r\n");
        Serial.print("Remove Clean Trap.");
        Serial.print("\r\n");
        Serial.print("\r\n");
        printFlag = 0x01;
      }

      if (digitalRead(MAIN_LEVER) == HIGH) {
        digitalWrite(YELLOW_LED_PIN, LOW);
        digitalWrite(RED_LED_PIN, LOW);
//        digitalWrite(TRAP_TRIGGER, LOW);
        digitalWrite(TOP_GREEN_LED, HIGH);
        digitalWrite(GREEN_LED_PIN, HIGH);
        delay(10000);
        state = READY;
      }

      break;
    case CUT_OFF_LEVER:
        unsigned long currentMillis = millis();
        int buttonstate = 0;
        if (buttonstate == 0) {

          digitalWrite(GREEN_LED_PIN, LOW);
          digitalWrite(RED_LED_PIN, HIGH);
          digitalWrite(TOP_RED_LED, HIGH);
           
          if (currentMillis - previousMillis >= interval) {
            digitalWrite(VENT_FAN, LOW);
            count++;
            previousMillis = currentMillis;

            if (count == 2) {
              digitalWrite(VENT_FAN, HIGH);
            } else if (count == 3) {
              digitalWrite(VENT_FAN, LOW);
              state = ECU_OFF;
            }
          }
          buttonstate = 1;
        } else {
          
          buttonstate = 0;
        
        }

      break;
    case ECU_OFF:
      Serial.print("ECU SHUTDOWN");
      if(CUTOFF_LEVER.getState() == LOW){
        Serial.print("COMING ONLINE");
      } else {
        Serial.print("ECU SHUTDOWN");
      }
      if (CUTOFF_LEVER.getState() == LOW) {
        wTrig.trackPlayPoly(2);               // Start Containment Unit constant hum
        wTrig.trackFade(2, 0, 7000, 0);       // Fade Track 2 up to 0dB over 2 secs
        wTrig.update();                       // Wait 2 secs
        wTrig.trackLoop(2, 1); 
        count = 0;
        state = READY;
      }      
      break;
  }
}

void shutDown() {
  wTrig.stopAllTracks();
  wTrig.update();
  wTrig.trackLoad(5);    
  wTrig.trackGain(5, 10); 
  wTrig.trackPlayPoly(5);
}
