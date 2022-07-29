#include <Arduino.h>
#include <TM1637Display.h>
#include <pitches.h>
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
#define QUADRANT_1 A0
#define QUADRANT_2 A1
#define QUADRANT_3 A2
#define QUADRANT_4 A3

#define BEEPER 16
#define CHANNEL 8
#define MAX_TOOTH_COUNT 128
#define MAX_ALARM_COUNT 8

#define TIMING_MODE 1
#define ALARM_MODE 2
#define OFF_MODE 0

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
/* struct Button
{
  const uint8_t PIN;
  uint32_t numberKeyPresses;
  bool pressed;
};
 */

void arpeggio(unsigned int *notes, int tempo, int lengthOfSong)
{

  int i = 0;
  while (i <= lengthOfSong)
  {
    tone(BEEPER, notes[i++], tempo);
    delay(tempo);
  }
  noTone(BEEPER);
  return;
}

void buckaroo()
{
  unsigned int arr[] = {NOTE_C7, NOTE_B6, NOTE_G6, '\0'};
  arpeggio(arr, 75, 3);
}

void changeMode(int mode)
{
  noTone(BEEPER);
  alarmCount = toothCount = 0;
  if (mode == ALARM_MODE)
  {
    display.setSegments(SEG_DONE);
  }
  if (mode == OFF_MODE)
  {
    display.setSegments(blank);
    for (int i = 0; i < 4; i++)
    {

      digitalWrite(quadrantPins[i], LOW);
    }
  }
  currentMode = mode;
}
/*
 void isr(void) {
  resetButton.numberKeyPresses++;
  Serial.println("IN RESET?");
  if (currentMode == OFF_MODE)
  {
    changeMode(TIMING_MODE);
  }
  else
  {
    changeMode(OFF_MODE);
  }
}
 */
void toothTimer()
{

  if (currentMode == TIMING_MODE)
  {
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DS_PIN, SHIFT_PIN, MSBFIRST, toothCount);
    digitalWrite(LATCH_PIN, HIGH);
    if (toothCount == 0 || toothCount == 32 || toothCount == 64 || toothCount == 96)
    {
      //  int arr[] = { NOTE_D6, NOTE_F6, NOTE_G6, NOTE_B6, '\0' };
      //  arpeggio(arr, 200);
      buckaroo();
      digitalWrite(quadrantPins[toothCount / 32], HIGH);
      {
        delay(150);
        buckaroo();
      }
    }
    display.showNumberDec(MAX_TOOTH_COUNT - toothCount);
    if (toothCount == MAX_TOOTH_COUNT)
    {
      changeMode(ALARM_MODE);
    }
    toothCount++;

    delay(1000);
  }
  if (currentMode == ALARM_MODE)
  {
    unsigned int arr2[] = {NOTE_C6, NOTE_E6, NOTE_G6, NOTE_C7, NOTE_G6, NOTE_E5, NOTE_C6, NOTE_C5, NOTE_C6, '\0'};
    arpeggio(arr2, 100, 9);

    delay(sizeof(arr2) * 100);
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
    shiftOut(DS_PIN, SHIFT_PIN, MSBFIRST, 0);
    digitalWrite(LATCH_PIN, HIGH);
    delay(1000);
  }
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  display.setBrightness(0x0f);

  // All segments on
  display.setSegments(data);

  pinMode(DS_PIN, OUTPUT);
  pinMode(SHIFT_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(QUADRANT_1, OUTPUT);
  pinMode(QUADRANT_2, OUTPUT);
  pinMode(QUADRANT_3, OUTPUT);
  pinMode(QUADRANT_4, OUTPUT);
  //  attachInterrupt(digitalPinToInterrupt(RESET_PIN), isr, FALLING);
  digitalWrite(BEEPER, LOW);
  //  digitalWrite(CLEAR_PIN, LOW);
  //  digitalWrite(CLEAR_PIN, HIGH);
}

void loop()
{
  // put your main code here, to run repeatedly:
  toothTimer();
}