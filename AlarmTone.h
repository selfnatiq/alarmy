#ifndef __ALARM_TONE_H__
#define __ALARM_TONE_H__

/**
 * AlarmTone class responsible for playing and stoping the alarm
 *
 */
class AlarmTone
{
public:
  AlarmTone(uint8_t pin);
  void begin();
  void play();
  void stop();

private:
  uint8_t _pin;
  bool _playing;
  uint8_t _tone_index;
  unsigned long _last_tone_time;
};

#endif /* __ALARM_TONE_H */
