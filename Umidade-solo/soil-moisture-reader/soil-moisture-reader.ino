#include "SSD1306.h" //responsável pela comunicação com o display
#include <WiFi.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "WebSocketStreamClient.h"
#include "ArduinoJson.h"

// config
#define SENSOR_PIN  34 
#define NET_SSID "VIVO-9C08"
#define NET_PASSWORD "C662349C08"
#define MQTT_BROKER "monitoring-landslides-broker.herokuapp.com"
#define MQTT_PORT 443
#define PATH "/mqtt"
#define GATEWAY_ID "6397c1ff6724af41e9d914d6"
#define BROKER_USER_NAME "Cefet-Broker"
#define BROKER_PASSWORD "78A23Erg"
#define MQTT_HUMIDITY_TOPIC "Humidity"
#define MQTT_PING_TOPIC "Ping"
#define GATEWAY_ID "Soil Moisture Sensor"

//parametros: address,SDA,SCL 
SSD1306 display(0x3c, 4, 15); //construtor do objeto que controlaremos o display

//Declaracao da variavel que armazena as leituras do sensor
int leitura_sensor = 0;

//Declaracao das variaveis que armazenam os valores de calibracao
const int VALOR_MAXIMO = 634; //Valor com solo seco
const int VALOR_MINIMO = 304; //Valor com solo umido

WiFiClientSecure wiFiClient;
WebSocketClient wsClient(wiFiClient, MQTT_BROKER, MQTT_PORT);
WebSocketStreamClient wsStreamClient(wsClient, PATH);
PubSubClient MQTT(wsStreamClient);

StaticJsonDocument<256> docHumidity;
char humidityBuffer[256];
char pingBuffer[256];
String deviceId = "6397c1ff6724af41e9d914d6";

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

  //Define o pino conectado ao sensor como uma entrada do sistema
  pinMode(SENSOR_PIN, INPUT);

  setupWifi();
  MQTT.setBufferSize(512);
  setupMQTT();
  docHumidity["deviceId"] = deviceId;
  
}

void loop() {

  //Realiza a leitura do sensor, a mapeia entre 0 e 100 % e exibe o valor no LCD
  leitura_sensor = analogRead(SENSOR_PIN);
  leitura_sensor = map(leitura_sensor, VALOR_MINIMO, VALOR_MAXIMO, 100, 0);
  
  if (!isnan(leitura_sensor)) 
  {
    digitalWrite(2, HIGH);   // liga o LED indicativo
    docHumidity["humidity"] = leitura_sensor;
    serializeJson(docHumidity, humidityBuffer);
    MQTT.publish(MQTT_HUMIDITY_TOPIC, humidityBuffer);
    delay(1000);
    digitalWrite(2, LOW);
  } 
  delay(300000);
  setupWifi();
  setupMQTT();


}
