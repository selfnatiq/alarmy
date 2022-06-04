#ifndef __CLOCK_H__
#define __CLOCK_H__

#include <ThreeWire.h>
#include <RtcDS1302.h>

enum AlarmState
{
  ALARM_DISABLED,
  ALARM_OFF,
  ALARM_ACTIVE,
  ALARM_STOPPED,
};

class Clock
{
public:
  Clock();
  void begin();

  RtcDateTime now();
  RtcDateTime alarmTime();

  void incrementAlarmHour();
  void incrementAlarmMinute();
  void stopAlarm();
  void toggleAlarm();
  void enableAlarm();
  void disableAlarm();

  bool alarmEnabled();
  bool alarmActive();

protected:
  uint8_t _alarm_hour;
  uint8_t _alarm_minute;
  AlarmState _alarm_state;

  bool _isAlarmDueTime();
};

#endif /* __CLOCK_H__ */
