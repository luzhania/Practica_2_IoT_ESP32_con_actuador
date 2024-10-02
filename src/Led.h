#include <Arduino.h>
#pragma once

class Led
{
private:
  unsigned int pin;
  unsigned char state;

public:
  Led() : pin(0), state(LOW) {}

  Led(unsigned int pin)
  {
    this->pin = pin;
    pinMode(pin, OUTPUT);
    state = LOW;
  }

  void turnOn()
  {
    digitalWrite(pin, HIGH);
    state = HIGH;
  }

  void turnOff()
  {
    digitalWrite(pin, LOW);
    state = LOW;
  }
};