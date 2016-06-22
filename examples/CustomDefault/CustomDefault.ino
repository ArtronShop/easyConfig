// Coding By IOXhop : www.ioxhop.com

// File : CustomDefault.ino
// Note :
//   1.Upload to ESP8266
//   2.Connect to WiFi name MeCustomName
//   3.go to http://192.168.4.1/config
//   4.Login Password : 123456

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <easyConfig.h>

ESP8266WebServer server(80);
easyConfig conf(server);

void setup() {
  conf.setValue("name", "MeCustomName"); // Set default name max 20 characters
  // conf.setValue("AuthPassword", "987654"); // Set default authentication password max 20 characters
  conf.begin(true); // void easyConfig::begin(bool StratServer);
}

void loop() {
  conf.run();
}