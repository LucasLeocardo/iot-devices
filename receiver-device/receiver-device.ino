#include <SPI.h> //responsável pela comunicação serial
#include <LoRa.h> //responsável pela comunicação com o WIFI Lora
#include <Wire.h>  //responsável pela comunicação i2c
#include "SSD1306.h" //responsável pela comunicação com o display
#include <WiFi.h>
#include <PubSubClient.h>

// Definição dos pinos 
#define SCK     5    // GPIO5  -- SX127x's SCK
#define MISO    19   // GPIO19 -- SX127x's MISO
#define MOSI    27   // GPIO27 -- SX127x's MOSI
#define SS      18   // GPIO18 -- SX127x's CS
#define RST     14   // GPIO14 -- SX127x's RESET
#define DI00    26   // GPIO26 -- SX127x's IRQ(Interrupt Request)

#define BAND    915E6  //Frequencia do radio - podemos utilizar ainda : 433E6, 868E6, 915E6
#define PABOOST true

//Net Setup
#define NET_SSID "VIVO-9C08"
#define NET_PASSWORD "C662349C08"

//MQTT Setup
#define GATEWAY_ID "3238w19e5a995b97fk288d00"
#define MQTT_BROKER "192.168.15.10"
#define MQTT_PORT 1883
#define MQTT_MILLIS_TOPIC "vibrationData"

//parametros: address,SDA,SCL 
SSD1306 display(0x3c, 4, 15); //construtor do objeto que controlaremos o display

String rssi = "RSSI --";
String packSize = "--";
String packet ;
char buffer[256];

WiFiClient espClient; //Cliente de rede
PubSubClient MQTT(espClient); //Cliente MQTT

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

void setupWifi() {
  //Configura a conexão à rede sem fio

  if (WiFi.status() == WL_CONNECTED)
        return;

  display.drawString(0, 0, "Connecting to WiFi");
  display.display();
  
  WiFi.begin(NET_SSID, NET_PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      display.drawString(0, 10, "......");
      display.display();
  }

  display.clear();
  display.drawString(0, 0, "WiFi connected");
  display.drawString(0, 10, "IP address: " + WiFi.localIP());
  display.display();
}

void setupMQTT() {
  
   if (MQTT.connected())
        return;

   MQTT.setServer(MQTT_BROKER, MQTT_PORT);   //informa qual broker e porta deve ser conectado
   
   while (!MQTT.connected()) 
    {
        display.clear();
        display.drawString(0, 0, "Connecting to Broker");
        display.drawString(0, 10, "MQTT...");
        display.display();
        delay(2000);
        if (MQTT.connect(GATEWAY_ID)) 
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
            display.drawString(0, 20, "attempt in 2s");
            display.display();
            delay(2000);
        }
    }
}

//void splitPacket(String packet){
//  std::vector<String> subStrings;
//  int j=0;
//  for(int i =0; i < packet.length(); i++){
//    if(packet.charAt(i) == ','){
//      subStrings.push_back(packet.substring(j,i));
//      j = i+1;
//    }
//  }
//  subStrings.push_back(packet.substring(j,packet.length())); //to grab the last value of the string
//  acelX = subStrings[0];
//  acelY = subStrings[1];
//  acelZ = subStrings[2];
//}

//void loraData(){
//  display.clear();
//  digitalWrite(2, LOW);
//  display.setTextAlignment(TEXT_ALIGN_LEFT);
//  display.drawString(0 , 0 , "AcelX: "+ acelX + " m/s^2");
//  display.drawString(0, 10 , "AcelY: "+ acelY + " m/s^2");
//  display.drawString(0, 20 , "AcelZ: "+ acelZ + " m/s^2");
//  display.drawString(0, 40, rssi);  
//  display.display();
//}
