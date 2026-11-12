/*
 * Author:  Alexan Mardigian
 *
 * MQTT subscriber that switches a PowerSwitch Tail 2 on or off.
 *
 * PORTED for the Seeed Studio XIAO ESP32-C6 (originally Wemos D1 Mini / ESP8266).
 *
 * Wiring:
 *   XIAO "D1" pad   ->  "+ in" on the PowerSwitch Tail 2
 *   XIAO "GND" pad  ->  "- in" on the PowerSwitch Tail 2
 *
 * Toolchain:
 *   - Arduino-ESP32 core 3.0.0+ (required for ESP32-C6 support).
 *       Boards Manager -> "esp32 by Espressif Systems" -> install latest.
 *       Tools -> Board -> select "XIAO_ESP32C6".
 *   - WiFiManager (tzapu) latest release (older versions break on core 3.x).
 *   - PubSubClient.
 *   - MQTTManager (Techno-Hwizrdry/mqtt-manager, dev branch).
 *
 * WiFi and MQTT settings are entered through a captive portal, not in source
 * code.  On first boot, connect to the "PowerSwitchTail-AP" access point,
 * browse to 192.168.4.1, and fill in your WiFi network and the "MQTT Broker"
 * section on the same page.  The MQTT settings are stored in flash by the
 * MQTTManager library.
 *
 * To change the MQTT settings later, hold the BOOT button for 3 seconds while
 * the board is running.  The stored MQTT settings are erased and the
 * "PowerSwitchTail-MQTT" access point starts; connect to it and browse to
 * 192.168.4.1.
 *
 * TLS: the broker's (self-signed) certificate is pinned with the CA in
 * broker_ca.h, so the password is only sent to your real broker.
 */

#include <WiFi.h>
#include <time.h>
#include <PubSubClient.h>
#include <WiFiManager.h>
#include "MQTTManager.h"
#include "MQTTWiFiManager.h"
#include "broker_ca.h"

const int POWERSWITCHTAIL_PIN = D1;   // top "D1" pad on the XIAO (a safe GPIO).
const int RESET_BUTTON_PIN    = 9;    // BOOT button on the XIAO ESP32-C6.
const int BAUD_SPEED = 115200;

const char* WIFI_AP_NAME = "PowerSwitchTail-AP";
const char* MQTT_AP_NAME = "PowerSwitchTail-MQTT";

const unsigned long RESET_HOLD_MS = 3000;  // Hold BOOT this long to re-enter MQTT setup.

unsigned long lastMsgTime = 0;
const unsigned long KEEPALIVE_INTERVAL = 60000;  // 60 seconds

unsigned long buttonPressedAt = 0;
bool          buttonHeld      = false;

WiFiManager wifiManager;
MQTTManager mqttManager(MQTT_AP_NAME);
MQTTWiFiManagerFields mqttFields(mqttManager, 8883, true);  // defaults: port 8883, TLS on

void onMqttConnect();
void onMqttPortalStart();
void callback(char* topic, byte* payload, unsigned int length);

// The topic entered in the portal (empty until a config is saved).
const char* mqttTopic() { return mqttManager.config().topic; }

