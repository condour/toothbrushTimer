#include <Arduino.h>
#include <TM1637Display.h>
#include <pitches.h>
#include <LowPower.h>

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif
#define DS_PIN 9
#define SHIFT_PIN 2
#define LATCH_PIN 3
#define LED_CLOCK 6
#define LED_DATA 5
// #define RESET_PIN 1
#define QUADRANT_1 A0
#define QUADRANT_2 A1
#define QUADRANT_3 A2
#define QUADRANT_4 A3
#define LED_BUILTIN 13
#define BEEPER 16
#define CHANNEL 8
#define MAX_TOOTH_COUNT 128
#define MAX_ALARM_COUNT 32

#define TIMING_MODE 1
#define ALARM_MODE 2
#define OFF_MODE 0

typedef struct
{
  unsigned int tone;
  unsigned int duration;
} Note;

Note charge[] = {
 { NOTE_G5, 1}, 
 { NOTE_C6, 1}, 
 { NOTE_E6, 1}, 
 { NOTE_G6, 1},
 { 0, 2}, 
 { NOTE_E6, 1}, 
 { NOTE_G6, 3},
 '\0'
  };
  Note buck[] = {
      {NOTE_C7, 1},
      {NOTE_B6, 1},
      {NOTE_G6, 1}, '\0'};
TM1637Display display(LED_CLOCK, LED_DATA);

int toothCount = 0, alarmCount = 0;
int currentMode = TIMING_MODE;
const uint8_t SEG_DONE[] = {
    SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,         // d
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F, // O
    SEG_C | SEG_E | SEG_G,                         // n
    SEG_A | SEG_D | SEG_E | SEG_F | SEG_G          // E
};
const uint8_t data[] = {0xff, 0xff, 0xff, 0xff};
const uint8_t blank[] = {0x00, 0x00, 0x00, 0x00};
const int quadrantPins[] = {QUADRANT_1, QUADRANT_2, QUADRANT_3, QUADRANT_4};
// struct Button
// {
//  const uint8_t PIN;
//  uint32_t numberKeyPresses;
//  bool pressed;
// };
// Button resetButton = {RESET_PIN, 0, false};
void arpeggio(Note *notes, int tempo, int lengthOfSong, bool doLights)
{

  int i = 0;
  while (i <= lengthOfSong)
  {
    Note tempNote = notes[i];
    
    tone(BEEPER, tempNote.tone, tempo*tempNote.duration);
    if(doLights) {
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DS_PIN, SHIFT_PIN, LSBFIRST, i);
    digitalWrite(LATCH_PIN, HIGH);

    }
    i++;
    delay(tempo * tempNote.duration);
    
  }
  noTone(BEEPER);
  Serial.println("got to end of song");
  return;
}



void changeMode(int mode)
{
  noTone(BEEPER);
  alarmCount = 0;
  if (mode == ALARM_MODE)
  {
    display.setSegments(SEG_DONE);
  }
  currentMode = mode;
}

/* void isr(void)
{
  Serial.println("key press");
  resetButton.numberKeyPresses++;
} */

void toothTimer()
{
  if (currentMode == TIMING_MODE)
  {
    int thisValue = toothCount;
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DS_PIN, SHIFT_PIN, LSBFIRST, thisValue);
    digitalWrite(LATCH_PIN, HIGH);

    const bool isQuadrantChange = toothCount == 0 || toothCount == 32 || toothCount == 64 || toothCount == 96;
    if (isQuadrantChange)
    {
      //  int arr[] = { NOTE_D6, NOTE_F6, NOTE_G6, NOTE_B6, '\0' };
      //  arpeggio(arr, 200);
       arpeggio(buck, 75, 3, false);
      digitalWrite(quadrantPins[toothCount / 32], HIGH);
      if(toothCount == 0) {
        delay(150);
         arpeggio(buck, 75, 3, false);
      }
    }
    display.showNumberDec(MAX_TOOTH_COUNT - toothCount);
    // Serial.println("showing number");
    if (toothCount == MAX_TOOTH_COUNT)
    {
      changeMode(ALARM_MODE);
    }
    toothCount++;
    delay(isQuadrantChange ? 850 : 1000);
  }
  if (currentMode == ALARM_MODE)
  {

    arpeggio(charge, 100, 7, true);

    delay(sizeof(charge) * 100);
    alarmCount++;
    if (alarmCount == MAX_ALARM_COUNT)
    {
      changeMode(OFF_MODE);
    }
  }
  if (currentMode == OFF_MODE)
  {
    digitalWrite(BEEPER, LOW);
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DS_PIN, SHIFT_PIN, LSBFIRST, 0);
    digitalWrite(LATCH_PIN, HIGH);

    display.setSegments(blank);
    for (int i = 0; i < 4; i++)
    {

      digitalWrite(quadrantPins[i], LOW);
    }

    digitalWrite(LED_BUILTIN, LOW);
    //  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
    // delay(1000);
  }
}

void setup()
{

  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(1000);
  display.setBrightness(0x09);
  Serial.println(currentMode);
  Serial.println("in setup");
  // All segments on
  display.setSegments(data);

  pinMode(DS_PIN, OUTPUT);
  pinMode(SHIFT_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(QUADRANT_1, OUTPUT);
  pinMode(QUADRANT_2, OUTPUT);
  pinMode(QUADRANT_3, OUTPUT);
  pinMode(QUADRANT_4, OUTPUT);
  // pinMode(RESET_PIN, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(BEEPER, LOW);

  delay(500);
  // attachInterrupt(digitalPinToInterrupt(RESET_PIN), isr, FALLING);
  // resetButton.numberKeyPresses = 0;
  // delay(1000);

  //  digitalWrite(CLEAR_PIN, LOW);
  //  digitalWrite(CLEAR_PIN, HIGH);
}

void loop()
{
  // put your main code here, to run repeatedly:
  toothTimer();
}