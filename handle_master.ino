//Chip: Arduino Nano, MASTER
//Components: 3-state 6pins switch, 2pins push switch, 2 6-pi LEDs, 2 Resistors [330Ohm], nRF24L01 Module

#include <SPI.h>      //to communicate other chipset
#include "RF24.h"     //to use nRF24L01 module

//pin macro part
#define CE 7          //for nRF24L01's pins [the others(MOSI, MISO, SCK, IRQ)]
#define CSN 8         //MOSI: No.11   MISO: No.12   SCK: No.13  IRQ: NONE
                      //the others should be connect Arduino Nano
#define SW_LEFT 2     
#define SW_RIGHT 3    //SW_LEFT & SW_RIGHT -> 3-state 6pins toggle Switch
#define SW_PUSH 4     //-> 2pins push Switch

#define LED_LEFT 5    
#define LED_RIGHT 6   //6-Pi  Yellow LEDs, connected with Resistor 330[Ohm]
//pin macro part

#define BLINK 200       //Blinking Period, 200[ms]
#define BLINK_COUNT 8   //Blinking Count. But, it can be ignored.

char con;                       //Control command
byte address[7] = "PMJPJW";     //Communication Address

bool pushState = false;         //Variant to use push switch to toggle switch  
bool toggle = false;            //button toggle variant
unsigned long previousMillis = 0;   //To use  unblocking way
int blinkState = 0;  // 0: OFF, 1: ON
int blinkCount = 0;   //Blinking count. But, it can be ignored.

RF24 handle(CE, CSN);   //Create RF24's Object

void initComm() {       //init nRF24L01 module
  handle.begin();
  handle.openWritingPipe(address);    //using address "PMJPJW", same address with SLAVE
  handle.stopListening();   //MASTER mode
}

void initPins() {     //init I/O pins
  pinMode(LED_LEFT, OUTPUT);
  pinMode(LED_RIGHT, OUTPUT);
  pinMode(SW_PUSH, INPUT_PULLUP);
  pinMode(SW_LEFT, INPUT_PULLUP);
  pinMode(SW_RIGHT, INPUT_PULLUP);
}

bool toggleState() {      //toggle handler
  static bool lastPush = HIGH;    //init value: Not pushed
  bool current = digitalRead(SW_PUSH);  //current push value
  if (lastPush == HIGH && current == LOW) { //if button pushed,
    pushState = !pushState;           //toggle pushState
    delay(50);                //debouncing code
    lastPush = current;       //save current value to lastPush(to continue LED pattern)
    return true;              //return ture
  }
  lastPush = current;       //else false
  return false;
}

void updateBlink() {        //to update current LED pattern
  unsigned long currentMillis = millis();       //unblocking, time syncronized
  if (currentMillis - previousMillis >= BLINK) {  //ex) 300ms - 0ms >= BLINK[200ms]
    previousMillis = currentMillis;               //save previous value to current value(will be 300ms)
    blinkState = !blinkState;                   //toggle blinkState
    blinkCount++;                               //blink count increase
                                                //let blinkCount 10
    if (blinkCount >= BLINK_COUNT) {          //ex) 10 >= 8
      blinkState = 0;                         
      blinkCount = 0;                         //init All to 0 value
    }
  }                                         //LEDs will be off
}

void blinkLeft() {        //led pattern for Left
  digitalWrite(LED_LEFT, blinkState ? HIGH : LOW);    //if LED blinking, it will be HIGH state
  digitalWrite(LED_RIGHT, LOW);                       //by fucntion 'updateBlink()'
}                                                     //rest one will be off

void blinkRight() {     //led pattern for Right
  digitalWrite(LED_LEFT, LOW);
  digitalWrite(LED_RIGHT, blinkState ? HIGH : LOW);
}

void blinkAll() {     //led pattern for All LEDs
  digitalWrite(LED_LEFT, blinkState ? HIGH : LOW);
  digitalWrite(LED_RIGHT, blinkState ? HIGH : LOW);
}

void ledOff(){        //All the LEDs off
  digitalWrite(LED_LEFT, LOW);
  digitalWrite(LED_RIGHT, LOW);  
}

void setup() {
  initComm();
  initPins();
  //Serial.begin(9600);   //for debugging
}

void loop() {
  con = 'X';  // init value : 'X', it means NOTHING to do

  if (digitalRead(SW_LEFT) == LOW) {  // SW_LEFT ON
    con = 'L';      //L for Left
    blinkLeft();
  }
  else if (digitalRead(SW_RIGHT) == LOW) {  // SW_RIGHT ON
    con = 'R';    //R for Right
    blinkRight();
  }

  if (toggleState()) {      // Is SW_PUSH Pushed?
    toggle = !toggle;       //then toggle.
  }

  if (toggle) {         
    con = 'E';    //E for Emergency
    blinkAll();
  }

  handle.write(&con, sizeof(con));    //Send data to SLAVE device
  delay(400)                      //To ensure wireless communication stability or wait for receiver processing time
  //Serial.print("Sent: ");
  //Serial.println(con);          //for debugging
  ledOff();
  updateBlink();
}
