#include <ArduinoMqttClient.h>
#if defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_NANO_33_IOT) || defined(ARDUINO_AVR_UNO_WIFI_REV2)
  #include <WiFiNINA.h>
#elif defined(ARDUINO_SAMD_MKR1000)
  #include <WiFi101.h>
#elif defined(ARDUINO_ARCH_ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ARDUINO_PORTENTA_H7_M7) || defined(ARDUINO_NICLA_VISION) || defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_GIGA) || defined(ARDUINO_OPTA)
  #include <WiFi.h>
#elif defined(ARDUINO_PORTENTA_C33)
  #include <WiFiC3.h>
#elif defined(ARDUINO_UNOR4_WIFI)
  #include <WiFiS3.h>
#endif

#include "arduino_secrets.h"
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char broker[] = "192.168.1.61";
int port = 1883;

const char led_topic[] = "arduino/led";
const char led_state_topic[] = "arduino/led/state";

// Pins
static const int led_pin = 2;

int connect_to_wifi() {
  int max_tries = 24;
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    if (--max_tries == 0) {
      Serial.println(" Failed to connect to WiFi");
      return WiFi.status();
    }
  }
  return WiFi.status();
}

void reboot_esp(char *message) {
  Serial.println(message);
  Serial.println("Rebooting...");
  ESP.restart();
}

void publish_current_status() {
  if (digitalRead(led_pin) == HIGH) {
    mqttClient.beginMessage(led_state_topic);
    mqttClient.print("ON");
    mqttClient.endMessage();
  } else {
    mqttClient.beginMessage(led_state_topic);
    mqttClient.print("OFF");
    mqttClient.endMessage();
  }
}

void setup() {
  pinMode(led_pin, OUTPUT);
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ;
  }

  int connection_status = connect_to_wifi();

  if (connection_status != WL_CONNECTED) {
    reboot_esp("Failed to connect to WiFi");
  }

  Serial.println("You're connected to the network");
  Serial.println();

    mqttClient.setUsernamePassword("harmonie", "Harmonie02!");
    while (!mqttClient.connect(broker, port)) {
      Serial.print("MQTT connection failed! Error code = ");
      Serial.println(mqttClient.connectError());
      Serial.println("Retrying MQTT connection in 5 seconds...");
      delay(5000);
      if (WiFi.status() != WL_CONNECTED) {
        connect_to_wifi();
      }
    }

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();

  Serial.print("Subscribing to topic: ");
  Serial.println(led_topic);
  Serial.println();

  mqttClient.subscribe(led_topic);

  Serial.print("Waiting for messages on topic: ");
  Serial.println(led_topic);
  Serial.println();

  publish_current_status();
}

void loop() {
  int messageSize = mqttClient.parseMessage();
  if (messageSize) {
    String incomingTopic = mqttClient.messageTopic();
    String payload = "";
    while (mqttClient.available()) {
      payload += (char)mqttClient.read();
    }

    Serial.print("Received a message with topic '");
    Serial.print(incomingTopic);
    Serial.print("', length ");
    Serial.print(messageSize);
    Serial.println(" bytes:");
    Serial.println(payload);

    if (incomingTopic == led_topic) {
      if (payload == "ON") {
        digitalWrite(led_pin, HIGH);
        Serial.println("LED ON");
        publish_current_status();
      } else if (payload == "OFF") {
        digitalWrite(led_pin, LOW);
        Serial.println("LED OFF");
        publish_current_status();
      }
    }
    Serial.println();
  }
}