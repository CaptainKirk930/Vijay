#include <ClickEncoder.h>
#include <TimerOne.h>
#include <LiquidCrystal_I2C.h>
#include <FastLED.h>
#include <SoftwareSerial.h>
#define PIN 25

const byte numLEDS = 83;

ClickEncoder *encoder;
int16_t last, value;

CRGB strip[numLEDS];

int encoderValue;
int encoderPosition;
uint8_t gHue = 0;

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

void timerIsr() {
  encoder->service();
}


void setup() {
 Serial.begin(115200); // start the serial monitor link

  lcd.begin(20, 4);        // initialize the lcd for 20 chars 4 lines

  FastLED.addLeds<NEOPIXEL, PIN>(strip, numLEDS);
  FastLED.setBrightness(255);
  encoder = new ClickEncoder(A1, A0, A2);
  encoder->setAccelerationEnabled(false);

  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr); 
  last = -1;

}

void loop(){
  
  value += encoder->getValue();
  
  if (value*2 != last) {
    encoderValue = value*2;
    encoderPosition = encoderValue%20;
      
    last = encoderValue;
    
    Serial.println("Value: ");
    Serial.println(encoderValue);
    
    Serial.println("Position:");
    
    if(encoderPosition <0)
      encoderPosition = -encoderPosition;

    if(encoderValue > 81)
    {
      value = 82/2;
      encoderValue = 82;
    }

    if(encoderValue < 0)
    {
      value = 0;
      encoderValue = 0;
    }

    Serial.println();
    for(int i = 0; i < encoderValue; i++)
    {
      //strip[i] = CRGB(255, 0, 0);
      
     
    }
    for(int i = 82; i > encoderValue-1; i--)
    {
      strip[i] = CRGB(0, 0, 0); 
    }
    

    
    
  }
  rainbow();
  FastLED.show();
    
  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    #define VERBOSECASE(label) case label:Serial.println(#label); break;
    switch (b) {
      VERBOSECASE(ClickEncoder::Pressed);
      VERBOSECASE(ClickEncoder::Held)
      VERBOSECASE(ClickEncoder::Released)
      VERBOSECASE(ClickEncoder::Clicked)
      case ClickEncoder::DoubleClicked:
          encoder->setAccelerationEnabled(!encoder->getAccelerationEnabled());
        break;
    }
  } 
  
}


void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( strip, encoderValue, gHue, 7);
}



