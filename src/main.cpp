#include <WiFi.h>

#define LED_QUANTITY 3

class Led {
private:
  unsigned int pin;
  unsigned char state;

public:
  Led() : pin(0), state(LOW) {}

  Led(unsigned int pin) {
    this->pin = pin;
    pinMode(pin, OUTPUT);
    state = LOW;
  }

  void turnOn() {
    digitalWrite(pin, HIGH);
    state = HIGH;
  }

  void turnOff() {
    digitalWrite(pin, LOW);
    state = LOW;
  }
};

// Definimos la clase ActuatorObserver como parte del patrón Observer
class ActuatorObserver {
public:
    virtual void update(int state) = 0;
};

// Adaptamos la clase LedController para manejar los estados y seguir el patrón Observer
class LedController : public ActuatorObserver {
private:
    Led usedLeds[LED_QUANTITY];
    unsigned int lastLed = 0;

public:
    LedController() : lastLed(0) {}

    void addLed(unsigned int pin) {
        if (lastLed < LED_QUANTITY) {
            usedLeds[lastLed] = Led(pin);
            lastLed++;
        }
    }

    void update(int state) override {
        controlLedsBasedOnState(state);
    }

private:
    void controlLedsBasedOnState(int state) {
        switch (state) {
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

    void controlLeds(int ledsToTurnOn) {
        for (int i = 0; i < lastLed; i++) {
            if (i < ledsToTurnOn) {
                usedLeds[i].turnOn();
            } else {
                usedLeds[i].turnOff();
            }
        }
    }
};

// Clase para manejar la conexión TCP y recibir los estados del servidor
class TCPClient {
private:
    WiFiClient client;
    const char* ssid = "HUAWEI-2.4G-M6xZ";
    const char* password = "HT7KU2Xv";
    const char* host = "192.168.100.11";
    const int port = 8080;
    ActuatorObserver* observer;

    bool isConnected = false;

public:
    TCPClient(ActuatorObserver* obs) : observer(obs) {}

    void setupWiFi() {
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) {
            delay(1000);
            Serial.println("Conectando al WiFi...");
        }
        Serial.println("Conectado al WiFi");
    }

    void connectToServer() {
        if (!client.connected()) {
            if (client.connect(host, port)) {
                Serial.println("Conectado al servidor");
                client.println("REGISTER");
                isConnected = true;
            } else {
                Serial.println("Error al conectar con el servidor");
                isConnected = false;
            }
        }
    }

    void listenToServer() {
        if (isConnected && client.connected()) {
            if (client.available()) {
                String response = client.readStringUntil('\n');
                Serial.print("Recibido del servidor: ");
                Serial.println(response);
                int state = parseServerResponse(response);
                observer->update(state);
            }
        } else {
            isConnected = false;
            client.stop(); // Cerrar la conexión si no está conectada
        }
    }

    bool isClientConnected() {
        return isConnected;
    }

private:
    int parseServerResponse(const String& response) {
        int state = -1;
        int index = response.indexOf("State updated to:");
        if (index != -1) {
            state = response.substring(index + 17).toInt();
        }
        return state;
    }
};

LedController ledController;
TCPClient tcpClient(&ledController);

void setup() {
    Serial.begin(115200);
    tcpClient.setupWiFi();

    // Configurar LEDs
    ledController.addLed(32);
    ledController.addLed(33);
    ledController.addLed(25);
}

void loop() {
    if (!tcpClient.isClientConnected()) {
        Serial.println("Intentando reconectar al servidor...");
        tcpClient.connectToServer();
        delay(5000);  // Espera antes de intentar reconectar
    } else {
        tcpClient.listenToServer();
    }
    delay(100);  // Pausa en el loop
}