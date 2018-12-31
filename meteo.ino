#include <Arduino.h>
#include <Adafruit_BMP280.h>
#include <math.h>
#include "Ucglib.h"
#include <SPI.h>
#include <Wire.h>
#include <cactus_io_AM2302.h>
#include <DS3231.h>
#include <SD.h>
#include <SoftwareSerial.h>

#define AM2302_PIN 2
#define SD_PORT 53

Ucglib_ST7735_18x128x160_SWSPI ucg(/*sclk=*/ 7, /*data=*/ 6, /*cd=*/ 5, /*cs=*/ 3, /*reset=*/ 4);
AM2302 dht(AM2302_PIN);
Adafruit_BMP280 bme;
RTClib RTC;
SoftwareSerial co2Serial(10, 11);
const byte GET_C02_COMMAND[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};

float humidity;
float temperature;
int pressure;
int co2;

void setup() {
  Serial.begin(9600);
  Serial1.begin(115200); // ESP-01 HardWare Serial
  dht.begin();
  co2Serial.begin(9600);

  pinMode(SD_PORT, OUTPUT);
  if (!SD.begin(SD_PORT)) {
    Serial.println("Card failed, or not present");
  } else {
    Serial.println("Card initialized");
  }

  ucg.begin(UCG_FONT_MODE_SOLID);
  ucg.clearScreen();
  ucg.setRotate90();
  ucg.setFont(ucg_font_inr24_mr);
  ucg.setColor(1, 0, 0, 0);  // background (1,R,G,B)

  Serial.println("Setup finished");
}

void loop() {
  updateHumidity();
  updateTemperature();
  updatePressure();
  updateCO2();
  logToFile();
  sendToEsp();
  updateDisplay();
  delay(8000);
}

void updateHumidity() {
  dht.readHumidity();
  if (isnan(dht.humidity)) {
    Serial.println("DHT sensor read failure!");
    return;
  }

  humidity = dht.humidity;
}

void updateTemperature() {
  dht.readTemperature();
  if (isnan(dht.temperature_C)) {
    Serial.println("DHT sensor read failure!");
    return;
  }

  temperature = dht.temperature_C;
}

void updatePressure() {
  if (!bme.begin()) {
    Serial.println("BME280 sensor read failure!");
    return;
  }

  pressure = bme.readPressure() / 133.3; // Convert from Pa to mmHg
}

void updateCO2() {
  co2Serial.write(GET_C02_COMMAND, 9);
  unsigned char response[9];
  memset(response, 0, 9);
  co2Serial.readBytes(response, 9);

  byte crc = 0;
  for (int i = 1; i < 8; i++) {
    crc += response[i];
  }
  crc = 255 - crc;
  crc++;

  if ( !(response[0] == 0xFF && response[1] == 0x86 && response[8] == crc) ) {
    Serial.println("MH-Z19B CRC error: " + String(crc) + " / " + String(response[8]));
  } else {
    unsigned int responseHigh = (unsigned int) response[2];
    unsigned int responseLow = (unsigned int) response[3];
    co2 = (256 * responseHigh) + responseLow;
  }
}

void logToFile() {
  long timestamp = RTC.now().unixtime();

  String logString = "";
  logString += String(timestamp);
  logString += ",";
  logString += String(humidity);
  logString += ",";
  logString += String(temperature);
  logString += ",";
  logString += String(pressure);
  logString += ",";
  logString += String(co2);

  File logFile = SD.open("meteo.txt", FILE_WRITE);
  if (logFile) {
    logFile.println(logString);
    logFile.close();
    Serial.println(logString); // TODO!
  } else {
    Serial.println("error opening file");
  }
}

void sendToEsp() {
  String dataString = "humidity value=";
  dataString += String(humidity);
  dataString += "\ntemperature value=";
  dataString += String(temperature);
  dataString += "\npressure value=";
  dataString += String(pressure);
  dataString += "\nco2 value=";
  dataString += String(co2);
  dataString += ";";
  Serial1.print(dataString);
}

void updateDisplay() {
  char humidity_result[8];
  char str_humidity[6];
  dtostrf(humidity, 3, 1, str_humidity);
  sprintf(humidity_result, "%s%%", str_humidity);

  ucg.setColor(0, 0, 102, 255);
  ucg.setPrintPos(0, 32);
  ucg.print(humidity_result);

  char temperature_result[8];
  char str_temperature[5];
  dtostrf(temperature, 3, 1, str_temperature);
  sprintf(temperature_result, "%sC", str_temperature);

  ucg.setColor(0, 51, 204, 51);
  ucg.setPrintPos(0, 64);
  ucg.print(temperature_result);

  char pressure_result[8];
  char str_pressure[6];
  sprintf(pressure_result, "%dmm", pressure);

  ucg.setColor(0, 255, 204, 0);
  ucg.setPrintPos(0, 96);
  ucg.print(pressure_result);

  char co2_result[8];
  sprintf(co2_result, "%d CO2", co2);

  ucg.setColor(0, 255, 51, 0);
  ucg.setPrintPos(0, 128);
  ucg.print(co2_result);
}
