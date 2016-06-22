// Coding By IOXhop : www.ioxhop.com

// File : Basic.ino
// Note :
//   1.Upload to ESP8266
//   2.Connect to WiFi name ESP_easyConfig 
//   3.go to http://192.168.4.1/config
//   4.Login Password : 123456

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <easyConfig.h>

ESP8266WebServer server(80);
easyConfig conf(server);

void setup() {
  Serial.begin(115200);
  conf.cloudConfig("netpie"); // Support NETPIE , MQTT (V1.2)
  
  conf.begin(true); // void easyConfig::begin(bool StratServer);

  // .:: After read config ::.
  
  // Read cloud config
  String APPID = conf.configValue("APPID"); // Read APPID for netpie config
  String KEY = conf.configValue("KEY"); // Read KEY for netpie config
  String SECRET = conf.configValue("SECRET"); // Read SECRET for netpie config
  String ALIAS = conf.configValue("ALIAS"); // Read ALIAS for netpie config

  Serial.println("Cloud config");
  Serial.println("APPID  -> " + APPID);
  Serial.println("KEY    -> " + KEY);
  Serial.println("SECRET -> " + SECRET);
  Serial.println("ALIAS  -> " + ALIAS);
}

void loop() {
  conf.run();
}