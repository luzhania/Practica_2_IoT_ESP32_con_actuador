#include "LedObserver.h"
#include "ActuatorClient.h"
#include "Utilities.h"


LedObserver ledObserver;
ActuatorClient actuatorClient("Galaxy S9+7c14", "betitox007.,", &ledObserver);

void setup()
{
  ledObserver.addLed(32);
  ledObserver.addLed(33);
  ledObserver.addLed(25);
  actuatorClient.setup();
}

void loop()
{
  Utilities::NonBlockingDelay(100, []()
                              { actuatorClient.loop(); });
}