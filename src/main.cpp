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
#define CLEAR_PIN 4
#define RESET_PIN 7
#define LED_CLOCK 6
#define LED_DATA 5
#define BEEPER 16
#define CHANNEL 8
#define MAX_TOOTH_COUNT 8
#define MAX_ALARM_COUNT 8

#define TIMING_MODE 1
#define ALARM_MODE 2
#define OFF_MODE 0

void tone(uint32_t note)
{
  
    ledcAttachPin(BEEPER, CHANNEL);
  int status = ledcSetup(CHANNEL, 50, 12);
  Serial.printf("status? %d, note? %d\n", status, note);
  ledcWriteTone(CHANNEL, note);
}
void noTone(void)
{
  tone(0);
}

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
struct Button
{
  const uint8_t PIN;
  uint32_t numberKeyPresses;
  bool pressed;
};

Button resetButton = {RESET_PIN, 0, false};
void buckaroo()
{
  tone(NOTE_C7);
  vTaskDelay(100);
  tone(NOTE_B6);
  vTaskDelay(100);
  tone(NOTE_G6);
  vTaskDelay(100);
  noTone();
}

void arpeggio(int *notes, int tempo)
{
  int lengthOfSong = sizeof(notes);
  int i = 0;
  while (i < lengthOfSong)
  {
    tone(notes[i++]);
    vTaskDelay(tempo);
  }
  noTone();
  return;
}
void changeMode(int mode)
{
  digitalWrite(CLEAR_PIN, LOW);
  digitalWrite(CLEAR_PIN, HIGH);
  noTone(BEEPER);
  alarmCount = toothCount = 0;
  if (mode == ALARM_MODE)
  {
    display.setSegments(SEG_DONE);
  }
  if (mode == OFF_MODE)
  {
    display.setSegments(blank);
  }
  currentMode = mode;
}

void IRAM_ATTR isr()
{
  resetButton.numberKeyPresses++;
  if (currentMode == OFF_MODE)
  {
    changeMode(TIMING_MODE);
  }
  else
  {
    changeMode(OFF_MODE);
  }
}

void toothTimer(void *pvParameters)
{
  for (;;)
  {
    if (resetButton.numberKeyPresses)
    {
      Serial.printf("number key presses %d\n", resetButton.numberKeyPresses);
      resetButton.numberKeyPresses = 0;
    }
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
      }
      display.showNumberDec(MAX_TOOTH_COUNT - toothCount);
      toothCount++;
      if (toothCount == MAX_TOOTH_COUNT)
      {
        changeMode(ALARM_MODE);
      }

      vTaskDelay(1000);
    }
    if (currentMode == ALARM_MODE)
    {
      int arr[] = {1047, 1319, 1568, '\0'};
      arpeggio(arr, 100);
      vTaskDelay(60);
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
    }
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
  pinMode(CLEAR_PIN, OUTPUT);
  pinMode(RESET_PIN, INPUT_PULLUP);
  attachInterrupt(RESET_PIN, isr, FALLING);
  digitalWrite(BEEPER, LOW);
  digitalWrite(CLEAR_PIN, LOW);
  digitalWrite(CLEAR_PIN, HIGH);

  xTaskCreatePinnedToCore(
      toothTimer,
      "toothTimer",
      10000,
      NULL,
      1,
      NULL,
      ARDUINO_RUNNING_CORE);
}

void loop()
{
  // put your main code here, to run repeatedly:
}