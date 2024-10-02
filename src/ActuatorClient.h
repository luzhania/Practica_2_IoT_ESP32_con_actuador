#include "LedObserver.h"
#include "WiFiConnection.h"
#pragma once

const char *SERVER_HOST = "192.168.43.111";
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