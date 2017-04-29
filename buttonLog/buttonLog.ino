#include <Wire.h>  // Comes with Arduino IDE
#include <LiquidCrystal_I2C.h>
#include <FastLED.h>
#include <SoftwareSerial.h>

#define PIN 25

const byte numLEDS = 109;

CRGB strip[numLEDS];

#define VS1053_RX  2 // This is the pin that connects to the RX pin on VS1053

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

#define Button1 18
#define Button2 19

unsigned long yesVote = 0;
unsigned long noVote = 0;
int ID = 0;

int noteCount = 2;
int noteTotal = 0;

bool button1Flag = 0;
bool button2Flag = 0;

byte buttonCount1 = 0;
byte buttonCount2 = 0;
byte randInt = 0;
byte oldRandInt = 0;

bool rainbowOn = 0;
bool button1Interrupt = 0;
bool button2Interrupt = 0;

int counter = 1;

uint8_t gHue = 0;

// See http://www.vlsi.fi/fileadmin/datasheets/vs1053.pdf Pg 31
#define VS1053_BANK_DEFAULT 0x00
#define VS1053_BANK_DRUMS1 0x78
#define VS1053_BANK_DRUMS2 0x7F
#define VS1053_BANK_MELODY 0x79

// See http://www.vlsi.fi/fileadmin/datasheets/vs1053.pdf Pg 32 for more!
#define VS1053_GM1_OCARINA 99

#define MIDI_NOTE_ON  0x90
#define MIDI_NOTE_OFF 0x80
#define MIDI_CHAN_MSG 0xB0
#define MIDI_CHAN_BANK 0x00
#define MIDI_CHAN_VOLUME 0x07
#define MIDI_CHAN_PROGRAM 0xC0

SoftwareSerial VS1053_MIDI(0, 2); // TX only, do not use the 'rx' side

void setup()
{
  Serial.begin(9600);
  lcd.begin(20, 4);        // initialize the lcd for 20 chars 4 lines 

  FastLED.addLeds<NEOPIXEL, PIN>(strip, numLEDS);

  VS1053_MIDI.begin(31250); // MIDI uses a 'strange baud rate'

  pinMode(Button1, INPUT_PULLUP);
  pinMode(Button2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(Button1), button1_ISR, LOW);
  attachInterrupt(digitalPinToInterrupt(Button2), button2_ISR, LOW);
  FastLED.setBrightness(255);
  randomSeed(analogRead(A1));
  ID = random(0, 4000);
  Serial.println(ID);
  //checkStatus();
  //setUpCom();
  //stoper=false;

  midiSetChannelBank(0, VS1053_BANK_MELODY);
  midiSetInstrument(0, VS1053_GM1_OCARINA);
  midiSetChannelVolume(0, 127);
}

void loop() {
  lcd.setCursor(0, 0); //Start at character 4 on line 0
  lcd.print("    Please Push");
  lcd.setCursor(0, 1); //Start at character 4 on line 0
  lcd.print("     a Button!");
  counter = counter + 1;
  counter = counter % 2;
  colorPick();
  if (button1Flag == 0 && button2Flag == 0)
  {
    if (counter % 2 == 0)
    {
      inwardTheaterChase(50); // Random
    }
    if (counter % 2 == 1)
    {
      outwardTheaterChase(50); // Random
    }
    thankYou();
    checkButtonCount();
  }
  button1Flag = 0;
  button2Flag = 0;

}

void button1_ISR()
{
  if(button2Interrupt == 0)
  {
      button1Flag = 1;
      button1Interrupt = 1;
  }
}

  void button2_ISR()
  {
    if(button1Interrupt == 0)
    {
        button2Flag = 1;
        button2Interrupt = 1;
    }
  }


void thankYou()
{
  if (button1Flag == 1 || button2Flag == 1)
  {
    lcd.clear();
    lcd.setCursor(0, 1); //Start at character 4 on line 0
    lcd.print("     Thank You!");
    if (button1Flag == 1)
    {
      for (int i = 0; i < 200; i++)
      {
        bpm();
        EVERY_N_MILLISECONDS( 20 ) {
          gHue++;  // slowly cycle the "base color" through the rainbow
        }
        FastLED.show();
        if(i % 10 == 0)
        { 
          midiNoteOn(0, 60 + noteTotal, 127);
         
          midiNoteOff(0, (60 - noteCount ) + noteTotal, 127);
          noteTotal += noteCount;

          if(noteTotal == noteCount * 20)
          {
            noteTotal = 0;
            midiNoteOff(0, 60 + noteTotal - noteCount, 127);
          }
        }
      }
    }
    if (button2Flag == 1)
    {
      for (int i = 0; i < 200; i++)
      {
        bpm();
        EVERY_N_MILLISECONDS( 20 ) {
          gHue++;  // slowly cycle the "base color" through the rainbow
        }
        FastLED.show();
        if(i % 10 == 0)
        { 
          midiNoteOn(0, 60 + (noteCount * 19) - noteTotal, 127);
         
          midiNoteOff(0, 60 + (noteCount *18) - noteTotal, 127);
          noteTotal += noteCount;
          if(noteTotal == noteCount * 20)
          {
            noteTotal = 0;
            midiNoteOff(0, 60, 127);
          }
        }
      }
    }
  }
}

