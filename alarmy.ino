#include <LiquidCrystal.h>
#include "Button.h"
#include "Clock.h"
#include "AlarmTone.h"
#include "Reader.h"

// lcd pins
#define PIN_D4 4
#define PIN_D5 5
#define PIN_D6 6
#define PIN_D7 7
#define PIN_RS 8
#define PIN_EN 9

LiquidCrystal lcd(PIN_RS, PIN_EN, PIN_D4, PIN_D5, PIN_D6, PIN_D7);
Clock clock;
AlarmTone alarmTone(A3);
Button btn(A0);
Reader rd(3, 2);

// icon bell
byte bell[8] = {
    B00100,
    B01110,
    B01110,
    B01110,
    B01110,
    B11111,
    B01000,
    B00100};

// icon pipe
byte pipe[] = {
    B01110,
    B01110,
    B01110,
    B01110,
    B01110,
    B01110,
    B01110,
    B01110};

enum DisplayState
{
  DisplayClock,
  DisplayAlarmTime,
  DisplayAlarmActive,
};

DisplayState displayState = DisplayClock;
unsigned long lastStateChange = 0;

long millisSinceStateChange()
{
  return millis() - lastStateChange;
}

void setDisplayStatus(const char *value)
{
  lcd.setCursor(0, 0);
  lcd.print(value);
}

void setDisplayState(DisplayState state)
{
  displayState = state;
  lastStateChange = millis();
}

void displayTime()
{
  RtcDateTime now = clock.now();
  char formattedDate[12];
  char formattedTime[12];

  sprintf(formattedDate, "%02u/%02u ", now.Day(), now.Month());
  sprintf(formattedTime, " %02u:%02u:%02u", now.Hour(), now.Minute(), now.Second());

  lcd.setCursor(0, 1);
  lcd.print(formattedDate);
  lcd.write(clock.alarmEnabled() ? 1 : 2);
  lcd.print(formattedTime);
}

void clockState()
{
  setDisplayStatus("***** Time *****");
  displayTime();

  if (btn.read() == Button::RELEASED && clock.alarmActive())
  {
    btn.has_changed();
    setDisplayState(DisplayAlarmActive);
    return;
  }

  if (btn.pressed() && btn.direction() == 'S')
  {
    clock.toggleAlarm();
    setDisplayState(DisplayAlarmTime);
  }
}

void alarmTimeState()
{
  setDisplayStatus("*** Set Alarm ***");
  RtcDateTime alarmTime = clock.alarmTime();

  char fDateTime[12];
  sprintf(fDateTime, "    %02u:%02u:00    ", alarmTime.Hour(), alarmTime.Minute());

  lcd.setCursor(0, 1);
  lcd.print(fDateTime);

  if (btn.pressed())
  {
    if (btn.direction() == 'U')
    {
      clock.incrementAlarmHour();
      lastStateChange = millis();
    }

    if (btn.direction() == 'D')
    {
      clock.incrementAlarmMinute();
      lastStateChange = millis();
    }

    if (btn.direction() == 'S')
    {
      setDisplayState(DisplayClock);
    }
  }
}

void alarmActiveState()
{
  setDisplayStatus("** WAKE UP!!! **");

  if (btn.read() == Button::RELEASED)
    alarmTone.play();

  if (rd.isValid())
  {
    alarmTone.stop();
    clock.stopAlarm();
    setDisplayState(DisplayClock);
  }
  else
  {
    setDisplayStatus("** Scan it!!! **");
  }
}

void setup()
{
  Serial.begin(9600);

  clock.begin();
  btn.begin();
  alarmTone.begin();
  rd.begin();

  lcd.createChar(1, bell);
  lcd.createChar(2, pipe);
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("---- Alarmy ----");
}

void loop()
{
  switch (displayState)
  {
  case DisplayClock:
    clockState();
    break;
  case DisplayAlarmTime:
    alarmTimeState();
    break;
  case DisplayAlarmActive:
    alarmActiveState();
    break;
  }
}
