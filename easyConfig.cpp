// Coding By IOXhop : www.ioxhop.com

#include "Arduino.h"
#include "ESP8266WiFi.h"
#include "WiFiClient.h"
#include "ESP8266WebServer.h"
#include "ArduinoJson.h"
#include "FS.h"
#include "easyConfig.h"

easyConfig::easyConfig(ESP8266WebServer &useServer): _server(&useServer) {
	ssid[0] = 0;
	password[0] = 0;
	sprintf(name, "ESP_easyConfig");
	sprintf(AuthUsername, "admin");
	sprintf(AuthPassword, "password");
}

void easyConfig::setValue(String key, String val) {
	if (key == "ssid") val.toCharArray(ssid, 20);
	if (key == "password") val.toCharArray(password, 20);
	if (key == "name") val.toCharArray(name, 20);
	if (key == "auth-username") val.toCharArray(AuthUsername, 20);
	if (key == "auth-password") val.toCharArray(AuthPassword, 20);
}

void easyConfig::begin(bool runWebServer) {
#ifdef DEBUG
	OUTPUT_DEBUG.println("[easyConfig] Set pin " + String(LED_DEBUG) + " to output");
#endif
	pinMode(LED_DEBUG, OUTPUT);
	digitalWrite(LED_DEBUG, LED_DEBUG_LOW);

#ifdef DEBUG
	OUTPUT_DEBUG.println("[easyConfig] Begin SPIFFS");
#endif
	if (!SPIFFS.begin()) {
#ifdef DEBUG
		OUTPUT_DEBUG.println("[easyConfig] Failed to mount file system");
#endif
		delay(5000);
		ESP.restart();
		return;
	}
	
#ifdef DEBUG
	OUTPUT_DEBUG.println("[easyConfig] call function easyConfig::loadConfig()");
#endif
	loadConfig();
#ifdef DEBUG
	OUTPUT_DEBUG.println("[easyConfig] set WiFi mode to " + String(_mode));
#endif
	WiFi.mode(_mode);
#ifdef DEBUG
	OUTPUT_DEBUG.println("[easyConfig] set Soft AP SSID to " + String(name));
#endif
	WiFi.softAP(name);
	if (ssid[0] != 0 && password[0] != 0) {
#ifdef DEBUG
		OUTPUT_DEBUG.println("[easyConfig] begin connect to " + String(ssid));
#endif
		WiFi.begin(ssid, password);
		
#ifdef DEBUG
		OUTPUT_DEBUG.println("[easyConfig] set timeout to 30S");
		OUTPUT_DEBUG.print("[easyConfig] Connect");
#endif
		int timeOut = 300; // Set time out to 30S
		while (WiFi.status() != WL_CONNECTED && timeOut>0) {
			// Serial.print(".");
#ifdef DEBUG
			OUTPUT_DEBUG.print(".");
#endif
			delay(100);
			digitalWrite(LED_DEBUG, !digitalRead(LED_DEBUG));
			timeOut--;
		}
#ifdef DEBUG
		OUTPUT_DEBUG.println();
#endif
		if (timeOut > 0) {
			_connectAP = true;
#ifdef DEBUG
			OUTPUT_DEBUG.println("[easyConfig] Connected");
#endif
			digitalWrite(LED_DEBUG, LED_DEBUG_HIGH);
		} else {
			_connectAP = false;
#ifdef DEBUG
			OUTPUT_DEBUG.println("[easyConfig] connect timeout.");
#endif
			digitalWrite(LED_DEBUG, LED_DEBUG_LOW);
		}
	}
	
	_server->on("/config", [&]() {
		if (!_server->authenticate(AuthUsername, AuthPassword)) {
			_server->requestAuthentication();
			return;
		}
		if (_server->method() == HTTP_GET) {
			String tmpConfigPage = configPageHTML;
			tmpConfigPage.replace("{ssid}", String(ssid));
			tmpConfigPage.replace("{password}", String(password));
			tmpConfigPage.replace("{name}", String(name));
			tmpConfigPage.replace("{auth-username}", String(AuthUsername));
			tmpConfigPage.replace("{auth-password}", String(AuthPassword));
			_server->send(200, "text/html", tmpConfigPage);
		} else if (_server->method() == HTTP_POST) {
			String tmpSSID, tmpPassword, tmpName, tmpAuthUsername, tmpAuthPassword;
			for (uint8_t i=0; i<_server->args(); i++){
				if (_server->argName(i) == "ssid") tmpSSID = _server->arg(i)=="" ? "NULL" : _server->arg(i);
				if (_server->argName(i) == "password") tmpPassword = _server->arg(i)=="" ? "NULL" : _server->arg(i);
				if (_server->argName(i) == "name") tmpName = _server->arg(i);
				if (_server->argName(i) == "auth-username") tmpAuthUsername = _server->arg(i);
				if (_server->argName(i) == "auth-password") tmpAuthPassword = _server->arg(i);
			}
			tmpSSID.toCharArray(ssid, 20);
			tmpPassword.toCharArray(password, 20);
			tmpName.toCharArray(name, 20);
			tmpAuthUsername.toCharArray(AuthUsername, 20);
			tmpAuthPassword.toCharArray(AuthPassword, 20);
			saveConfig();
			_server->send(200, "text/plain", "Save and reboot, Please wait 30 Sec - 2 Min.");
			ESP.restart();
		}
	});
	
	_server->on("/config/restart", [&]() {
		_server->send(200, "text/plain", "Restart now, Please wait 30 Sec - 2 Min.");
		ESP.restart();
	});
	
	_server->on("/config/restore", [&]() {
		_server->send(200, "text/plain", "Restore and reboot, Please wait 30 Sec - 2 Min.");
		restore(true);
	});
	
	if (runWebServer) {
		_server->begin();
	}
}

