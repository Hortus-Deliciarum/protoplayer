#include <ArduinoOSCWiFi.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

const char* ssid = "FASTWEB-DAB6F7";
const char* password = "1NJYRZE2T4";
const int recv_port = 54321;

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void onOscReceived(const OscMessage& m) {
    Serial.print(m.remoteIP());
    Serial.print(" ");
    Serial.print(m.remotePort());
    Serial.print(" ");
    Serial.print(m.size());
    Serial.print(" ");
    Serial.print(m.address());
    Serial.print(" ");
    Serial.println(m.arg<float>(0));
}

void setup() {
  Serial.begin(115200);
  initWiFi();
  OscWiFi.subscribe(recv_port, "/callback", onOscReceived);
}

void loop() {
  OscWiFi.parse();
}
