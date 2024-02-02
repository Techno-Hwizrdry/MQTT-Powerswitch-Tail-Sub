# MQTT-Powerswitch-Tail-Sub 
A MQTT subscriber to turn off and on a Powerswitch Tail 2, using a the WeMos D1 mini ESP8266 board.

## Description
This arduino sketch will switch a Powerswitch Tail 2 either on or off, via MQTT subscription.

The code was created for the Wemos D1 Mini, but can be modified to run on other ESP8266 boards.
After uploading the code to the Wemos D1 Mini, wire the D1 pin to "+ in" on the Powerswitch Tail.
Then wire GND on the Wemos D1 mini to "- in" on the Powerswitch Tail.

## Prerequistes
This code was written and tested with VScode and PlatformIO.  VScode was also used to upload the sketch to the board.
The steps to set up the tool chain is:

0. [Download](https://www.python.org/downloads/) and install Python 3.5 or higher.
1. [Download](https://code.visualstudio.com/download) and install VSCode.
2. In VScode, download and install the following VScode extensions:
    - Arduino
    - C/C++
    - C/C++ Extension Pack
    - PlatformIO IDE

## secrets.h
Before you compile and upload to your board, you will need to:

1. Copy `src/secrets.h_example` to `src/secrets.h`
2. Enter the details of your MQTT server in secrets.h.

In secrets.h, your `mqttCertFingerprint[]` should look something like this:

`const uint8_t mqttCertFingerprint[] = {0xAA,0xBB,0xCC,0xDD,0xEE,0xFF,0x11,0x22,0x33,0x44,0x55,0x1A,0x69,0x42,0x00,0x80,0x08,0x55,0xB0,0x0B};`

If you do not want to use TLS encrpytion, simply comment out `"#define ENABLE_SSL"` in mqtt_powerswitchtail_pub.ino.