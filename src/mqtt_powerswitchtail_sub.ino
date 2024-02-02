/*
 * Author:  Alexan Mardigian
 * 
 * This sketch will turn a Powerswitch Tail 2 either on or off, via MQTT
 * subscription.
 * 
 * 
 * This sketch has been created for the Wemos D1 Mini, but can be modified
 * to run on other ESP8266 boards.  After uploading the code to the Wemos D1 Mini,
 * wire the D1 pin to "+ in" on the Powerswitch Tail.  Then wire GND on the Wemos 
 * D1 mini to "- in" on the Powerswitch Tail.
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>
#include "secrets.h"

#define ENABLE_SSL

const int POWERSWITCHTAIL_PIN = D1;
const int BAUD_SPEED = 115200;

#ifdef ENABLE_SSL
  const int MQTT_PORT = 8883;

  X509List caCertX509(caCert);
  WiFiClientSecure espClient;
#else
  const int MQTT_PORT = 1883;
  WiFiClient espClient;
#endif

PubSubClient client(espClient);

void connect_to_mqtt_server();
void callback(char* topic, byte* payload, unsigned int length);

void setup()
{
  Serial.begin(BAUD_SPEED);
  delay(10);
  
  pinMode(POWERSWITCHTAIL_PIN, OUTPUT);  // prepare GPIO2

  WiFiManager wifiManager;
  wifiManager.autoConnect("PowerSwitchTail-AP");

#ifdef ENABLE_SSL
  // Configure secure client connection.
  espClient.setTrustAnchors(&caCertX509);         // Load CA cert into trust store.
  espClient.allowSelfSignedCerts();               // Enable self-signed cert support.
  espClient.setFingerprint(mqttCertFingerprint);  // Load SHA1 mqtt broker cert fingerprint for connection validation.
#endif

  // Connect to the MQTT server.
  client.setServer(MQTT_SERVER_IP, MQTT_PORT);
  client.setCallback(callback);

  connect_to_mqtt_server();
}

void loop()
{
  if (!client.connected()) {
    connect_to_mqtt_server();
  }

  client.loop();
}

void connect_to_mqtt_server()
{
  Serial.print("Connecting to MQTT server ");
  Serial.print(MQTT_SERVER_IP);
  Serial.print("..");
  
  while (!client.connected()) {
    Serial.print(".");
 
    if (client.connect(MQTT_CLIENT_NAME, MQTT_USER, MQTT_PASSWORD )) {
      Serial.println("connected.");  
    } else {
      Serial.print("failed with state ");
      Serial.println(client.state());
      delay(2000);
    }
  }

  client.publish(MQTT_TOPIC, "Hello from ESP8266 Powerswitch Tail");
  client.subscribe(MQTT_TOPIC);
}

void callback(char* topic, byte* payload, unsigned int length)
{
  const int ON = 1;

  int lampState = *payload - 48;  // Convert ASCII data to an integer (either 1 or 0).

  if (strcmp(topic, MQTT_TOPIC) == 0) {  
    if (lampState == ON) {
      digitalWrite(POWERSWITCHTAIL_PIN, HIGH);
      Serial.println("Powerswitch tail ON");
    } else {
      digitalWrite(POWERSWITCHTAIL_PIN, LOW);
      Serial.println("Powerswitch tail OFF");
    }
  }
}