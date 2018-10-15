#include <Arduino.h>
#include <Adafruit_BMP280.h>
#include <math.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>
#include "cactus_io_AM2302.h"

#define AM2302_PIN 2
#define BMP_SCK 13
#define BMP_MISO 10
#define BMP_MOSI 12
#define BMP_CS 11

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0);
AM2302 dht(AM2302_PIN);
Adafruit_BMP280 bme(BMP_CS, BMP_MOSI, BMP_MISO,  BMP_SCK);

void setup() {
  Serial.begin(9600);
  u8g2.begin();
  dht.begin();
}

void loop() {
  updateHumidity();
  delay(2000);
  updateTemperature();
  delay(2000);
  updatePressure();
  delay(2000);
}

void updateHumidity() {
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
}

void updateTemperature() {
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
}

void updatePressure() {
  if (!bme.begin()) {
    Serial.println("BME280 sensor read failure!");
    return;
  }

  float pressure = bme.readPressure() / 133.3; // Convert from Pa to mmHg

  char result[8];
  char str_pressure[6];
  dtostrf(pressure, 3, 1, str_pressure);
  sprintf(result, "%smm", str_pressure);

  printToMonitor(result);
}

void printToMonitor(String value) {
  char buffer[20];
  value.toCharArray(buffer, 20);

  u8g2.clearBuffer();          // clear the internal memory
  u8g2.setFont(u8g2_font_logisoso28_tr);  // choose a suitable font at https://github.com/olikraus/u8g2/wiki/fntlistall
  u8g2.drawStr(8, 29, buffer); // write something to the internal memory
  u8g2.sendBuffer();         // transfer internal memory to the display
}
