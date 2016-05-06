// Coding By IOXhop : www.ioxhop.com

// File : RestoreButton.ino
// Note :
//   1.Upload to ESP8266
//   2.Connect to WiFi name ESP_easyConfig
//   3.go to http://192.168.4.1/config
//   4.Login Username: admin and Password: password
//   5.Edit config and restart
//   6.Press and hold the restore button for 5 seconds
//   7.Config restore to default config

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <easyConfig.h>

// Config Switch pin (GPIO0 is flash button)
#define SW_RESTROE 0

ESP8266WebServer server(80);
easyConfig conf(server);

void setup() {
  // void easyConfig::restoreButton(int pin[, bool pullUp=true]);
  // pullUp=true  is external pullup
  // pullUp=false is external pulldown
  conf.restoreButton(SW_RESTROE, true);
  conf.begin(true); // void easyConfig::begin(bool StratServer);
}

void loop() {
  conf.run();
}