void easyConfig::setMode(WiFiMode mode) {
	_mode = mode;
}

bool easyConfig::isConnect() {
	return _connectAP;
}

void easyConfig::restore(bool reboot) {
	if (SPIFFS.exists("/config.json")) {
		SPIFFS.remove("/config.json");
	}
	if (reboot) {
		ESP.restart();
	}
}

void easyConfig::restoreButton(int pin) {
	restoreButton(pin, true);
}

void easyConfig::restoreButton(int pin, bool activeHigh) {
	pinMode(pin, INPUT);
	_restore_btn = true;
	_restore_btn_pin = pin;
	_restore_active = activeHigh;
}

void easyConfig::run() {
	_server->handleClient();
	
	// Restore Button
	if (_restore_btn && ((_restore_active && !digitalRead(_restore_btn_pin)) || (!_restore_active && digitalRead(_restore_btn_pin)))) {
		if (!_restore_btn_enter) {
			_restore_btn_enter_start = millis();
			_restore_btn_enter = true;
		} else {
			if ((millis() - _restore_btn_enter_start) >= 5000) {
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
	delay(10);
}

void easyConfig::loadConfig() {
#ifdef DEBUG
	OUTPUT_DEBUG.println("[easyConfig] Open file /config.json read only");
#endif
	File configFile = SPIFFS.open("/config.json", "r");
	if (!configFile) {
#ifdef DEBUG
	OUTPUT_DEBUG.println("[easyConfig] Fail to open file /config.json");
#endif
		return;
	}

	size_t size = configFile.size();
	if (size > 1024) {
		return;
	}

	std::unique_ptr<char[]> buf(new char[size]);

	configFile.readBytes(buf.get(), size);

	StaticJsonBuffer<200> jsonBuffer;
	JsonObject& json = jsonBuffer.parseObject(buf.get());

	if (!json.success()) {
#ifdef DEBUG
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
		if (stricmp("NULL", json["ssid"]) == 0) ssid[0] = 0;
		else strcpy(ssid, json["ssid"]);
	}
	if (json.containsKey("password") && json["password"].is<const char*>()) {
		if (stricmp("NULL", json["password"]) == 0) password[0] = 0;
		else strcpy(password, json["password"]);
	}
	if (json.containsKey("name") && json["name"].is<const char*>()) {
		strcpy(name, json["name"]);
	}
	if (json.containsKey("auth-username") && json["auth-username"].is<const char*>()) {
		strcpy(AuthUsername, json["auth-username"]);
	}
	if (json.containsKey("auth-password") && json["auth-password"].is<const char*>()) {
		strcpy(AuthPassword, json["auth-password"]);
	}
}

void easyConfig::saveConfig() {
#ifdef DEBUG
	OUTPUT_DEBUG.println("[easyConfig] config to json encode");
#endif
	StaticJsonBuffer<200> jsonBuffer;
	JsonObject& json = jsonBuffer.createObject();
	json["ssid"] = ssid;
	json["password"] = password;
	json["name"] = name;
	json["auth-username"] = AuthUsername;
	json["auth-password"] = AuthPassword;

#ifdef DEBUG
	OUTPUT_DEBUG.println("[easyConfig] Open file /config.json write only");
#endif
	File configFile = SPIFFS.open("/config.json", "w");
	if (!configFile) {
#ifdef DEBUG
		OUTPUT_DEBUG.println("[easyConfig] Fail to open file /config.json");
#endif
		return;
	}

	// json.printTo(Serial);
#ifdef DEBUG
	OUTPUT_DEBUG.println("[easyConfig] Write json config to /config.json");
#endif
	json.printTo(configFile);
}