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

## secrets.h
Before you compile and upload to your board, you will need to:

1. Copy `src/secrets.h_example` to `src/secrets.h`
2. Enter the details of your MQTT server in secrets.h.

If you do not want to use TLS encrpytion, simply comment out `"#define ENABLE_SSL"` in mqtt_powerswitchtail_pub.ino.

## Connecting to your WiFi network
This project uses the WiFiManager.h library.  Once your XIAO ESP32-C6 board has been successfully flashed, and either hasn't been connected to wifi before or to a network it recognizes, it will act as a WiFi access point (AP).  The AP serves a web interface that will allow you to add your WiFi network's credentials.  Once added, the AP turns off and the XIAO ESP32-C6 will join the WiFi network you specified.  To do this:

1. Connect to the `PowerSwitchTail-AP` access point.
2. In a web browser, go to `192.168.4.1`
3. Tap the Configure WiFi button.
4. Add your WiFi network's credentials.