// Coding By IOXhop : www.ioxhop.com
// This version 1.2

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

//#define DEBUG_CONFIG
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
		
		// Add on V1.2
		void cloudConfig(String name) ;
		String configValue(String name) ;
		
		
	private:
		WiFiMode _mode = WIFI_AP_STA;
		bool _connected = false;
		unsigned long _blink_debug_led = 0;
		String _RootURL = "/config";
		
		// Restore Button
		bool _restore_btn = false;
		int _restore_btn_pin = 0;
		bool _restore_active = true;
		bool _restore_btn_enter = false;
		unsigned long _restore_btn_enter_start = 0;
		
		// Add on V1.2
		String _CustomConfig[20][2];
		byte _CustomConfigIndex = 0;
		
		// Connect to AP
		unsigned long _next_connect = 0;
		
		char ssid[20];
		char password[20];
		char name[20];
		char AuthPassword[7];
		
		void loadConfig() ;
		void saveConfig() ;
		void wifiConnect() ;
		// Add on V1.2
		void addCustomConfig(String name) ;

		ESP8266WebServer *_server;

String templateHTML = 
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
".main-box {max-width: 768px;background: #FFF;margin: auto;box-shadow: 0 0 5px rgba(0, 0, 0, 0.1);padding: 10px;margin-bottom:20px}\n"
"h1 {margin: 0;margin: -10px -10px 10px -10px;background: #2196F3;padding: 20px 15px;color: #FFF}\n"
"input {border: none;border-bottom: 2px solid #EFEFEF;padding: 4px;outline: none;font-size: 16px;transition: border-bottom-color 300mS}\n"
"input:focus {border-bottom-color: #2196F3}\n"
"button[type='submit'],button[type='reset'],.btn {width: 50%;color: #FFF !important;border: none;outline: none;font-size: 16px;padding: 10px}\n"
"button[type='submit'],.btn-restart,.btn-p {background: #2196F3}\n"
"button[type='submit']:hover,.btn-restart:hover,.btn-p:hover {background: #1976D2}\n"
"button[type='reset'],.btn-restore {background: #CCC}\n"
"button[type='reset']:hover,.btn-restore:hover {background: #999}\n"
".btn{width:100%;display:inline-block;text-decoration:none;text-align: center}\n"
"@media screen and (max-width: 768px) {body {margin: 0;background:#FFF;}.main-box{ box-shadow:none;}}\n"
"@media screen and (max-width: 480px) {\n"
"\th1 {margin-bottom: 0}\n"
"\tfieldset > div > label {display: block;width: 100%;margin-bottom: 10px}\n"
"\tfieldset > div > input {width: 100%;margin-bottom: 10px}\n"
"\tform {margin: 0 -10px}\n"
"\tlegend {padding: 5px 15px 3px 15px}\n"
"\tfieldset > div {padding: 5px 15px}\n"
"\t.main-box {padding-bottom: 0;}\n"
"}\n"
"h2{text-align: center;color: #333333;}\n"
".box-auth-pass {text-align: center;padding: 20px 0;}\n"
".box-auth-pass > span {padding: 0 4px;border-bottom: 2px solid #ECECEC;margin: 0 5px;display: inline-block;font-size: 28px;color: #666}\n"
".box-keypad {text-align: center}\n"
".box-keypad > div {margin-bottom: 10px}\n"
".box-keypad button{display: inline-block;width: auto;font-size: 22px;margin: 0 5px}\n"
"a{color:#2196F3}\n"
"a:hover{color:#1565C0}\n"
"</style>\n"
"</head>\n"
"\n"
"<body>\n"
"<div class=\"main-box\">\n"
"  <h1>easyConfig</h1>\n"
"{INCODEHTML}"
"</div>\n"
"</body>\n"
"</html>\n"
;

String configPageHTML = 
"  <form action=\"\" method=\"post\">\n"
"    <fieldset>\n"
"      <legend>Device Settings</legend>\n"
"      <div>\n"
"        <label for=\"name\">Device Name</label>\n"
"        <input type=\"text\" id=\"name\" name=\"name\" value=\"{name}\" maxlength=\"20\" required>\n"
"      </div>\n"
"    </fieldset>\n"
"{CustomSet}\n"
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
"        <label for=\"auth-password\">Password</label>\n"
"        <input type=\"text\" id=\"auth-password\" name=\"auth-password\" value=\"{auth-password}\" pattern=\"[0-9]{6}\" title=\"Enter config auth password (Number 6 char)\" required>\n"
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
;

