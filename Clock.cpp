#include <Arduino.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>

#include "Clock.h"

// ds1302 pins
#define CLK A5
#define DAT A4
#define RST A2

#define MINUTE 60 * 1000L /* ms */

#define NVRAM_ADDR_ALARM_ENABLED 0
#define NVRAM_ADDR_ALARM_HOUR 1
#define NVRAM_ADDR_ALARM_MINUTE 2

ThreeWire rtcWires(DAT, CLK, RST);
RtcDS1302<ThreeWire> Rtc(rtcWires);

Clock::Clock()
    : _alarm_state(ALARM_OFF), _alarm_hour(0), _alarm_minute(0)
{
}

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

bool Clock::_isAlarmDueTime()
{
  auto currentTime = now();
  auto alarm = alarmTime();
  return ((currentTime.Hour() == alarm.Hour()) && (currentTime.Minute() == alarm.Minute()));
}

bool Clock::alarmEnabled()
{
  return _alarm_state != ALARM_DISABLED;
}

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

void Clock::stopAlarm()
{
  _alarm_state = ALARM_STOPPED;
}

void Clock::toggleAlarm()
{
  bool enabled = !alarmEnabled();
  _alarm_state = enabled ? ALARM_OFF : ALARM_DISABLED;
  Rtc.SetMemory((const uint8_t)NVRAM_ADDR_ALARM_ENABLED, enabled);
}

void Clock::enableAlarm()
{
  _alarm_state = ALARM_OFF;
  Rtc.SetMemory((const uint8_t)NVRAM_ADDR_ALARM_ENABLED, true);
}

void Clock::disableAlarm()
{
  _alarm_state = ALARM_DISABLED;
  Rtc.SetMemory((const uint8_t)NVRAM_ADDR_ALARM_ENABLED, false);
}

RtcDateTime Clock::now()
{
  return Rtc.GetDateTime();
}

RtcDateTime Clock::alarmTime()
{
  char timeString[7];
  sprintf(timeString, "%02u:%02u:00", _alarm_hour, _alarm_minute);

  RtcDateTime now = Rtc.GetDateTime();
  RtcDateTime alarm = RtcDateTime(__DATE__, timeString);

  return alarm;
}

void Clock::incrementAlarmHour()
{
  _alarm_hour = (_alarm_hour + 1) % 24;
  _alarm_state = ALARM_OFF;
  Rtc.SetMemory((const uint8_t)NVRAM_ADDR_ALARM_HOUR, _alarm_hour);
}

void Clock::incrementAlarmMinute()
{
  _alarm_minute = (_alarm_minute + 1) % 60;
  _alarm_state = ALARM_OFF;
  Rtc.SetMemory((const uint8_t)NVRAM_ADDR_ALARM_MINUTE, _alarm_minute);
}
