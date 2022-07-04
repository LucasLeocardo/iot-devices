# iot-devices
Repository with the scripts used in the devices of the slope slide monitoring project.
# How to send data to MQTT  over Websockets Heroku Broker
To send data from an ESP32 Lora to an MQTT Broker over Websockets you need to use, in addition to the normal MQTT and WiFi libraries, you will need to use the WebSocketStreamClient folder in this repository and include it in the library folder of your Arduino IDE.


If your MQTT message is longer than 128 bytes the data will not be sent. To resolve this issue, follow the steps below:
- add pubsubclient.setBufferSize(512)
- Make the following change in the websocket.h library in your arduino libraries folder => iTxBuffer[512]


In addition, it is also important to install the ESP32 board library with version 1.5 or greater.

Github chat link that helps with questions: https://github.com/areve/WebSocketStreamClient/issues/4 