#include <Arduino.h>
#include <Adafruit_BMP280.h>
#include <math.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>
#include <cactus_io_AM2302.h>
#include <DS3231.h>
#include <SD.h>
#include <SoftwareSerial.h>

#define AM2302_PIN 2
#define SD_PORT 53

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0);
AM2302 dht(AM2302_PIN);
Adafruit_BMP280 bme;
RTClib RTC;
SoftwareSerial co2Serial(10, 11);
const byte GET_C02_COMMAND[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};

void setup() {
  Serial.begin(9600);
  u8g2.begin();
  dht.begin();
  co2Serial.begin(9600);

  pinMode(SD_PORT, OUTPUT);
  if (!SD.begin(SD_PORT)) {
    Serial.println("Card failed, or not present");
  } else {
    Serial.println("Card initialized");
  }

  Serial.println("Setup finished");
}

void loop() {
  float humidity = updateHumidity();
  delay(2000);
  float temperature = updateTemperature();
  delay(2000);
  int pressure = updatePressure();
  delay(2000);
  int co2 = updateCO2();
  delay(2000);
  logToFile(humidity, temperature, pressure, co2);
}

float updateHumidity() {
  dht.readHumidity();
  if (isnan(dht.humidity)) {
    Serial.println("DHT sensor read failure!");
    return;
  }

  float humidity = dht.humidity;

  char result[8];
  char str_humidity[6];
  dtostrf(humidity, 3, 1, str_humidity);
  sprintf(result, "%s%%", str_humidity);

  printToMonitor(result);
  return humidity;
}

float updateTemperature() {
  dht.readTemperature();
  if (isnan(dht.temperature_C)) {
    Serial.println("DHT sensor read failure!");
    return;
  }

  float temperature = dht.temperature_C;

  char result[8];
  char str_temperature[5];
  dtostrf(temperature, 3, 1, str_temperature);
  sprintf(result, "%s*C", str_temperature);

  printToMonitor(result);
  return temperature;
}

int updatePressure() {
  if (!bme.begin()) {
    Serial.println("BME280 sensor read failure!");
    return;
  }

  int pressure = bme.readPressure() / 133.3; // Convert from Pa to mmHg

  char result[8];
  char str_pressure[6];
  sprintf(result, "%dmm", pressure);

  printToMonitor(result);
  return pressure;
}

int updateCO2() {
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
    return -1;
  } else {
    unsigned int responseHigh = (unsigned int) response[2];
    unsigned int responseLow = (unsigned int) response[3];
    int co2 = (256 * responseHigh) + responseLow;

    char result[8];
    sprintf(result, "%d CO2", co2);

    printToMonitor(result);
    return co2;
  }
}

void logToFile(float humidity, float temperature, int pressure, int co2) {

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

void printToMonitor(String value) {
  char buffer[20];
  value.toCharArray(buffer, 20);

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_logisoso28_tr);
  u8g2.drawStr(8, 31, buffer);
  u8g2.sendBuffer();
}
