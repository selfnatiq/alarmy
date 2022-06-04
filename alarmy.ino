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

// initializers
LiquidCrystal lcd(PIN_RS, PIN_EN, PIN_D4, PIN_D5, PIN_D6, PIN_D7);
Clock clock;
AlarmTone alarmTone(A3);
Button btn(A0);
Reader rd(3, 2);

// bell symbol
byte bell[8] = {
    B00100,
    B01110,
    B01110,
    B01110,
    B01110,
    B11111,
    B00000,
    B00100};

// pipe symbol
byte pipe[] = {
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111};

/**
 * Display states for displaying different elements
 *
 * DisplayClock: Shows current time
 * DisplayAlarmTime: User can set/edit alarm time
 * DisplayAlarmActive: Shows current time with a WAKE UP message
 */
enum DisplayState
{
  DisplayClock,
  DisplayAlarmTime,
  DisplayAlarmActive,
};

// default display state
DisplayState displayState = DisplayClock;

// tracks last state change
unsigned long lastStateChange = 0;

/**
 * Returns millis since last state change
 */
long millisSinceStateChange()
{
  return millis() - lastStateChange;
}

/**
 * Displays the given message on first lcd row
 */
void setDisplayStatus(const char *value)
{
  lcd.setCursor(0, 0);
  lcd.print(value);
}

/**
 * Sets display state to the given state
 * updates the lastStateChange
 */
void setDisplayState(DisplayState state)
{
  displayState = state;
  lastStateChange = millis();
}

/**
 * Gets the current time from rtc
 * formats it to human readable and displays it on lcd
 */
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

/**
 * Display state for current time with status
 * listens for button events
 *
 * if alarm is active? display state will change to DisplayAlarmActive state
 */
void clockState()
{
  setDisplayStatus("Date    Time    ");
  displayTime();

  if (btn.read() == Button::RELEASED && clock.alarmActive())
  {
    btn.has_changed();
    setDisplayState(DisplayAlarmActive);
    return;
  }

  if (btn.pressed())
  {
    if (btn.direction() == 'S')
      setDisplayState(DisplayAlarmTime);

    if (btn.direction() == 'R')
      clock.toggleAlarm();
  }
}

/**
 * Display when DisplayState is DisplayAlarmTime
 * User can set or edit alarm time
 * After setting alarm the alarm will be set
 */
void alarmTimeState()
{
  setDisplayStatus("   *Set Alarm*    ");
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
      clock.enableAlarm();
      setDisplayState(DisplayClock);
    }
  }
}

/**
 * Display when alarm is triggered!
 * listens for button events
 * User can turn off alarm by scaning the right NFC card
 */
void alarmActiveState()
{
  displayTime();

  if (btn.read() == Button::RELEASED)
    alarmTone.play();

  if (rd.validate() == 'A')
    setDisplayStatus("    Wake Up     ");

  if (rd.validate() == 'F')
    setDisplayStatus(" Invalid card!  ");

  if (rd.validate() == 'T')
  {
    alarmTone.stop();
    clock.stopAlarm();
    setDisplayState(DisplayClock);
  }
}

/**
 * setup to initialze all classes
 * print the brand message on lcd panel
 * also creates the defined symbols
 */
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

/**
 * Switch statment to display different displays states
 */
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
