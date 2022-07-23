// Coding By IOXhop : www.ioxhop.com
// This version 1.3

#include "Arduino.h"
#ifdef ESP32
#include "WiFi.h"
#include "WebServer.h"
#else
#include "ESP8266WiFi.h"
#include "ESP8266WebServer.h"
#endif
#include "WiFiClient.h"
#include "ArduinoJson-v6.19.4.h"
#include "SPIFFS.h"
#include "easyConfig.h"


#ifdef ESP32
easyConfig::easyConfig(WebServer &useServer): _server(&useServer) {
#else
easyConfig::easyConfig(ESP8266WebServer &useServer): _server(&useServer) {
#endif
	ssid[0] = 0;
	password[0] = 0;
	sprintf(name, "ESP_easyConfig");
	sprintf(AuthPassword, "123456");
}

void easyConfig::setValue(String key, String val) {
	if (key == "ssid") val.toCharArray(ssid, 20);
	else if (key == "password") val.toCharArray(password, 20);
	else if (key == "name") val.toCharArray(name, 20);
	else if (key == "auth-password") val.toCharArray(AuthPassword, 6);
	else {
		// Custom Config
		for (int index=0;index<_CustomConfigIndex;index++) {
			if (_CustomConfig[index][0].length() > 0 && _CustomConfig[index][0].charAt(0) != ':') {
				if (_CustomConfig[index][0] == key) _CustomConfig[index][1] = val;
			}
		}
	}
}

void easyConfig::begin(bool runWebServer) {
#ifdef DEBUG_CONFIG
	OUTPUT_DEBUG.println("[easyConfig] Set pin " + String(LED_DEBUG) + " to output");
#endif
	pinMode(LED_DEBUG, OUTPUT);
	digitalWrite(LED_DEBUG, LED_DEBUG_LOW);

#ifdef DEBUG_CONFIG
	OUTPUT_DEBUG.println("[easyConfig] Begin SPIFFS");
#endif
	if (!SPIFFS.begin()) {
#ifdef DEBUG_CONFIG
		OUTPUT_DEBUG.println("[easyConfig] Failed to mount file system");
#endif
		delay(5000);
		ESP.restart();
		return;
	}
	
#ifdef DEBUG_CONFIG
	OUTPUT_DEBUG.println("[easyConfig] call function easyConfig::loadConfig()");
#endif
	loadConfig();
#ifdef DEBUG_CONFIG
	OUTPUT_DEBUG.println("[easyConfig] set WiFi mode to " + String(_mode));
#endif
	WiFi.mode(_mode);
	WiFi.setAutoConnect(true);
	WiFi.setAutoReconnect(true);
#ifdef DEBUG_CONFIG
	OUTPUT_DEBUG.println("[easyConfig] set Soft AP SSID to " + String(name));
#endif
	WiFi.softAP(name);
	wifiConnect();
	
	const char * headerkeys[] = {"Cookie"} ;
	size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);

	_server->collectHeaders(headerkeys, headerkeyssize );
	
	
	_server->on(_RootURL.c_str(), [&]() {
		String tmpConfigPage = templateHTML;
		if (!isLogin()){
#ifdef DEBUG_CONFIG
			int count  = _server->headers();
			for (int CountA=0;CountA<count;CountA++) {
				OUTPUT_DEBUG.println("[easyConfig] Header '" + _server->headerName(CountA) + "' -> '" + _server->header(CountA) + "'");
			}
			OUTPUT_DEBUG.println("[easyConfig] hasHeader 'Cookie' is " + (String)(_server->hasHeader("Cookie") ? "True" : "False"));
			OUTPUT_DEBUG.println("[easyConfig] Cookie = '" + _server->header("Cookie") + "'");
#endif
			tmpConfigPage.replace("{INCODEHTML}", authHTML);
			_server->send(200, "text/html", tmpConfigPage);
			return;
		}
		if (_server->method() == HTTP_GET) {
			tmpConfigPage.replace("{INCODEHTML}", configPageHTML);
			tmpConfigPage.replace("{ssid}", String(ssid));
			tmpConfigPage.replace("{password}", String(password));
			tmpConfigPage.replace("{name}", String(name));
			tmpConfigPage.replace("{auth-password}", String(AuthPassword));
			
			// Custom Config
			String htmlCustomConfig = "";

			for (int index=0;index<_CustomConfigIndex;index++) {
				if (_CustomConfig[index][0].length() > 0) {
					if (_CustomConfig[index][0].charAt(0) == ':') {
						htmlCustomConfig += "<fieldset>";
						htmlCustomConfig += "<legend>" + _CustomConfig[index][0].substring(1) + "</legend>";
						continue;
					}
					htmlCustomConfig += "<div>";
					htmlCustomConfig += "<label for=\"custom-" + _CustomConfig[index][0] + "\">" + _CustomConfig[index][0] + "</label>";
					htmlCustomConfig += "<input type=\"text\" id=\"custom-" + _CustomConfig[index][0] + "\" name=\"custom-" + String(index) + "\" value=\"" + _CustomConfig[index][1] + "\">";
					htmlCustomConfig += "</div>";
					if (index+1 != _CustomConfigIndex) {
						if (_CustomConfig[index+1][0].charAt(0) == ':') {
							htmlCustomConfig += "</fieldset>";
						}
					} else {
						htmlCustomConfig += "</fieldset>";
					}
				}
			}
			tmpConfigPage.replace("{CustomSet}", htmlCustomConfig);
			tmpConfigPage.replace("{ROOT}", _RootURL);
			tmpConfigPage.replace("{STATUS}", WiFi.isConnected() ? "Connected : " + WiFi.localIP().toString() : "Disconnect");
			_server->send(200, "text/html", tmpConfigPage);
		} else if (_server->method() == HTTP_POST) {
			String tmpSSID, tmpPassword, tmpName, tmpAuthUsername, tmpAuthPassword;
			for (uint8_t i=0; i<_server->args(); i++){
				if (_server->argName(i) == "ssid") tmpSSID = _server->arg(i)=="" ? "NULL" : _server->arg(i);
				if (_server->argName(i) == "password") tmpPassword = _server->arg(i)=="" ? "NULL" : _server->arg(i);
				if (_server->argName(i) == "name") tmpName = _server->arg(i);
				if (_server->argName(i) == "auth-password") tmpAuthPassword = _server->arg(i);
				
				if (_server->argName(i).indexOf("custom-") >= 0) {
					// Custom Config
					for (int index=0;index<_CustomConfigIndex;index++) {
						if (_CustomConfig[index][0].length() > 0 && _CustomConfig[index][0].charAt(0) != ':') {
							if (("custom-" + String(index)) == _server->argName(i)) {
								_CustomConfig[index][1] = _server->arg(i);
#ifdef DEBUG_CONFIG
								OUTPUT_DEBUG.println("[easyConfig] _CustomConfig[" + String(index) + "][1] -> '" + _server->arg(i) + "'");
#endif
							}
						}
					}
				}
#ifdef DEBUG_CONFIG
				OUTPUT_DEBUG.println("[easyConfig] POST '" + _server->argName(i) + "' -> '" + _server->arg(i) + "'");
#endif
			}
			tmpSSID.toCharArray(ssid, 20);
			tmpPassword.toCharArray(password, 20);
			tmpName.toCharArray(name, 20);
			tmpAuthPassword.toCharArray(AuthPassword, 7);
			saveConfig();
			tmpConfigPage.replace("{INCODEHTML}", showConfigHTML);
			tmpConfigPage.replace("{ROOT}", _RootURL);
			tmpConfigPage.replace("{TEXTSHOW}", "Save &amp; reboot, Please wait 30s- 2 min and reconnect wifi.");
			_server->send(200, "text/html", tmpConfigPage);
			ESP.restart();
		}
	});
	
	_server->on((_RootURL + "/restart").c_str(), [&]() {
		if (!isLogin()){
			_server->send(200, "text/html", "<script>location.href='" + _RootURL + "'</script>");
			return;
		}
		String tmpConfigPage = templateHTML;
		tmpConfigPage.replace("{INCODEHTML}", showConfigHTML);
		tmpConfigPage.replace("{ROOT}", _RootURL);
		tmpConfigPage.replace("{TEXTSHOW}", "Restart now, Please wait 30 Sec - 2 Min.");
		_server->send(200, "text/html", tmpConfigPage);
		ESP.restart();
	});
	
	_server->on((_RootURL + "/restore").c_str(), [&]() {
		if (!isLogin()){
			_server->send(200, "text/html", "<script>location.href='" + _RootURL + "'</script>");
			return;
		}
		String tmpConfigPage = templateHTML;
		tmpConfigPage.replace("{INCODEHTML}", showConfigHTML);
		tmpConfigPage.replace("{ROOT}", _RootURL);
		tmpConfigPage.replace("{TEXTSHOW}", "Restore and reboot, Please wait 30 Sec - 2 Min.");
		_server->send(200, "text/html", tmpConfigPage);
		restore(true);
	});
	
	_server->on((_RootURL + "/scan").c_str(), [&]() {
		if (!isLogin()){
			_server->send(200, "application/json", "[]");
			return;
		}
		String textScan = "[";
		int n = WiFi.scanNetworks();
		for (int i = 0;i<n; i++) {
			textScan += "[\"" + WiFi.SSID(i) + "\", " + String(WiFi.RSSI(i)) + ", " + (WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "false" : "true") + "]";
			textScan += (i+1 < n) ? "," : "";
		}
		textScan += "]";
		_server->send(200, "application/json", textScan);
	});
	
	if (runWebServer) {
		_server->begin();
	}
}

