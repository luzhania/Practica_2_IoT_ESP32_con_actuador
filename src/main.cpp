#include <Arduino.h>
#include <WiFi.h>
#define LED_QUANTITY 3
class Led
{
private:
  unsigned int pin;
  unsigned char state;

public:
  Led() : pin(0), state(LOW)
  {
  }

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

// Definimos la clase ActuatorObserver como parte del patrón Observer
class ActuatorObserver {
public:
    virtual void update(int state) = 0;
};

// Adaptamos la clase LedController para manejar los estados y seguir el patrón Observer
class LedController : public ActuatorObserver {
private:
    Led usedLeds[LED_QUANTITY];  // LED_QUANTITY es la cantidad máxima de LEDs
    unsigned int lastLed = 0;

public:
    LedController() : lastLed(0) {}

    void addLed(unsigned int pin) {
        if (lastLed < LED_QUANTITY) {
            usedLeds[lastLed] = Led(pin);
            lastLed++;
        }
    }

    // Implementación del método update del patrón Observer para manejar los estados
    void update(int state) override {
        controlLedsBasedOnState(state);
    }

private:
    // Método para controlar los LEDs basado en el estado recibido
    void controlLedsBasedOnState(int state) {
        switch (state) {
            case 0:
                controlLeds(3);  // Encender 3 LEDs
                break;
            case 1:
                controlLeds(2);  // Encender 2 LEDs
                break;
            case 2:
                controlLeds(1);  // Encender 1 LED
                break;
            case 3:
                controlLeds(0);  // Apagar todos los LEDs
                break;
            default:
                controlLeds(0);  // Estado inválido, apagar todos los LEDs
                break;
        }
    }

    // Método que enciende/apaga los LEDs según el número indicado
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
    ActuatorObserver* observer;  // Referencia al observador (LedController en este caso)

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

    bool connectToServer() {
        if (client.connect(host, port)) {
            Serial.println("Conectado al servidor");
            client.println("REGISTER");
            return true;
        } else {
            Serial.println("Error al conectar con el servidor");
            return false;
        }
    }

    void listenToServer() {
        if (client.connected()) {
            if (client.available()) {
                String response = client.readStringUntil('\n');
                Serial.print("Recibido del servidor: ");
                Serial.println(response);
                int state = parseServerResponse(response);
                observer->update(state);  // Notifica al observador (LedController)
            }
        }
    }

private:
    int parseServerResponse(const String& response) {
        // Extraer el estado del servidor, se asume que el mensaje es algo como "State updated to: X"
        int state = -1;
        int index = response.indexOf("State updated to:");
        if (index != -1) {
            state = response.substring(index + 17).toInt();  // Extraemos el estado
        }
        return state;
    }
};

LedController ledController;  // Instanciamos el controlador de LEDs
TCPClient tcpClient(&ledController);  // Asignamos el LedController como observador

void setup() {
    Serial.begin(115200);
    tcpClient.setupWiFi();

    // Configurar LEDs
    ledController.addLed(32);   // Asignamos el pin del primer LED
    ledController.addLed(33);  // Asignamos el pin del segundo LED
    ledController.addLed(25);  // Asignamos el pin del tercer LED
}

void loop() {
    if (!tcpClient.connectToServer()) {
        Serial.println("Reintentando conexión al servidor...");
        delay(5000);  // Espera antes de intentar reconectar
    } else {
        tcpClient.listenToServer();
    }
    delay(100);  // Pequeña pausa en el loop
}
