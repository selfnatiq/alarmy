#include <Arduino.h>
#include "AlarmTone.h"

#define TONE_TIME 500    /* ms */
#define TONE_SPACING 100 /* ms */

static const uint16_t TONES[] = {
    500,
    800,
};
const uint16_t NUM_TONES = sizeof(TONES) / sizeof(TONES[0]);

AlarmTone::AlarmTone(uint8_t pin)
    : _pin(pin), _playing(false), _tone_index(0), _last_tone_time(0)
{
}

void AlarmTone::begin()
{
  pinMode(_pin, OUTPUT);
}

void AlarmTone::play()
{
  if (!_playing || _last_tone_time + TONE_TIME + TONE_SPACING < millis())
  {
    tone(_pin, TONES[_tone_index], TONE_TIME);
    _tone_index = (_tone_index + 1) % NUM_TONES;
    _last_tone_time = millis();
  }
  _playing = true;
}

void AlarmTone::stop()
{
  noTone(_pin);
  _tone_index = 0;
  _playing = false;
}
