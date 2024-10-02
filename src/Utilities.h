#include <Arduino.h>
#include <functional>
#include <iostream>
#pragma once

using namespace std;

class Utilities
{
public:
  static void serialPrintNonBlockingDelay(unsigned long milliseconds, unsigned int cm)
  {
    static unsigned long lastMeasurement = 0;
    unsigned long currentMillis = millis();

    if (currentMillis - lastMeasurement >= milliseconds)
    {
      Serial.print(cm);
      Serial.println("cm");
      lastMeasurement = currentMillis;
    }
  }

  static void NonBlockingDelay(unsigned long milliseconds, function<void()> callback)
  {
    static unsigned long lastMeasurement = 0;
    unsigned long currentMillis = millis();

    if (currentMillis - lastMeasurement >= milliseconds)
    {
      callback();
      lastMeasurement = currentMillis;
    }
  }
};