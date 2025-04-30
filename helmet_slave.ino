//Chip: Arduino Nano, SLAVE
//Components: 2 LED Matrixs(NeoPixel, 8), nRF24L01 module

#include <SPI.h>      //to communicate other chipset
#include "RF24.h"     //to use nRF24L01 module
#include <Adafruit_NeoPixel.h>    //to use NEO_PIXEL LED RING

//pin macro part
#define CE 7          //for nRF24L01's pins [the others(MOSI, MISO, SCK, IRQ)]
#define CSN 8         //MOSI: No.11   MISO: No.12   SCK: No.13  IRQ: NONE
                      //the others should be connect Arduino Nano

#define LED_LEFT 2    
#define LED_RIGHT 3   //LED pins
//pin macro part

#define LED_NUM 8       //Number of LEDs
#define LED_BRIGHT 64   //Brightness, 0-255

#define BLINK 150     //Blinking Period, 150[ms]
#define BLINK_COUNT 1 //Blinking Count. But, it can be ignored.

char con;                    //Control command
byte address[7] = "PMJPJW";  //Communication Address

Adafruit_NeoPixel leftLed = Adafruit_NeoPixel(LED_NUM, LED_LEFT, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel rightLed = Adafruit_NeoPixel(LED_NUM, LED_RIGHT, NEO_GRB + NEO_KHZ800);
//Create NEO_PIXEL ring's object. for Left LED ring and Right LED ring
// NEO_KHZ800  <- it means code of product

RF24 helmet(CE, CSN);   //Create RF24's Object

bool blinking = false;    //to determine LED are blinking or not
char blinkMode = 'X';      //init value : 'X'
unsigned long previousMillis = 0;   //To use unblocking way
int blinkState = 0;   // 0: OFF, 1: ON
int blinkCount = 0;   //Blinking count. But, it can be ignored.

void initComm() {       //init nRF24L01 module
  helmet.begin();
  helmet.openReadingPipe(1, address);   //using address "PMJPJW" same address with MASTER
  helmet.startListening();    //SLAVE MODE
}

void initLeds() {       //init LED
  leftLed.setBrightness(LED_BRIGHT);
  rightLed.setBrightness(LED_BRIGHT);
  leftLed.begin();
  rightLed.begin();
  leftLed.show();  
  rightLed.show();
}

void setAllLeds(uint32_t colorL, uint32_t colorR) {   //to use GRB code   .Color(green, red, yellow)
  for (int i = 0; i < LED_NUM; i++) {   //[0] to [7] LED on
    leftLed.setPixelColor(i, colorL);
    rightLed.setPixelColor(i, colorR);
  }
  leftLed.show();
  rightLed.show();      //update color
}

void updateBlink() {      //to update current LED matrix pattern
  unsigned long currentMillis = millis();         //unblocking, time syncronized
  if (blinking && currentMillis - previousMillis >= BLINK) {    //ex) if LED blinking and (300ms - 0ms >= BLINK[150ms])
    previousMillis = currentMillis;             //save previous value to current value(will be 300ms)
    if (blinkState == 0) { // Turn ON
      switch (blinkMode) {    //blinkMode = con [L, R, E, X]
        case 'L': setAllLeds(leftLed.Color(255, 255, 0), rightLed.Color(0, 0, 0)); break; //Left on
        case 'R': setAllLeds(leftLed.Color(0, 0, 0), rightLed.Color(255, 255, 0)); break; //Right on
        case 'E': setAllLeds(leftLed.Color(255, 255, 0), rightLed.Color(255, 255, 0)); break; //All on
      }
      blinkState = 1;   //toggle state
    } else {
      setAllLeds(0, 0);   //turn LED off
      blinkState = 0;     //toggle state
      blinkCount++;       //blink count increase, let blinkCount 10
      if (blinkCount >= BLINK_COUNT) {    //ex) 10 >= 1
        blinking = false;      //Don't blink
        blinkMode = 'X';    //init con value
      }
    }
  }
}

void setup() {
  initComm();
  initLeds();
  //Serial.begin(9600);   //for debugging
}

void loop() {
  if (helmet.available()) {         //Is possible to communicate?
    helmet.read(&con, sizeof(con));   //then read out values
    //Serial.print("Received character: "); 
    //Serial.println(con);        //for debugging

    if (con == 'L' || con == 'R' || con == 'E') { 
      blinking = true;
      blinkMode = con;
      blinkCount = 0;
      blinkState = 0;
      previousMillis = millis();    //update current time
    }
  }
  delay(200);   //To ensure wireless communication stability or allow time for transmission completion
  updateBlink();
}
