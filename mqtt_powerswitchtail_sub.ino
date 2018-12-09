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

const int POWERSWITCHTAIL_PIN = D1;
const int BAUD_SPEED = 115200;

const char* NETWORK_SSID = "";
const char* NETWORK_PASSWORD = "";

const char* MQTT_SERVER = "";
const int MQTT_PORT = 1883;
const char* MQTT_USER = "";
const char* MQTT_PASSWORD = "";
const char* MQTT_CLIENT_NAME = "WemosD1MiniPowerswitchTailClient";
const char* MQTT_TOPIC = "";

WiFiClient espClient;
PubSubClient client(espClient);

void setup()
{
  Serial.begin(BAUD_SPEED);
  delay(10);
  
  pinMode(POWERSWITCHTAIL_PIN, OUTPUT);  // prepare GPIO2

  //  Connect to the wifi network.
  WiFi.begin(NETWORK_SSID, NETWORK_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  
  Serial.println("Connected to the WiFi network");

  // Connect to the MQTT server.
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(callback);

  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect(MQTT_CLIENT_NAME, MQTT_USER, MQTT_PASSWORD )) {
      Serial.println("connected");  
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
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

void loop()
{
  client.loop();
}
