# MQTT-Powerswitch-Tail-Sub 
A MQTT subscriber to turn off and on a [Powerswitch Tail 2](https://www.adafruit.com/product/268), using the [Seeed Studio XIAO ESP32-C6](https://www.seeedstudio.com/Seeed-Studio-XIAO-ESP32C6-p-5884.html) board.

## Description
This arduino sketch will switch a Powerswitch Tail 2 either on or off, via MQTT subscription.

The code was created for the XIAO ESP32-C6.
After uploading the code to the XIAO ESP32-C6, wire the D1 pin to "+ in" on the Powerswitch Tail.
Then wire GND on the XIAO ESP32-C6 to "- in" on the Powerswitch Tail.

## Prerequistes
The steps to set up the tool chain is:

0. [Download](https://www.arduino.cc/en/software/) and install the Arduino IDE.
1. In Arduino IDE, bring up the Board Manager (Cntrl + Shift + B) and search for `esp32`.  Then click the Install button next to `esp32 by Espressif Systems`.
2. Bring up the Library manager (Cntrl + Shift + I) and search for `WiFiManager by tzapu`.  Then install that library.

## MQTT Manager
MQTT settings are no longer kept in `secrets.h`. They are entered in a captive portal and stored on the board by the [MQTTManager](https://github.com/Techno-Hwizrdry/mqtt-manager) library.

To install the library in the Arduino IDE:

1. Download the `dev` branch of mqtt-manager as a ZIP (or check out `dev` in your local clone and zip that folder).
2. **Sketch → Include Library → Add .ZIP Library…** and pick the ZIP.
3. In the Library Manager, install **PubSubClient** and **ArduinoJson** if you don't have them.

The portal fields are: broker host or IP, port (default 8883), username, password, client ID (leave blank to use one made from the board's MAC address), topic, and TLS (1 = on, 0 = off).

With TLS on, the broker's certificate is checked against `broker_ca.h` in the sketch folder. Put your broker's CA certificate (PEM) there.

To change the MQTT settings later, hold the BOOT button for 3 seconds while the board is running. The stored MQTT settings are erased and the `PowerSwitchTail-MQTT` access point starts. Connect to it and go to `192.168.4.1`.

## Connecting to your WiFi network
This project uses the WiFiManager.h library.  Once your XIAO ESP32-C6 board has been successfully flashed, and either hasn't been connected to wifi before or to a network it recognizes, it will act as a WiFi access point (AP).  The AP serves a web interface that will allow you to add your WiFi network's credentials.  Once added, the AP turns off and the XIAO ESP32-C6 will join the WiFi network you specified.  To do this:

1. Connect to the `PowerSwitchTail-AP` access point.
2. In a web browser, go to `192.168.4.1`
3. Tap the Configure WiFi button.
4. Add your WiFi network's credentials, and fill in the MQTT Broker section below them on the same page.