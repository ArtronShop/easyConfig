// Coding By IOXhop : www.ioxhop.com
// This version 1.1

#include "Arduino.h"
#include "ESP8266WiFi.h"
#include "WiFiClient.h"
#include "ESP8266WebServer.h"
#include "ArduinoJson.h"
#include "FS.h"

// Debug
#define LED_DEBUG 2
#define LED_DEBUG_HIGH LOW
#define LED_DEBUG_LOW HIGH

// #define DEBUG_CONFIG
#define OUTPUT_DEBUG Serial

class ESP8266WebServer;

static struct {
	bool connected = false;
} _eConf;

class easyConfig {
	public:
		easyConfig(ESP8266WebServer &useServer) ;
		
		void setValue(String name, String val) ;
		void begin(bool runWebServer) ;
		void setMode(WiFiMode mode) ;
		bool isConnected() ;
		void run() ;
		void restore(bool reboot) ;
		void restoreButton(int pin, bool activeHigh=true) ;
		
		
	private:
		WiFiMode _mode = WIFI_AP_STA;
		bool _connected = false;
		unsigned long _blink_debug_led = 0;
		
		// Restore Button
		bool _restore_btn = false;
		int _restore_btn_pin = 0;
		bool _restore_active = true;
		bool _restore_btn_enter = false;
		unsigned long _restore_btn_enter_start = 0;
		
		// Connect to AP
		unsigned long _next_connect = 0;
		
		char ssid[20];
		char password[20];
		char name[20];
		char AuthUsername[20];
		char AuthPassword[20];
		
		void loadConfig() ;
		void saveConfig() ;
		void wifiConnect() ;

		ESP8266WebServer *_server;
		
String configPageHTML = 
"<!DOCTYPE HTML>\n"
"<html>\n"
"<head>\n"
"<meta charset=\"utf-8\">\n"
"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
"<title>easyConfig</title>\n"
"<style>\n"
"* {box-sizing: border-box}\n"
"body {font-family: Arial, Helvetica, sans-serif;margin: 0;background: #F5F5F5;margin-top: 20px;color: #333}\n"
"fieldset {border: none;margin:0;margin-bottom: 10px;padding: 0}\n"
"fieldset > div {padding: 5px;margin-bottom: 10px}\n"
"fieldset > div > label {width: 140px;display: inline-block}\n"
"legend {font-size: 18px;width: 100%;padding: 0;background: #ECECEC;color: #BBB;padding: 5px;padding-bottom: 3px;margin-bottom: 10px}\n"
".main-box {max-width: 768px;background: #FFF;margin: auto;box-shadow: 0 0 5px rgba(0, 0, 0, 0.1);padding: 10px}\n"
"h1 {margin: 0;margin: -10px -10px 10px -10px;background: #2196F3;padding: 20px 15px;color: #FFF}\n"
"input {border: none;border-bottom: 2px solid #EFEFEF;padding: 4px;outline: none;font-size: 16px;transition: border-bottom-color 300mS}\n"
"input:focus {border-bottom-color: #2196F3}\n"
"button[type='submit'],button[type='reset'],.btn {width: 50%;color: #FFF;border: none;outline: none;font-size: 16px;padding: 10px}\n"
"button[type='submit'],.btn-restart {background: #2196F3}\n"
"button[type='submit']:hover,.btn-restart:hover {background: #1976D2}\n"
"button[type='reset'],.btn-restore {background: #CCC}\n"
"button[type='reset']:hover,.btn-restore:hover {background: #999}\n"
".btn{width:100%;display:inline-block;text-decoration:none;text-align: center}\n"
"@media screen and (max-width: 768px) {body {margin: 0}}\n"
"@media screen and (max-width: 480px) {\n"
"\th1 {margin-bottom: 0}\n"
"\tfieldset > div > label {display: block;width: 100%;margin-bottom: 10px}\n"
"\tfieldset > div > input {width: 100%;margin-bottom: 10px}\n"
"\tform {margin: 0 -10px}\n"
"\tlegend {padding: 5px 15px 3px 15px}\n"
"\tfieldset > div {padding: 5px 15px}\n"
"\t.main-box {padding-bottom: 0}\n"
"}\n"
"</style>\n"
"</head>\n"
"\n"
"<body>\n"
"<div class=\"main-box\">\n"
"  <h1>easyConfig</h1>\n"
"  <form action=\"\" method=\"post\">\n"
"    <fieldset>\n"
"      <legend>Device Settings</legend>\n"
"      <div>\n"
"        <label for=\"name\">Device Name</label>\n"
"        <input type=\"text\" id=\"name\" name=\"name\" value=\"{name}\" maxlength=\"20\" required>\n"
"      </div>\n"
"    </fieldset>\n"
"    <fieldset>\n"
"      <legend>WiFi Connect</legend>\n"
"      <div>\n"
"        <label for=\"ssid\">WiFi Name</label>\n"
"        <input type=\"text\" id=\"ssid\" name=\"ssid\" value=\"{ssid}\" maxlength=\"20\">\n"
"      </div>\n"
"      <div>\n"
"        <label for=\"password\">WiFi Password</label>\n"
"        <input type=\"text\" id=\"password\" name=\"password\" value=\"{password}\" maxlength=\"20\">\n"
"      </div>\n"
"    </fieldset>\n"
"    <fieldset>\n"
"      <legend>Auth Login</legend>\n"
"      <div>\n"
"        <label for=\"auth-username\">Username</label>\n"
"        <input type=\"text\" id=\"auth-username\" name=\"auth-username\" value=\"{auth-username}\" maxlength=\"20\" required>\n"
"      </div>\n"
"      <div>\n"
"        <label for=\"auth-password\">Password</label>\n"
"        <input type=\"text\" id=\"auth-password\" name=\"auth-password\" value=\"{auth-password}\" maxlength=\"20\" required>\n"
"      </div>\n"
"    </fieldset>\n"
"    <fieldset>\n"
"      <legend>Systems management</legend>\n"
"      <div>\n"
"        <a href=\"./config/restart\" class=\"btn btn-restart\">Restart device</a>\n"
"      </div>\n"
"      <div>\n"
"        <a href=\"./config/restore\" class=\"btn btn-restore\" onClick=\"return confirm('Restore all config to default Yes or No ?');\">Restore config to Default</a>\n"
"      </div>\n"
"    </fieldset>\n"
"    <button type=\"submit\">Save</button><button type=\"reset\">Reset</button>\n"
"  </form>\n"
"</div>\n"
"</body>\n"
"</html>\n"
""
;
		
}
;