byte colorPick()
{
  gHue = random(0, 255);
  while ( (gHue < (oldRandInt + 50) % 255) && (gHue > (oldRandInt - 50) % 255) )
  {
    gHue = random(0, 255);
  }

  oldRandInt = gHue;

}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 120;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for ( int i = 0; i < numLEDS; i++) { //9948
    strip[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
  }
}


void checkButtonCount()
{
  if (button1Flag == 1)
  {

    yesVote = yesVote + 1;
    //FUNCTION TO WEB CALL HERE
    //sendYes();
    button1Flag = 0;
  }
  if (button2Flag == 1)
  {
    noVote = noVote + 1;
    //sendNo();
    //FUNCTION TO WEB CALL HERE
    button2Flag = 0;
  }
  button1Interrupt = 0;
  button2Interrupt = 0;
}


//Theatre-style crawling lights.
void outwardTheaterChase(uint8_t wait) {
  CRGBPalette16 palette = PartyColors_p;
  Serial.print("RAND INT : ");
  Serial.println(gHue);
  for (int j = 0; j < 10; j++) { //do 10 cycles of chasing
    for (int q = 0; q < 3; q++) {
      for (int i = 0; i < numLEDS / 2; i = i + 3) {
        strip[i + q] = ColorFromPalette(palette, gHue, 255);  //turn every third pixel on
      }
      for (int k = numLEDS; k > numLEDS / 2 + 1; k = k - 3) {
        strip[k - q] = ColorFromPalette(palette, gHue, 255);  //turn every third pixel on
      }
      FastLED.show();

      delay(wait);

      for (int i = 0; i < numLEDS / 2; i = i + 3) {
        strip[i + q] = CRGB(0, 0, 0);      //turn every third pixel off
      }
      for (int k = numLEDS; k > numLEDS / 2 + 1; k = k - 3) {
        strip[k - q] = CRGB(0, 0, 0);      //turn every third pixel off
      }
    }
  }
}

void inwardTheaterChase( uint8_t wait) {
  CRGBPalette16 palette = PartyColors_p;
  for (int j = 0; j < 10; j++) { //do 10 cycles of chasing
    for (int q = 0; q < 3; q++) {
      for (uint16_t i = numLEDS / 2; i < numLEDS; i = i + 3) {
        strip[i + q] = ColorFromPalette(palette, gHue, 255);  //turn every third pixel on
      }

      for (int k = (numLEDS / 2 - 1); k > 0; k = k - 3) {
        strip[k - q] = ColorFromPalette(palette, gHue, 255);  //turn every third pixel on
      }
      FastLED.show();

      delay(wait);

      for (uint16_t i = numLEDS / 2; i < numLEDS; i = i + 3) {
        strip[i + q] = CRGB(0, 0, 0);  //turn every third pixel on
      }
      for (int k = (numLEDS / 2 - 1); k > 0; k = k - 3) {
        strip[k - q] = CRGB(0, 0, 0);  //turn every third pixel on
      }
    }
  }
}



void midiSetInstrument(uint8_t chan, uint8_t inst) {
  if (chan > 15) return;
  inst --; // page 32 has instruments starting with 1 not 0 :(
  if (inst > 127) return;

  VS1053_MIDI.write(MIDI_CHAN_PROGRAM | chan);
  VS1053_MIDI.write(inst);
}


void midiSetChannelVolume(uint8_t chan, uint8_t vol) {
  if (chan > 15) return;
  if (vol > 127) return;

  VS1053_MIDI.write(MIDI_CHAN_MSG | chan);
  VS1053_MIDI.write(MIDI_CHAN_VOLUME);
  VS1053_MIDI.write(vol);
}

void midiSetChannelBank(uint8_t chan, uint8_t bank) {
  if (chan > 15) return;
  if (bank > 127) return;

  VS1053_MIDI.write(MIDI_CHAN_MSG | chan);
  VS1053_MIDI.write((uint8_t)MIDI_CHAN_BANK);
  VS1053_MIDI.write(bank);
}

void midiNoteOn(uint8_t chan, uint8_t n, uint8_t vel) {
  if (chan > 15) return;
  if (n > 127) return;
  if (vel > 127) return;

  VS1053_MIDI.write(MIDI_NOTE_ON | chan);
  VS1053_MIDI.write(n);
  VS1053_MIDI.write(vel);
}

void midiNoteOff(uint8_t chan, uint8_t n, uint8_t vel) {
  if (chan > 15) return;
  if (n > 127) return;
  if (vel > 127) return;

  VS1053_MIDI.write(MIDI_NOTE_OFF | chan);
  VS1053_MIDI.write(n);
  VS1053_MIDI.write(vel);
}

