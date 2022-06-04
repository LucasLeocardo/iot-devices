#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <LoRa.h> //responsável pela comunicação com o WIFI Lora
#include <SPI.h> //responsável pela comunicação serial
#include "ArduinoJson.h"


// Definição dos pinos 
#define SCK     5    // GPIO5  -- SX127x's SCK
#define MISO    19   // GPIO19 -- SX127x's MISO
#define MOSI    27   // GPIO27 -- SX127x's MOSI
#define SS      18   // GPIO18 -- SX127x's CS
#define RST     14   // GPIO14 -- SX127x's RESET
#define DI00    26   // GPIO26 -- SX127x's IRQ(Interrupt Request)

#define BAND    915E6  //Frequencia do radio - podemos utilizar ainda : 433E6, 868E6, 915E6
#define PABOOST true

Adafruit_MPU6050 mpu;

StaticJsonDocument<256> doc;
char out[256];
String deviceId = "628eccd5aa6849c399d00ee6";

void setup() {
  pinMode(2,OUTPUT);
  Serial.begin(9600);
  while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens

  SPI.begin(SCK,MISO,MOSI,SS); //inicia a comunicação serial com o Lora
  LoRa.setPins(SS,RST,DI00); //configura os pinos que serão utlizados pela biblioteca (deve ser chamado antes do LoRa.begin)
  
  //inicializa o Lora com a frequencia específica.
  if (!LoRa.begin(BAND))
  {
    Serial.print("Starting LoRa failed!");
    while (1);
  }

  Serial.print("LoRa Initial success!");
  Serial.println("Adafruit MPU6050 test!");

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
  case MPU6050_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case MPU6050_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case MPU6050_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case MPU6050_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }
  
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
  case MPU6050_RANGE_250_DEG:
    Serial.println("+- 250 deg/s");
    break;
  case MPU6050_RANGE_500_DEG:
    Serial.println("+- 500 deg/s");
    break;
  case MPU6050_RANGE_1000_DEG:
    Serial.println("+- 1000 deg/s");
    break;
  case MPU6050_RANGE_2000_DEG:
    Serial.println("+- 2000 deg/s");
    break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_260_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
  case MPU6050_BAND_260_HZ:
    Serial.println("260 Hz");
    break;
  case MPU6050_BAND_184_HZ:
    Serial.println("184 Hz");
    break;
  case MPU6050_BAND_94_HZ:
    Serial.println("94 Hz");
    break;
  case MPU6050_BAND_44_HZ:
    Serial.println("44 Hz");
    break;
  case MPU6050_BAND_21_HZ:
    Serial.println("21 Hz");
    break;
  case MPU6050_BAND_10_HZ:
    Serial.println("10 Hz");
    break;
  case MPU6050_BAND_5_HZ:
    Serial.println("5 Hz");
    break;
  }

  doc["deviceId"] = deviceId;
  
  Serial.println("");
  delay(100);
}

void loop() {

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  double aux1 = roundf(a.acceleration.x * 100);
  double acelx = aux1 / 100;

  double aux2 = roundf(a.acceleration.y * 100);
  double acely = aux2 / 100;

  double aux3 = roundf(a.acceleration.z * 100);
  double acelZ = aux3 / 100;

  double aux4 = roundf(g.gyro.x * 100);
  double alphaX = aux4 / 100;

  double aux5 = roundf(g.gyro.y * 100);
  double alphaY = aux5 / 100;

  double aux6 = roundf(g.gyro.z * 100);
  double alphaZ = aux6 / 100;
  
  //beginPacket : abre um pacote para adicionarmos os dados para envio
  doc["acelX"]  = acelx;
  doc["acelY"]  = acely;
  doc["acelZ"]  = acelZ;
  doc["alphaX"] = alphaX;
  doc["alphaY"] = alphaY;
  doc["alphaZ"] = alphaZ;
  
  LoRa.beginPacket();
  //print: adiciona os dados no pacote
  serializeJson(doc, out);
  LoRa.print(out);
  //endPacket : fecha o pacote e envia
  LoRa.endPacket(); //retorno= 1:sucesso | 0: falha

  digitalWrite(2, HIGH);   // liga o LED indicativo
  delay(500);                       // aguarda 500ms
  digitalWrite(2, LOW);    // desliga o LED indicativo
  delay(500);                       // aguarda 500msno contador

  Serial.print("Acceleration X: ");
  Serial.print(a.acceleration.x);
  Serial.print(", Y: ");
  Serial.print(a.acceleration.y);
  Serial.print(", Z: ");
  Serial.print(a.acceleration.z);
  Serial.println(" m/s^2");

  Serial.print("Rotation X: ");
  Serial.print(g.gyro.x);
  Serial.print(", Y: ");
  Serial.print(g.gyro.y);
  Serial.print(", Z: ");
  Serial.print(g.gyro.z);
  Serial.println(" rad/s");

//  Serial.print("Temperature: ");
//  Serial.print(temp.temperature);
//  Serial.println(" degC");

  Serial.println(" ");

  delay(5000); 
  
}
