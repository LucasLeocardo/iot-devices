#include "DHT.h"
#include "SSD1306.h" //responsável pela comunicação com o display
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "WebSocketStreamClient.h"
#include "ArduinoJson.h"

// config
#define DHT_PIN  17   // Pino de recebimento de dado do DHT
#define DHT_TYPE DHT22 // DHT22
#define NET_SSID "VIVO-9C08"
#define NET_PASSWORD "C662349C08"
#define MQTT_BROKER "monitoring-landslides-broker.herokuapp.com"
#define MQTT_PORT 443
#define PATH "/mqtt"
#define GATEWAY_ID "62479bcee7668ac9a912b280"
#define BROKER_USER_NAME "Cefet-Broker"
#define BROKER_PASSWORD "78A23Erg"
#define MQTT_TEMPERATURE_TOPIC "Temperature"
#define MQTT_HUMIDITY_TOPIC "Humidity"
#define MQTT_PING_TOPIC "Ping"
#define GATEWAY_ID "62479bcee7668ac9a912b280"

//parametros: address,SDA,SCL 
SSD1306 display(0x3c, 4, 15); //construtor do objeto que controlaremos o display
DHT dht(DHT_PIN, DHT_TYPE);

WiFiClientSecure wiFiClient;
WebSocketClient wsClient(wiFiClient, MQTT_BROKER, MQTT_PORT);
WebSocketStreamClient wsStreamClient(wsClient, PATH);
PubSubClient MQTT(wsStreamClient);

StaticJsonDocument<256> docHumidity;
StaticJsonDocument<256> docTemperature;
char humidityBuffer[256];
char temperatureBuffer[256];
char pingBuffer[256];
String deviceId = "631e04657175e9b83839da91";
float humidity, temperature;

void setupWifi() {
  //Configura a conexão à rede sem fio

  if (WiFi.status() == WL_CONNECTED)
        return;

  display.drawString(0, 0, "Connecting to WiFi");
  display.display();

  WiFi.disconnect();
  WiFi.begin(NET_SSID, NET_PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      display.drawString(0, 10, "......");
      display.display();
  }

  wiFiClient.setInsecure();
  display.clear();
  display.drawString(0, 0, "WiFi connected");
  display.drawString(0, 10, "IP address: " + WiFi.localIP());
  display.display();
  delay(1000);
}

void setupMQTT() {

   if (MQTT.connected())
        return;

   MQTT.setKeepAlive(60); 
   
   while (!MQTT.connected()) 
   {
        display.clear();
        display.drawString(0, 0, "Connecting to Broker");
        display.drawString(0, 10, "MQTT...");
        display.display();
        delay(2000);
        if (MQTT.connect(GATEWAY_ID, BROKER_USER_NAME, BROKER_PASSWORD)) 
        {
            display.clear();
            display.drawString(0, 0, "Successfully connected");
            display.drawString(0, 10, "to MQTT broker");
            display.drawString(0, 20, "Wait for incoming data...");
            display.display();
        } 
        else
        {
            display.clear();
            display.drawString(0, 0, "Failed to connect");
            display.drawString(0, 10, "There will be another connection");
            display.drawString(0, 20, "attempt in 10s");
            display.display();
            delay(10000);
        }
    }
    delay(1000);
    display.clear();
    display.drawString(0, 0, "Sending MQTT Data...");
    display.display();
}


void setup() {
  dht.begin();

  //configura os pinos como saida
  pinMode(16,OUTPUT); //RST do oled
  pinMode(2,OUTPUT);
  digitalWrite(16, LOW);    // reseta o OLED
  delay(50); 
  digitalWrite(16, HIGH); // enquanto o OLED estiver ligado, GPIO16 deve estar HIGH
  display.init();
  display.flipScreenVertically();  
  display.setFont(ArialMT_Plain_10);
  delay(1500);
  display.clear();
  
  setupWifi();
  MQTT.setBufferSize(512);
  setupMQTT();
  docHumidity["deviceId"] = deviceId;
  docTemperature["deviceId"] = deviceId;
}

void loop() 
{
  // A leitura da temperatura e umidade pode levar 250ms!
  // O atraso do sensor pode chegar a 2 segundos.
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  // testa se retorno é valido, caso contrário algo está errado.
  if (!isnan(temperature) && !isnan(humidity)) 
  {
    digitalWrite(2, HIGH);   // liga o LED indicativo
    docHumidity["humidity"] = humidity;
    docTemperature["temperature"] = temperature;
    serializeJson(docHumidity, humidityBuffer);
    MQTT.publish(MQTT_HUMIDITY_TOPIC, humidityBuffer);
    serializeJson(docTemperature, temperatureBuffer);
    MQTT.publish(MQTT_TEMPERATURE_TOPIC, temperatureBuffer);
    delay(1000);
    digitalWrite(2, LOW);
  } 
  for (int i = 0; i <= 2; i++) {
    MQTT.publish(MQTT_PING_TOPIC, pingBuffer);
    delay(40000);
  }
  setupWifi();
  setupMQTT();
}
