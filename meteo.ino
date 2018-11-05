#include <Arduino.h>
#include <Adafruit_BMP280.h>
#include <math.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>
#include <cactus_io_AM2302.h>
#include <DS3231.h>

#define AM2302_PIN 2

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0);
AM2302 dht(AM2302_PIN);
Adafruit_BMP280 bme;
RTClib RTC;

void setup() {
  Serial.begin(9600);
  Serial.println("in progress..");
  u8g2.begin();
  dht.begin();
  Serial.println("started");
}

void loop() {
  updateHumidity();
  delay(2000);
  updateTemperature();
  delay(2000);
  updatePressure();
  delay(2000);

  DateTime now = RTC.now();
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(' ');
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
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

  int pressure = bme.readPressure() / 133.3; // Convert from Pa to mmHg

  char result[8];
  char str_pressure[6];
  sprintf(result, "%dmm", pressure);

  printToMonitor(result);
}

void printToMonitor(String value) {
  char buffer[20];
  value.toCharArray(buffer, 20);

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_logisoso28_tr);
  u8g2.drawStr(8, 31, buffer);
  u8g2.sendBuffer();
}