#ifdef ESP32
void easyConfig::setMode(WiFiMode_t mode) {
#else
void easyConfig::setMode(WiFiMode mode) {
#endif
	_mode = mode;
}

bool easyConfig::isConnected() {
	return WiFi.isConnected();
}

void easyConfig::restore(bool reboot) {
	if (SPIFFS.exists("/config.json")) {
		SPIFFS.remove("/config.json");
	}
	if (reboot) {
		ESP.restart();
	}
}

void easyConfig::restoreButton(int pin, bool activeHigh) {
	pinMode(pin, INPUT);
	_restore_btn = true;
	_restore_btn_pin = pin;
	_restore_active = activeHigh;
}

static bool _disconnect_flag = false;

void easyConfig::run() {
	_server->handleClient();
	
	// Restore Button
	if (_restore_btn && ((_restore_active && !digitalRead(_restore_btn_pin)) || (!_restore_active && digitalRead(_restore_btn_pin)))) {
		if (!_restore_btn_enter) {
			_restore_btn_enter_start = millis();
			_restore_btn_enter = true;
#ifdef DEBUG_CONFIG
			OUTPUT_DEBUG.println("[easyConfig] Start enter restore button");
#endif
		} else {
			if ((millis() - _restore_btn_enter_start) >= 5000) {
#ifdef DEBUG_CONFIG
				OUTPUT_DEBUG.println("[easyConfig] Restore config by button and restart");
#endif
				for (int i=0;i<4;i++) {
					digitalWrite(LED_DEBUG, !digitalRead(LED_DEBUG));
					delay(50);
				}
				restore(true);
			}
		}
	} else if (_restore_btn && _restore_btn_enter && ((_restore_active && digitalRead(_restore_btn_pin)) || (!_restore_active && !digitalRead(_restore_btn_pin)))) {
		_restore_btn_enter = false;
	}
	
	// On wait connect
	if (!WiFi.isConnected() && ((millis() - _blink_debug_led) >= 100) && ssid[0] != 0 && password[0] != 0) {
		_blink_debug_led = millis();
#ifdef DEBUG_CONFIG
		OUTPUT_DEBUG.println("[easyConfig] Wait connect");
#endif
		digitalWrite(LED_DEBUG, !digitalRead(LED_DEBUG));
	}

	static int wifi_state = 0;
	if (wifi_state == 0) {
		if (!WiFi.isConnected()) {
			if (ssid && ssid[0] != 0 && password && password[0] != 0) {
				WiFi.begin(ssid, password);
				wifi_state = 1;
			}
		} else {
			wifi_state = 2;
		}
	} else if (wifi_state == 1) {
		if (WiFi.isConnected()) {
			wifi_state = 2;
		} else if (_disconnect_flag) {
			_disconnect_flag = false;
			wifi_state = 99;
		}
	} else if (wifi_state == 2) {
		if (!WiFi.isConnected()) {
			wifi_state = 0;
		}
	} else if (wifi_state == 99) {
		static uint64_t start_wait = 0;
		if (start_wait == 0) {
			start_wait = millis();
		} else if ((millis() - start_wait) >= 1000) {
			start_wait = 0;
			wifi_state = 0;
		}
	}
}

void easyConfig::wifiConnect() {
#ifdef DEBUG_CONFIG
	OUTPUT_DEBUG.println("[easyConfig] Config event");
#endif
	WiFi.onEvent([](WiFiEvent_t event) {
#ifdef DEBUG_CONFIG
		OUTPUT_DEBUG.printf("[easyConfig] Event id: %d\n", event);
#endif

		switch(event) {
			case SYSTEM_EVENT_STA_CONNECTED:
#ifdef DEBUG_CONFIG
				OUTPUT_DEBUG.println("[easyConfig] WiFi Connected");
				OUTPUT_DEBUG.print("[easyConfig] IP address: ");
				OUTPUT_DEBUG.println(WiFi.localIP());
#endif
				break;
			case SYSTEM_EVENT_STA_STOP:
			case SYSTEM_EVENT_STA_DISCONNECTED:
			case SYSTEM_EVENT_STA_LOST_IP:
				digitalWrite(LED_DEBUG, LED_DEBUG_LOW);
				_disconnect_flag = true;
#ifdef DEBUG_CONFIG
				OUTPUT_DEBUG.println("[easyConfig] WiFi Disconnect");
#endif
				break;
		}
	});
	
#ifdef DEBUG_CONFIG
	OUTPUT_DEBUG.println("[easyConfig] begin connect to " + String(ssid));
#endif
}

void easyConfig::loadConfig() {
#ifdef DEBUG_CONFIG
	OUTPUT_DEBUG.println("[easyConfig] Open file /config.json read only");
#endif
	File configFile = SPIFFS.open("/config.json", "r");
	if (!configFile) {
#ifdef DEBUG_CONFIG
	OUTPUT_DEBUG.println("[easyConfig] Fail to open file /config.json");
#endif
		return;
	}

	size_t size = configFile.size();
	if (size > 1024) {
		return;
	}

	StaticJsonDocument<200> json;

	if (deserializeJson(json, configFile) != DeserializationError::Ok) {
#ifdef DEBUG_CONFIG
		OUTPUT_DEBUG.println("[easyConfig] json parse fail");
#endif
		return;
	}
	
	/*
	sprintf(ssid, "%s", (const char*)json["ssid"]);
	sprintf(password, "%s", (const char*)json["password"]);
	sprintf(name, "%s", (const char*)json["name"]);
	sprintf(AuthUsername, "%s", (const char*)json["auth-username"]);
	sprintf(AuthPassword, "%s", (const char*)json["auth-password"]);
	*/
	if (json.containsKey("ssid") && json["ssid"].is<const char*>()) {
		if (strcmp("NULL", json["ssid"]) == 0) ssid[0] = 0;
		else strcpy(ssid, json["ssid"]);
	}
	if (json.containsKey("password") && json["password"].is<const char*>()) {
		if (strcmp("NULL", json["password"]) == 0) password[0] = 0;
		else strcpy(password, json["password"]);
	}
	if (json.containsKey("name") && json["name"].is<const char*>()) {
		strcpy(name, json["name"]);
	}
	if (json.containsKey("auth-password") && json["auth-password"].is<const char*>()) {
		strcpy(AuthPassword, json["auth-password"]);
	}
	
	// Custom Config
	for (int index=0;index<_CustomConfigIndex;index++) {
		if (_CustomConfig[index][0].length() > 0 && _CustomConfig[index][0].charAt(0) != ':') {
			if (json.containsKey(_CustomConfig[index][0]) && json[_CustomConfig[index][0]].is<const char*>()) {
				_CustomConfig[index][1] = String((const char*)json[_CustomConfig[index][0]]);
			}
		}
	}
}

void easyConfig::saveConfig() {
#ifdef DEBUG_CONFIG
	OUTPUT_DEBUG.println("[easyConfig] config to json encode");
#endif
	StaticJsonDocument<200> json;
	json["ssid"] = ssid;
	json["password"] = password;
	json["name"] = name;
	json["auth-password"] = AuthPassword;
	
	// Custom Config
	for (int index=0;index<_CustomConfigIndex;index++) {
		if (_CustomConfig[index][0].length() > 0 && _CustomConfig[index][0].charAt(0) != ':') {
			json[_CustomConfig[index][0]] = _CustomConfig[index][1].c_str();
#ifdef DEBUG_CONFIG
			OUTPUT_DEBUG.println("[easyConfig] CustomConfig '" + _CustomConfig[index][0] + "' -> '" + _CustomConfig[index][1] + "'");
#endif
		}
	}

#ifdef DEBUG_CONFIG
	OUTPUT_DEBUG.println("[easyConfig] Open file /config.json write only");
#endif
	File configFile = SPIFFS.open("/config.json", "w");
	if (!configFile) {
#ifdef DEBUG_CONFIG
		OUTPUT_DEBUG.println("[easyConfig] Fail to open file /config.json");
#endif
		return;
	}

#ifdef DEBUG_CONFIG
	OUTPUT_DEBUG.println("[easyConfig] Write json config to /config.json");
#endif
	serializeJson(json, configFile);
}

// Add on v1.2
void easyConfig::cloudConfig(String name) {
	name.toLowerCase();
	if (name == "netpie") {
		addCustomConfig(":NETPIE");
		addCustomConfig("APPID");
		addCustomConfig("KEY");
		addCustomConfig("SECRET");
		addCustomConfig("ALIAS");
	} else if (name == "mqtt") {
		addCustomConfig(":MQTT");
		addCustomConfig("Server");
		addCustomConfig("Port");
		addCustomConfig("Username");
		addCustomConfig("Password");
	}
}

String easyConfig::configValue(String name) {
	String strVal = "NULL";
	if (name == "ssid") strVal = ssid;
	else if (name == "password") strVal = password;
	else if (name == "name") strVal = name;
	else if (name == "auth-password") strVal = AuthPassword;
	else {
		// Custom Config
		for (int index=0;index<_CustomConfigIndex;index++) {
			if (name == _CustomConfig[index][0] && _CustomConfig[index][0].charAt(0) != ':') {
				 strVal = _CustomConfig[index][1];
				 break;
			}
		}
	}
	return strVal;
}

void easyConfig::addCustomConfig(String name) {
	_CustomConfig[_CustomConfigIndex][0] = name;
	_CustomConfigIndex++;
}

bool easyConfig::isLogin() {
	return (_server->hasHeader("Cookie") && _server->header("Cookie").indexOf("auth=" + String(AuthPassword)) >= 0);
}