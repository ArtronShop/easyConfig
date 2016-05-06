// Coding By IOXhop : www.ioxhop.com

// File : Basic.ino
// Note :
//   1.Upload to ESP8266
//   2.Connect to WiFi name ESP_easyConfig 
//   3.go to http://192.168.4.1/config
//   4.Login Username: admin and Password: password

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <easyConfig.h>

ESP8266WebServer server(80);
easyConfig conf(server);

void setup() {
  conf.begin(true); // void easyConfig::begin(bool StratServer);
}

void loop() {
  conf.run();
}