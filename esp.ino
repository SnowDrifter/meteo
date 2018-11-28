#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

const char INFLUXDB_URL[] = "http://username:password@host:port/write?db=db_name";
const char WIFI_NAME[] = "";
const char WIFI_PASSWORD[] = "";

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_NAME, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  sendData("start value=1");
}

void loop() {
  if (Serial.available() > 0) {
    String data = Serial.readStringUntil(';');
    sendData(data);
  }
}

void sendData(String data) {
  HTTPClient http;
  http.begin(INFLUXDB_URL);
  http.POST(data);
  http.end();
}
