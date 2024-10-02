#include "Led.h"
#pragma once

#define LED_QUANTITY 3

class Observer
{
public:
  virtual void update(int state) = 0;
};

class LedObserver : public Observer
{
private:
  Led usedLeds[LED_QUANTITY];
  unsigned int lastLed = 0;

public:
  LedObserver() : lastLed(0) {}

  void addLed(unsigned int pin)
  {
    if (lastLed < LED_QUANTITY)
    {
      usedLeds[lastLed] = Led(pin);
      lastLed++;
    }
  }

  void update(int state) override
  {
    controlLedsBasedOnState(state);
  }

private:
  void controlLedsBasedOnState(int state)
  {
    switch (state)
    {
    case 0:
      controlLeds(3);
      break;
    case 1:
      controlLeds(2);
      break;
    case 2:
      controlLeds(1);
      break;
    case 3:
      controlLeds(0);
      break;
    default:
      controlLeds(0);
      break;
    }
  }

  void controlLeds(int ledsToTurnOn)
  {
    for (int i = 0; i < lastLed; i++)
    {
      if (i < ledsToTurnOn)
      {
        usedLeds[i].turnOn();
      }
      else
      {
        usedLeds[i].turnOff();
      }
    }
    
  }
};