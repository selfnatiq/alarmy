#include <Arduino.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>

#include "Clock.h"

// ds1302 pins
#define CLK A5
#define DAT A4
#define RST A2

#define MINUTE 60 * 1000L /* ms */

// storage keys in nvram
#define NVRAM_ADDR_ALARM_ENABLED 0
#define NVRAM_ADDR_ALARM_HOUR 1
#define NVRAM_ADDR_ALARM_MINUTE 2

// initializers
ThreeWire rtcWires(DAT, CLK, RST);
RtcDS1302<ThreeWire> Rtc(rtcWires);

// assign values to class members
Clock::Clock()
    : _alarm_state(ALARM_OFF), _alarm_hour(0), _alarm_minute(0)
{
}

/**
 * This method initializes RTC and adjust time on rtc with current time on system
 * then it runs some if checks to make sure that RTC is running correctly
 *
 * at the end, it reads from rtc memory the keys that are declared on line 15 - 17
 */
void Clock::begin()
{
  Rtc.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  Serial.println();

  if (!Rtc.IsDateTimeValid())
  {
    // Common Causes:
    //    1) first time you ran and the device wasn't running yet
    //    2) the battery on the device is low or even missing

    Serial.println("RTC lost confidence in the DateTime!");
    Rtc.SetDateTime(compiled);
  }

  if (Rtc.GetIsWriteProtected())
  {
    Serial.println("RTC was write protected, enabling writing now");
    Rtc.SetIsWriteProtected(false);
  }

  if (!Rtc.GetIsRunning())
  {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }

  RtcDateTime now = Rtc.GetDateTime();
  if (now < compiled)
  {
    Serial.println("RTC is older than compile time!  (Updating DateTime)");
    Rtc.SetDateTime(compiled);
  }
  else if (now > compiled)
  {
    Serial.println("RTC is newer than compile time. (this is expected)");
  }
  else if (now == compiled)
  {
    Serial.println("RTC is the same as compile time! (not expected but all is fine)");
  }

  _alarm_state = Rtc.GetMemory(NVRAM_ADDR_ALARM_ENABLED) ? ALARM_OFF : ALARM_DISABLED;
  _alarm_hour = Rtc.GetMemory(NVRAM_ADDR_ALARM_HOUR) % 24;
  _alarm_minute = Rtc.GetMemory(NVRAM_ADDR_ALARM_MINUTE) % 60;
}

/**
 * This method will return true if alarmTime matches the current time
 *
 */
bool Clock::_isAlarmDueTime()
{
  auto currentTime = now();
  auto alarm = alarmTime();
  return ((currentTime.Hour() == alarm.Hour()) && (currentTime.Minute() == alarm.Minute()));
}

/**
 * Checks for if alarm is enabled
 * returns true if enabled
 */
bool Clock::alarmEnabled()
{
  return _alarm_state != ALARM_DISABLED;
}

/**
 * This method will return true if alarm is enabled and _isAlarmDueTime method return true
 *
 */
bool Clock::alarmActive()
{
  if (_alarm_state == ALARM_ACTIVE)
    return true;

  if (_alarm_state == ALARM_DISABLED)
    return false;

  if (_alarm_state == ALARM_OFF)
  {
    if (_isAlarmDueTime())
    {
      _alarm_state = ALARM_ACTIVE;
      return true;
    }

    return false;
  }

  if (_alarm_state == ALARM_STOPPED)
  {
    if (!_isAlarmDueTime())
      _alarm_state = ALARM_OFF;

    return false;
  }

  return false;
}

/**
 * This methods stop the alarm by setting _alarm_state to stopped
 *
 */
void Clock::stopAlarm()
{
  _alarm_state = ALARM_STOPPED;
}

/**
 * This method toggles that alarm state and also saves it memory
 *
 */
void Clock::toggleAlarm()
{
  bool enabled = !alarmEnabled();
  _alarm_state = enabled ? ALARM_OFF : ALARM_DISABLED;
  Rtc.SetMemory((const uint8_t)NVRAM_ADDR_ALARM_ENABLED, enabled);
}

/**
 * This method enables the alarm and alse updates the memory
 *
 */
void Clock::enableAlarm()
{
  _alarm_state = ALARM_OFF;
  Rtc.SetMemory((const uint8_t)NVRAM_ADDR_ALARM_ENABLED, true);
}

/**
 * This method disables the alarm and updates the memory
 *
 */
void Clock::disableAlarm()
{
  _alarm_state = ALARM_DISABLED;
  Rtc.SetMemory((const uint8_t)NVRAM_ADDR_ALARM_ENABLED, false);
}

/**
 * Returns the current time from RTC
 *
 */
RtcDateTime Clock::now()
{
  return Rtc.GetDateTime();
}

/**
 * Returns the alarm time that is stored in the memory
 *
 */
RtcDateTime Clock::alarmTime()
{
  char timeString[7];
  sprintf(timeString, "%02u:%02u:00", _alarm_hour, _alarm_minute);

  RtcDateTime now = Rtc.GetDateTime();
  RtcDateTime alarm = RtcDateTime(__DATE__, timeString);

  return alarm;
}

/**
 * This method increments alarm hour by 1 and updates the it to memory
 *
 */
void Clock::incrementAlarmHour()
{
  _alarm_hour = (_alarm_hour + 1) % 24;
  _alarm_state = ALARM_OFF;
  Rtc.SetMemory((const uint8_t)NVRAM_ADDR_ALARM_HOUR, _alarm_hour);
}

/**
 * This method increments alarm minute by 1 and updates the it to memory
 *
 */
void Clock::incrementAlarmMinute()
{
  _alarm_minute = (_alarm_minute + 1) % 60;
  _alarm_state = ALARM_OFF;
  Rtc.SetMemory((const uint8_t)NVRAM_ADDR_ALARM_MINUTE, _alarm_minute);
}
