#include "Button.h"
#include <Arduino.h>

Button::Button(uint8_t pin, uint16_t debounce_ms)
    : _pin(pin), _delay(debounce_ms), _state(HIGH), _ignore_until(0), _has_changed(false)
{
}

void Button::begin()
{
  pinMode(_pin, INPUT_PULLUP);
}

bool Button::read()
{
  // ignore pin changes until after this delay time
  if (_ignore_until > millis())
  {
    // ignore any changes during this period
  }
  else if (!(analogRead(_pin) < 1000) != _state)
  {
    _state = !_state;

    _ignore_until = millis() + _delay;
    _has_changed = true;
  }

  return _state;
}

uint16_t Button::value()
{
  if (read() == PRESSED)
  {
    return analogRead(_pin);
  }

  return 1023; // max 10 bit value
}

// mostly internal, tells you if a button has changed after calling the read() function
bool Button::has_changed()
{
  if (_has_changed)
  {
    _has_changed = false;
    return true;
  }
  return false;
}

// has the button gone from off -> on or pressed
bool Button::pressed()
{
  return (read() == PRESSED && has_changed());
}

// has the button gone from on -> off
bool Button::released()
{
  return (read() == RELEASED && has_changed());
}

char Button::direction()
{
  uint16_t v = value();

  if (v < 100)
  {
    return 'R';
  }

  if (v < 200)
  {
    return 'U';
  }

  if (v < 400)
  {
    return 'D';
  }

  if (v < 600)
  {
    return 'L';
  }

  if (v < 800)
  {
    return 'S';
  }
}