void setup()
{
  Serial.begin(BAUD_SPEED);
  delay(10);

  pinMode(POWERSWITCHTAIL_PIN, OUTPUT);
  digitalWrite(POWERSWITCHTAIL_PIN, LOW);  // lamp starts off
  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);

  /* --- Optional: external U.FL antenna ---
   * The onboard ceramic antenna is the default and needs no code.
   * Uncomment below ONLY if you attach an external antenna:
   *
   * pinMode(WIFI_ENABLE, OUTPUT);
   * digitalWrite(WIFI_ENABLE, LOW);        // GPIO3: power the RF switch
   * delay(100);
   * pinMode(WIFI_ANT_CONFIG, OUTPUT);
   * digitalWrite(WIFI_ANT_CONFIG, HIGH);   // GPIO14: select external antenna
   */

  // WiFi and MQTT are configured on one portal page. If WiFi can't be joined
  // (e.g. the router is still booting after a power cut), give up after
  // 3 minutes and reboot to try again instead of waiting in the portal forever.
  mqttFields.addTo(wifiManager);
  wifiManager.setConnectRetries(3);
  wifiManager.setConfigPortalTimeout(180);
  if (!wifiManager.autoConnect(WIFI_AP_NAME)) {
    ESP.restart();
  }

  // Connect to the MQTT server, using the settings saved from the portal.
  // If none are saved yet, MQTTManager starts its own setup portal.
  mqttManager.setCACert(caCert);
  mqttManager.onConnect(onMqttConnect);
  mqttManager.onMessage(callback);
  mqttManager.onPortalStart(onMqttPortalStart);
  mqttManager.autoConnect();

  const MQTTConfig& cfg = mqttManager.config();
  Serial.printf("MQTT broker %s:%u, TLS %s, user \"%s\", topic \"%s\"\n",
                cfg.host, cfg.port, cfg.tlsEnabled ? "on" : "off", cfg.username, cfg.topic);
}

void loop()
{
  // Keeps the MQTT connection alive, reconnects every 5 s if it drops,
  // and runs the setup portal when it is active.
  mqttManager.loop();

  // Periodic heartbeat to keep the connection alive and surface silent failures.
  PubSubClient& client = mqttManager.getClient();
  unsigned long now = millis();

  // While the broker is unreachable, report why every 5 s.
  // PubSubClient states: -4 timeout, -2 connect/TLS failed, 4 bad user/password, 5 not authorized.
  static unsigned long lastStatus = 0;
  if (mqttManager.config().host[0] != '\0' && !client.connected() && now - lastStatus > 5000) {
    lastStatus = now;
    Serial.printf("MQTT not connected, state %d, clock %ld\n", client.state(), (long)time(nullptr));
  }

  // Typing "reset" in the Serial Monitor does the same as holding BOOT.
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd == "reset") {
      Serial.println("Erasing MQTT settings and starting the setup portal.");
      mqttManager.resetSettings();
    }
  }
  if (client.connected() && now - lastMsgTime > KEEPALIVE_INTERVAL) {
    lastMsgTime = now;
    client.publish("powerswitch/heartbeat", "alive");
  }

  // Hold BOOT to erase the MQTT settings and open the MQTT setup portal.
  if (digitalRead(RESET_BUTTON_PIN) == LOW) {
    if (!buttonHeld) {
      buttonHeld      = true;
      buttonPressedAt = now;
    } else if (buttonPressedAt != 0 && now - buttonPressedAt >= RESET_HOLD_MS) {
      Serial.println("BOOT held, erasing MQTT settings and starting the setup portal.");
      buttonPressedAt = 0;  // fire once per hold
        mqttManager.resetSettings();
    }
  } else {
    buttonHeld = false;
  }
}

// Called by MQTTManager every time it connects (or reconnects) to the broker.
void onMqttConnect()
{
  Serial.println("Connected to MQTT server.");
  PubSubClient& client = mqttManager.getClient();

  client.publish(
    "debug/powerswitchtail",
    "Hello from XIAO ESP32-C6 PowerSwitch Tail"
  );

  if (mqttTopic()[0] != '\0') {
    client.subscribe(mqttTopic());
  } else {
    Serial.println("No MQTT topic is set. Hold BOOT 3 s to enter one.");
  }
}

void onMqttPortalStart()
{
  Serial.print("MQTT setup portal started. Connect to the ");
  Serial.print(MQTT_AP_NAME);
  Serial.println(" access point and browse to 192.168.4.1");
}

void callback(char* topic, byte* payload, unsigned int k)
{
  if (strcmp(topic, mqttTopic()) != 0 || k == 0) return;

  char cmd = (char)payload[0];

  if (cmd == '1') {
    digitalWrite(POWERSWITCHTAIL_PIN, HIGH);
    Serial.println("Powerswitch tail ON");
  } else if (cmd == '0') {
    digitalWrite(POWERSWITCHTAIL_PIN, LOW);
    Serial.println("Powerswitch tail OFF");
  }
}
