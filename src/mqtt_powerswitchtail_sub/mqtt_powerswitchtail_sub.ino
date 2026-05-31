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
 *
 * What changed from the ESP8266 version (and why):
 *   - TLS is mbedTLS here, not BearSSL. setTrustAnchors(), setFingerprint(),
 *     and allowSelfSignedCerts() do NOT exist on ESP32. The broker's
 *     self-signed certificate is pinned with setCACert() instead (a self-signed
 *     cert is its own CA, so this validates it correctly and stays secure).
 *   - setCACert() validation checks the certificate's notBefore/notAfter dates,
 *     so the system clock MUST be set via NTP before the TLS connect.
 *     sync_time() is therefore now ENABLED (it was commented out before).
 *   - WPA3 + PMF are handled natively by the ESP32-C6 radio; no code needed.
 *     You can set your router to use WPA3 / PMF Required and this still connects.
 *   - The XIAO ESP32-C6 uses its onboard ceramic antenna by default; no antenna
 *     code is required. See the optional external-antenna block in setup().
 *
 * secrets.h must define: caCert (the broker's certificate, as a PEM C-string),
 *   MQTT_SERVER_IP, MQTT_CLIENT_NAME, MQTT_USER, MQTT_PASSWORD, MQTT_TOPIC.
 */

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <WiFiManager.h>
#include "secrets.h"

#define ENABLE_SSL

const int POWERSWITCHTAIL_PIN = D1;   // top "D1" pad on the XIAO (a safe GPIO).
const int BAUD_SPEED = 115200;

#ifdef ENABLE_SSL
  const int MQTT_PORT = 8883;
  WiFiClientSecure espClient;
#else
  const int MQTT_PORT = 1883;
  WiFiClient espClient;
#endif

PubSubClient client(espClient);

unsigned long lastMsgTime = 0;
const unsigned long KEEPALIVE_INTERVAL = 60000;  // 60 seconds

void connect_to_mqtt_server();
void callback(char* topic, byte* payload, unsigned int length);

void sync_time()
{
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Waiting for NTP time sync");

  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }

  Serial.println();
  Serial.print("Time synced: ");
  Serial.println(ctime(&now));
}

void setup()
{
  Serial.begin(BAUD_SPEED);
  delay(10);

  pinMode(POWERSWITCHTAIL_PIN, OUTPUT);
  digitalWrite(POWERSWITCHTAIL_PIN, LOW);  // lamp starts off

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

  WiFiManager wifiManager;
  wifiManager.autoConnect("PowerSwitchTail-AP");

#ifdef ENABLE_SSL
  sync_time();                  // REQUIRED before TLS (cert validity is checked)
  espClient.setCACert(caCert);  // pin the broker's (self-signed) certificate
#endif

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

  // Periodic heartbeat to keep the connection alive and surface silent failures.
  unsigned long now = millis();
  if (now - lastMsgTime > KEEPALIVE_INTERVAL) {
    lastMsgTime = now;
    client.publish("powerswitch/heartbeat", "alive");
  }
}

void connect_to_mqtt_server()
{
  Serial.print("Connecting to MQTT server ");
  Serial.print(MQTT_SERVER_IP);
  Serial.print("..");

  while (!client.connected()) {
    Serial.print(".");

    if (client.connect(MQTT_CLIENT_NAME, MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("connected.");
    } else {
      Serial.print("failed with state ");
      Serial.println(client.state());  // -2 = TLS/connect fail; check cert/time
      delay(2000);
    }
  }

  client.publish(MQTT_TOPIC, "Hello from XIAO ESP32-C6 PowerSwitch Tail");
  client.subscribe(MQTT_TOPIC);
}

void callback(char* topic, byte* payload, unsigned int length)
{
  const int ON = 1;

  int lampState = *payload - 48;  // ASCII '1'/'0' -> integer 1/0

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
