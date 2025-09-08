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
const char motion_state_topic[] = "arduino/motion/state";
const char manual_topic[] = "arduino/manualoverride";
const char manual_state_topic[] = "arduino/manualoverride/state";

// Pins
static const int led_pin = 2;
static const int motion_pin = 13;

const int delay_time = 5000;
int state = LOW;
int val = 0;
int d = -1;
bool manualoverride = false;

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

void publish_current_led_status() {
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

void publish_current_motion_status() {
  if (digitalRead(motion_pin) == HIGH) {
    mqttClient.beginMessage(motion_state_topic);
    mqttClient.print("ON");
    mqttClient.endMessage();
  } else {
    mqttClient.beginMessage(motion_state_topic);
    mqttClient.print("OFF");
    mqttClient.endMessage();
  }
  if (manualoverride) {
    mqttClient.beginMessage(manual_state_topic);
    mqttClient.print("ON");
    mqttClient.endMessage();
  } else {
    mqttClient.beginMessage(manual_state_topic);
    mqttClient.print("OFF");
    mqttClient.endMessage();
  }
}

void setup() {
  pinMode(led_pin, OUTPUT);
  pinMode(motion_pin, INPUT);
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
  mqttClient.subscribe(manual_topic);

  Serial.print("Waiting for messages on topic: ");
  Serial.println(led_topic);
  Serial.println();

  publish_current_led_status();
  publish_current_motion_status();
}

void loop() {
  val = digitalRead(motion_pin);
  if (val != state) {
    publish_current_motion_status();
  }
  state = val;

  if (!manualoverride) {
    if (val == HIGH) {
      Serial.println("Motion detected!");
      if (digitalRead(led_pin) == LOW) {
        digitalWrite(led_pin, HIGH);
        publish_current_led_status();
      }
      d = millis() + delay_time;
    }
    if (d <= millis()) {
      if (digitalRead(led_pin) == HIGH) {
        Serial.println("No motion, turning led off");
        digitalWrite(led_pin, LOW);
        publish_current_led_status();
      }
    }
  }

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

    if (incomingTopic == led_topic && manualoverride) {
      if (payload == "ON") {
        digitalWrite(led_pin, HIGH);
        Serial.println("LED ON");
        publish_current_led_status();
      } else if (payload == "OFF") {
        digitalWrite(led_pin, LOW);
        Serial.println("LED OFF");
        publish_current_led_status();
      }
    }

    if (incomingTopic == manual_topic) {
      if (payload == "ON") {
        manualoverride = true;
        Serial.println("Manual override ON");
        mqttClient.beginMessage(manual_state_topic);
        mqttClient.print("ON");
        mqttClient.endMessage();
      } else if (payload == "OFF") {
        manualoverride = false;
        d = millis() + delay_time;
        Serial.println("Manual override OFF");
        mqttClient.beginMessage(manual_state_topic);
        mqttClient.print("OFF");
        mqttClient.endMessage();
      }
    }
    Serial.println();
  }
}