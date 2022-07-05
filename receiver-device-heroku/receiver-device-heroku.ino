#include <SPI.h> //responsável pela comunicação serial
#include <LoRa.h> //responsável pela comunicação com o WIFI Lora
#include <Wire.h>  //responsável pela comunicação i2c
#include "SSD1306.h" //responsável pela comunicação com o display
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "WebSocketStreamClient.h"

// Definição dos pinos 
#define SCK     5    // GPIO5  -- SX127x's SCK
#define MISO    19   // GPIO19 -- SX127x's MISO
#define MOSI    27   // GPIO27 -- SX127x's MOSI
#define SS      18   // GPIO18 -- SX127x's CS
#define RST     14   // GPIO14 -- SX127x's RESET
#define DI00    26   // GPIO26 -- SX127x's IRQ(Interrupt Request)

#define BAND    915E6  //Frequencia do radio - podemos utilizar ainda : 433E6, 868E6, 915E6
#define PABOOST true

// config
#define NET_SSID "VIVO-9C08"
#define NET_PASSWORD "C662349C08"
#define MQTT_BROKER "monitoring-landslides-broker.herokuapp.com"
#define MQTT_PORT 443
#define PATH "/mqtt"
#define GATEWAY_ID "62479bcee7668ac9a912b280"
#define BROKER_USER_NAME "Cefet-Broker"
#define BROKER_PASSWORD "78A23Erg"
#define MQTT_MILLIS_TOPIC "vibrationData"

//parametros: address,SDA,SCL 
SSD1306 display(0x3c, 4, 15); //construtor do objeto que controlaremos o display

String rssi = "RSSI --";
String packSize = "--";
String packet ;
char buffer[256];

WiFiClientSecure wiFiClient;
WebSocketClient wsClient(wiFiClient, MQTT_BROKER, MQTT_PORT);
WebSocketStreamClient wsStreamClient(wsClient, PATH);
PubSubClient MQTT(wsStreamClient);

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
}

void setupMQTT() {
  
   if (MQTT.connected())
        return;
   
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
}

void cbk(int packetSize) {
  packet ="";
  packSize = String(packetSize,DEC); //transforma o tamanho do pacote em String para imprimirmos
  for (int i = 0; i < packetSize; i++) { 
    packet += (char) LoRa.read(); //recupera o dado recebido e concatena na variável "packet"
  }
  rssi = "(RSSI) = " + String(LoRa.packetRssi(), DEC) + "dB"; //configura a String de Intensidade de Sinal (RSSI)
  display.clear();
  display.drawString(0, 0, "Signal strength"); 
  display.drawString(0, 20, rssi); 
  display.display();
  packet.toCharArray(buffer, 256);
  MQTT.publish(MQTT_MILLIS_TOPIC, buffer);
  digitalWrite(2, LOW);
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
  
  setupWifi();
  MQTT.setBufferSize(512);
  setupMQTT();
  delay(1000);

  SPI.begin(SCK,MISO,MOSI,SS); //inicia a comunicação serial com o Lora
  LoRa.setPins(SS,RST,DI00); //configura os pinos que serão utlizados pela biblioteca (deve ser chamado antes do LoRa.begin)
  
  //inicializa o Lora com a frequencia específica.
  if (!LoRa.begin(BAND)) {
    display.clear();
    display.drawString(0, 0, "Starting LoRa failed!");
    display.display();
    while (1);
  }

  //indica no display que inicilizou corretamente.
  display.clear();
  display.drawString(0, 0, "LoRa Initial success!");
  display.drawString(0, 10, "Wait for incoming data...");
  display.display();
  delay(1000);

  //LoRa.onReceive(cbk);
  LoRa.receive(); //habilita o Lora para receber dados
}

void loop() {
  int packetSize = LoRa.parsePacket();
   if (packetSize) { 
      digitalWrite(2, HIGH);   // liga o LED indicativo
      delay(500);
      cbk(packetSize);  
  } 

  setupWifi();
  setupMQTT();
}
