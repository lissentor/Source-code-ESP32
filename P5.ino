#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <iostream>
#include <string>
#include "DHT.h"
//here we use 14 of ESP32 to read data
#define DHTPIN 13
//our sensor is DHT11 type
#define DHTTYPE DHT11
//create an instance of DHT sensor
DHT dht(DHTPIN, DHTTYPE);

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
int humidity;
int temperature;

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define DHTDATA_CHAR_UUID "6E400003-B5A3-F393-E0A9-E50E24DCCA9E" 


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();
      Serial.println(rxValue[0]);

      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");

        for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]);
        }
        Serial.println();
        Serial.println("*********");
      }
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  Serial.println("DHT11 sensor!");
  //call begin to start sensor
  dht.begin();

  BLEDevice::init("AhmadFaiz");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Cria o servico UART
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Cria uma Característica BLE para envio dos dados
  pCharacteristic = pService->createCharacteristic(
                      DHTDATA_CHAR_UUID,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
                      
  pCharacteristic->addDescriptor(new BLE2902());

  // cria uma característica BLE para recebimento dos dados
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());

  // Inicia o serviço
  pService->start();

  // Inicia a descoberta do ESP32
  pServer->getAdvertising()->start();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void loop() {
  // Read temperature as Celsius (the default)
  if (deviceConnected) {
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();
  // Check if any reads failed and exit early (to try again).
  //if (isnan(humidity)) {
  //Serial.println("Failed to read from DHT sensor!");
  //return;
  //}
  // print the result to Terminal
  char humidityString[12];
  char temperatureString[10];
  dtostrf(humidity, 1, 2, humidityString);
  dtostrf(temperature, 1, 2, temperatureString);
  pCharacteristic->setValue(humidityString);
  pCharacteristic->setValue(temperatureString);
  pCharacteristic->notify();

  char dhtDataString[16];
  sprintf(dhtDataString, "%d,%d", temperature, humidity);
  
  //Serial.print("humidity: ");
  Serial.println(dhtDataString);
  //Serial.print(" %   ");
  //Serial.print("Temperature: ");
  //Serial.print(temperatureString);
  //Serial.println(" *C ");
  //we delay a little bit for next read
  delay(2000);
  }
}