String authHTML = 
"  <h2>Enter auth password (6 Digits)</h2>\n"
"  <div class=\"box-auth-pass\">\n"
"    <span id=\"digi-1\">&nbsp;</span><span id=\"digi-2\">&nbsp;</span><span id=\"digi-3\">&nbsp;</span><span id=\"digi-4\">&nbsp;</span><span id=\"digi-5\">&nbsp;</span><span id=\"digi-6\">&nbsp;</span>\n"
"  </div>\n"
"  <div class=\"box-keypad\">\n"
"    <div><button class=\"btn btn-p\">7</button><button class=\"btn btn-p\">8</button><button class=\"btn btn-p\">9</button></div>\n"
"    <div><button class=\"btn btn-p\">4</button><button class=\"btn btn-p\">5</button><button class=\"btn btn-p\">6</button></div>\n"
"    <div><button class=\"btn btn-p\">1</button><button class=\"btn btn-p\">2</button><button class=\"btn btn-p\">3</button></div>\n"
"    <div><button class=\"btn btn-p\">?</button><button class=\"btn btn-p\">0</button><button class=\"btn btn-p\">C</button></div>\n"
"  </div>\n"
"<script>\n"
"$ = function (query) {return document.querySelectorAll(query)};\n"
"var sels = $(\".box-keypad button\");\n"
"for(i=0; i<sels.length; i++) {\n"
"\tsels[i].addEventListener(\"click\", function() {\n"
"\t\tvar number = this.innerText;\n"
"\t\tif (number == '?') return;\n"
"\t\tif (number == 'C') {\n"
"\t\t\tfor (var iEle=1;iEle<=6;iEle++) $(\"#digi-\" + iEle)[0].innerHTML = \"&nbsp;\";\n"
"\t\t\tdigitIndex=1;\n"
"\t\t\treturn;\n"
"\t\t}\n"
"\t\t\n"
"\t\t$(\"#digi-\" + digitIndex)[0].innerText = number;\n"
"\t\tif (digitIndex == 6) {\n"
"\t\t\tvar auth = \"\";\n"
"\t\t\tfor (var iEle=1;iEle<=6;iEle++) auth += $(\"#digi-\" + iEle)[0].innerHTML;\n"
"\t\t\tdocument.cookie = \"auth=\" + auth + \";\";\n"
"\t\t\tlocation.reload();\n"
"\t\t}\n"
"\t\tdigitIndex++\n"
"\t});\n"
"}\n"
"var digitIndex = 1;\n"
"</script>\n"
;

String showConfigHTML = 
"  <h2>{TEXTSHOW}</h2>\n"
"  <div style=\"text-align:center;\"><canvas id=\"myCanvas\" width=\"200\" height=\"100\"></canvas>\n"
"  <div style=\"text-align:center;\"><a href=\"javascript:location.href='{ROOT}';\">Back to config</a></div>\n"
"  <script>\n"
"\tvar c = document.querySelector(\"#myCanvas\");\n"
"\tvar ctx = c.getContext(\"2d\");\n"
"\tctx.beginPath();\n"
"\tctx.arc(30,50,20,0,2*Math.PI);\n"
"\tctx.fillStyle = \"#2196F3\";\n"
"\tctx.fill();\n"
"\n"
"\tctx.beginPath();\n"
"\tctx.moveTo(30,50);\n"
"\tctx.lineTo(52,60);\n"
"\tctx.lineTo(52,40);\n"
"\tctx.fillStyle = \"#FFF\";\n"
"\tctx.fill();\n"
"\n"
"\tfor (var i=0;i<3;i++) {\n"
"\t\tctx.beginPath();\n"
"\t\tctx.arc(80 + (i * (30 + 8)),50,8,0,2 * Math.PI);\n"
"\t\tctx.fillStyle = \"#64B5F6\";\n"
"\t\tctx.fill();\n"
"\t}\n"
"  </script>"
;

}
;
