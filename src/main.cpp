#include <WiFi.h>
#include <functional>

#define LED_QUANTITY 3

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

class WiFiConnection
{
private:
  const char *SSID;
  const char *PASSWORD;

public:
  WiFiConnection(const char *SSID, const char *PASSWORD)
      : SSID(SSID), PASSWORD(PASSWORD) {}

  void connect()
  {
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.println("Conectando a WiFi...");
    }
    Serial.println("Conectado a WiFi");
  }
};

const char *SERVER_HOST = "192.168.100.11";
const unsigned int SERVER_PORT = 8080;

class ActuatorClient
{
private:
  Observer *ledObserver;
  WiFiClient wifiClient;
  WiFiConnection *wifiConnection;
  bool isConnected = false;

public:
  ActuatorClient(const char *SSID, const char *PASSWORD, Observer *obs) : ledObserver(obs)
  {
    wifiConnection = new WiFiConnection(SSID, PASSWORD);
  }

  ~ActuatorClient()
  {
    delete wifiConnection;
  }

  void setup()
  {
    Serial.begin(115200);
    wifiConnection->connect();
  }

  void loop()
  {
    if (!isConnected)
    {
      Serial.println("Intentando reconectar al servidor...");
      connectToServer();
    }
    else
    {
      listenToServer();
    }
  }

private:
  void connectToServer()
  {
    if (!wifiClient.connected())
    {
      if (wifiClient.connect(SERVER_HOST, SERVER_PORT))
      {
        Serial.println("Conectado al servidor");
        wifiClient.print("REGISTER ACTUATOR");
        isConnected = true;
      }
      else
      {
        Serial.println("Error al conectar con el servidor");
        isConnected = false;
        delay(5000);
      }
    }
  }

  void listenToServer()
  {
    if (isConnected && wifiClient.connected())
    {
      if (wifiClient.available())
      {
        String response = wifiClient.readStringUntil('\n');
        Serial.print("Recibido del servidor: ");
        Serial.println(response);
        int state = parseServerResponse(response);
        ledObserver->update(state);
      }
    }
    else
    {
      isConnected = false;
      wifiClient.stop();
    }
  }

  int parseServerResponse(const String &response)
  {
    int spaceIndex = response.indexOf(' ');
    if (spaceIndex != -1)
    {
      String stateStr = response.substring(spaceIndex + 1);
      return stateStr.toInt();
    }
    return -1;
  }
};

LedObserver ledObserver;
ActuatorClient actuatorClient("HUAWEI-2.4G-M6xZ", "HT7KU2Xv", &ledObserver);

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