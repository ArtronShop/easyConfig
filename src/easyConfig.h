// Coding By IOXhop : www.ioxhop.com
// This version 1.3

/*
 * .:: Minify ::.
 * HTML : https://kangax.github.io/html-minifier/
 * CSS  : https://cssminifier.com/
 * JS   : http://jscompress.com/
 *
 * .:: Text to C String ::.
 *      : http://tomeko.net/online_tools/cpp_text_escape.php?lang=en
 *
 * Coding -> Minify -> Text to C String -> Copy & Paste to this
 */

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
#include "FS.h"

// Debug
#define LED_DEBUG 2
#define LED_DEBUG_HIGH LOW
#define LED_DEBUG_LOW HIGH

// #define DEBUG_CONFIG
#define OUTPUT_DEBUG Serial

class easyConfig {
	public:
#ifdef ESP32
		easyConfig(WebServer &useServer) ;
#else
		easyConfig(ESP8266WebServer &useServer) ;
#endif
		void setValue(String name, String val) ;
		void begin(bool runWebServer) ;
#ifdef ESP32
		void setMode(WiFiMode_t mode) ;
#else
		void setMode(WiFiMode mode) ;
#endif
		bool isConnected() ;
		void run() ;
		void restore(bool reboot) ;
		void restoreButton(int pin, bool activeHigh=true) ;
		
		// Add on V1.2
		void cloudConfig(String name) ;
		String configValue(String name) ;
		
		
	private:
#ifdef ESP32
		WiFiMode_t _mode = WIFI_AP_STA;
#else
		WiFiMode _mode = WIFI_AP_STA;
#endif
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
		// Add on V1.3
		bool isLogin();

#ifdef ESP32
		WebServer *_server;
#else
		ESP8266WebServer *_server;
#endif

String templateHTML = 
"<!DOCTYPE HTML>\n"
"<html>"
"<head>"
"<meta charset=\"utf-8\">"
"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
"<title>easyConfig</title>"
"<style>"
".box-auth-pass,.box-keypad,.btn,h2{text-align:center}*{box-sizing:border-box}body{font-family:Arial,Helvetica,sans-serif;margin:20px 0 0;background:#F5F5F5;color:#333}fieldset{border:none;margin:0 0 10px;padding:0}fieldset>div{padding:5px;margin-bottom:10px}fieldset>div>label{width:140px;display:inline-block}legend{font-size:18px;width:100%;background:#ECECEC;color:#BBB;padding:5px 5px 3px;margin-bottom:10px}.main-box{max-width:768px;background:#FFF;margin:auto auto 20px;box-shadow:0 0 5px rgba(0,0,0,.1);padding:10px}.btn-p,.btn-restart,button[type=submit],h1{background:#2196F3}h1{margin:-10px -10px 10px;padding:20px 15px;color:#FFF}input{border:none;border-bottom:2px solid #EFEFEF;padding:4px;outline:0;font-size:16px;transition:border-bottom-color 300mS}input:focus{border-bottom-color:#2196F3}.btn,button[type=submit],button[type=reset]{width:50%;color:#FFF!important;border:none;outline:0;font-size:16px;padding:10px}.btn-p:hover,.btn-restart:hover,button[type=submit]:hover{background:#1976D2}.btn-restore,button[type=reset]{background:#CCC}.btn-restore:hover,button[type=reset]:hover{background:#999}.btn{width:100%;display:inline-block;text-decoration:none}@media screen and (max-width:768px){body{margin:0;background:#FFF}.main-box{box-shadow:none}}@media screen and (max-width:480px){h1{margin-bottom:0}fieldset>div>input,fieldset>div>label{width:100%;margin-bottom:10px}fieldset>div>label{display:block}form{margin:0 -10px}legend{padding:5px 15px 3px}fieldset>div{padding:5px 15px}.main-box{padding-bottom:0}}h2{color:#333}.box-auth-pass{padding:20px 0}.box-auth-pass>span{padding:0 4px;border-bottom:2px solid #ECECEC;margin:0 5px;display:inline-block;font-size:28px;color:#666}.box-keypad>div{margin-bottom:10px}.box-keypad button{display:inline-block;width:auto;font-size:22px;margin:0 5px}a{color:#2196F3}a:hover{color:#1565C0}.loading{position:fixed;top:0;left:0;right:0;bottom:0;background:#FFF;display:none}.loading>spen{border-radius:100%;border:5px solid #ECECEC;border-bottom-color:#2196F3;position:absolute;top:50%;left:50%;width:80px;height:80px;display:block;animation:rotateLoop 1s linear infinite;margin:-40px 0 0 -40px}.box-list,.box-list>.bk-black{position:fixed;left:0;right:0;top:0;bottom:0;height:100%;width:100%}@keyframes rotateLoop{from{transform:rotate(0)}to{transform:rotate(360deg)}}.box-list{display:none;padding:20px;overflow:auto}.box-list>.bk-black{background:rgba(0,0,0,.6)}.box-list>.content{max-width:300px;background:#FFF;margin:auto;position:relative;box-shadow:0 0 10px rgba(0,0,0,.41);overflow:auto}.box-list>.content>ul{width:100%;margin:0;padding:0}.box-list>.content>ul>li{padding:10px 16px;border-bottom:1px solid #e4e4e4;list-style:none;text-align:left}.box-list>.content>ul>li:last-child{border-bottom:none}.box-list>.content>ul>li:hover{background:#f7f7f7}.label{font-size:12px;background:#eaeaea;border-radius:3px;padding:2px 6px}"
/*
* {box-sizing: border-box}
body {font-family: Arial, Helvetica, sans-serif;margin: 0;background: #F5F5F5;margin-top: 20px;color: #333}
fieldset {border: none;margin:0;margin-bottom: 10px;padding: 0}
fieldset > div {padding: 5px;margin-bottom: 10px}
fieldset > div > label {width: 140px;display: inline-block}
legend {font-size: 18px;width: 100%;padding: 0;background: #ECECEC;color: #BBB;padding: 5px;padding-bottom: 3px;margin-bottom: 10px}
.main-box {max-width: 768px;background: #FFF;margin: auto;box-shadow: 0 0 5px rgba(0, 0, 0, 0.1);padding: 10px;margin-bottom:20px}
h1 {margin: 0;margin: -10px -10px 10px -10px;background: #2196F3;padding: 20px 15px;color: #FFF}
input {border: none;border-bottom: 2px solid #EFEFEF;padding: 4px;outline: none;font-size: 16px;transition: border-bottom-color 300mS}
input:focus {border-bottom-color: #2196F3}
button[type='submit'],button[type='reset'],.btn {width: 50%;color: #FFF !important;border: none;outline: none;font-size: 16px;padding: 10px}
button[type='submit'],.btn-restart,.btn-p {background: #2196F3}
button[type='submit']:hover,.btn-restart:hover,.btn-p:hover {background: #1976D2}
button[type='reset'],.btn-restore {background: #CCC}
button[type='reset']:hover,.btn-restore:hover {background: #999}
.btn{width:100%;display:inline-block;text-decoration:none;text-align: center}
@media screen and (max-width: 768px) {body {margin: 0;background:#FFF;}.main-box{ box-shadow:none;}}
@media screen and (max-width: 480px) {
	h1 {margin-bottom: 0}
	fieldset > div > label {display: block;width: 100%;margin-bottom: 10px}
	fieldset > div > input {width: 100%;margin-bottom: 10px}
	form {margin: 0 -10px}
	legend {padding: 5px 15px 3px 15px}
	fieldset > div {padding: 5px 15px}
	.main-box {padding-bottom: 0;}
}
h2{text-align: center;color: #333333;}
.box-auth-pass {text-align: center;padding: 20px 0;}
.box-auth-pass > span {padding: 0 4px;border-bottom: 2px solid #ECECEC;margin: 0 5px;display: inline-block;font-size: 28px;color: #666}
.box-keypad {text-align: center}
.box-keypad > div {margin-bottom: 10px}
.box-keypad button{display: inline-block;width: auto;font-size: 22px;margin: 0 5px}
a{color:#2196F3}
a:hover{color:#1565C0}
.loading {position: fixed;top: 0;left: 0;right: 0;bottom: 0;background: #FFF;display: none}
.loading > spen {border-radius: 100%;border: 5px solid #ECECEC;border-bottom-color: #2196F3;position: absolute;top: 50%;left: 50%;width: 80px;height: 80px;display: block;animation: rotateLoop 1s linear infinite;margin: -40px 0 0 -40px}
@keyframes rotateLoop {
	from {transform: rotate(0deg)}
	to{transform: rotate(360deg)}
}
.box-list {display:none;position: fixed;left: 0;right:0;top: 0;bottom: 0;width:100%;height: 100%;padding: 20px;overflow: auto}
.box-list > .bk-black {position: fixed;left: 0;right:0;top: 0;bottom: 0;width:100%;height: 100%;background: rgba(0, 0, 0, 0.6)}
.box-list > .content {max-width: 300px;background: #FFF;margin: auto;position: relative;box-shadow: 0 0 10px rgba(0, 0, 0, 0.41);overflow: auto;}
.box-list > .content > ul {width: 100%;margin: 0;padding: 0}
.box-list > .content > ul > li {padding: 10px 16px;border-bottom: 1px solid #e4e4e4;list-style: none;text-align: left}
.box-list > .content > ul > li:last-child {border-bottom: none}
.box-list > .content > ul > li:hover {background: #f7f7f7}
.label { font-size:12px; background: #eaeaea; border-radius: 3px; padding: 2px 6px; }
*/
"</style>"
"</head>"
""
"<body>"
"<div class=\"main-box\">"
"<h1>easyConfig</h1>"
"{INCODEHTML}"
"</div>"
"</body>"
"</html>"
;

String configPageHTML = 
"<form action=\"\"method=post><fieldset><legend>Device Settings</legend><div><label for=name>Device Name</label><input id=name name=name value=\"{name}\" maxlength=20 required></div></fieldset>{CustomSet}<fieldset><legend>WiFi Connect</legend><div><label for=ssid>WiFi Name</label><input id=ssid name=ssid value=\"{ssid}\" maxlength=20> <a href=# id=scan-wifi>Scan</a></div><div><label for=password>WiFi Password</label><input id=password name=password value=\"{password}\" maxlength=20></div><div><label>Status</label><spen class=label>{STATUS}</spen></div></fieldset><fieldset><legend>Auth Login</legend><div><label for=auth-password>Password</label><input id=auth-password name=auth-password value=\"{auth-password}\" required pattern=[0-9]{6} title=\"Enter config auth password (Number 6 char)\"></div></fieldset><fieldset><legend>Systems management</legend><div><a href=./config/restart class=\"btn btn-restart\">Restart device</a></div><div><a href=./config/restore class=\"btn btn-restore\"onclick='return confirm(\"Restore all config to default Yes or No ?\")'>Restore config to Default</a></div></fieldset><button type=submit>Save</button><button type=reset>Reset</button></form><div class=loading id=loader><spen></spen></div><div class=box-list><div class=bk-black></div><div class=content><ul id=list-content></ul></div></div>"
/*
  <form action="" method="post">
    <fieldset>
      <legend>Device Settings</legend>
      <div>
        <label for="name">Device Name</label>
        <input type="text" id="name" name="name" value="{name}" maxlength="20" required>
      </div>
    </fieldset>
{CustomSet}
    <fieldset>
      <legend>WiFi Connect</legend>
      <div>
        <label for="ssid">WiFi Name</label>
        <input type="text" id="ssid" name="ssid" value="{ssid}" maxlength="20">
        <a id="scan-wifi" href="#">Scan</a>
      </div>
      <div>
        <label for="password">WiFi Password</label>
        <input type="text" id="password" name="password" value="{password}" maxlength="20">
      </div>
      <div>
        <label>Status</label>
        <spen class="label">{STATUS}</spen>
      </div>
    </fieldset>
    <fieldset>
      <legend>Auth Login</legend>
      <div>
        <label for="auth-password">Password</label>
        <input type="text" id="auth-password" name="auth-password" value="{auth-password}" pattern="[0-9]{6}" title="Enter config auth password (Number 6 char)" required>
      </div>
    </fieldset>
    <fieldset>
      <legend>Systems management</legend>
      <div>
        <a href="./config/restart" class="btn btn-restart">Restart device</a>
      </div>
      <div>
        <a href="./config/restore" class="btn btn-restore" onClick="return confirm('Restore all config to default Yes or No ?');">Restore config to Default</a>
      </div>
    </fieldset>
    <button type="submit">Save</button><button type="reset">Reset</button>
  </form>
  <div class="loading" id="loader"><spen></spen></div>
  <div class="box-list">
    <div class="bk-black"></div>
    <div class="content">
      <ul id="list-content"></ul>
    </div>
  </div>
*/
"<script>"
"var ScanWiFi=[];$=function(t){return 1==document.querySelectorAll(t).length?document.querySelectorAll(t)[0]:document.querySelectorAll(t)},show=function(t,n){t.style.display=n?\"block\":\"none\"};var SelectList=function(t,n){show($(\".box-list\"),!0),$(\"#list-content\").innerHTML=\"\";for(var e=0;e<t.length;e++)$(\"#list-content\").innerHTML+='<li data-index=\"'+e+'\">'+t[e]+\"</li>\";if(\"function\"==typeof n)for(var e=0;e<$(\"#list-content > li\").length;e++)$(\"#list-content > li\")[e].addEventListener(\"click\",function(){show($(\".box-list\"),!1),n(this.getAttribute(\"data-index\"),this.textContent)})};$(\"#scan-wifi\").addEventListener(\"click\",function(){show($(\"#loader\"),!0);var t=new XMLHttpRequest;t.onreadystatechange=function(){if(4==t.readyState){if(show($(\"#loader\"),!1),200!=t.status)return void window.alert(\"Error scan\");if(ScanWiFi=JSON.parse(t.responseText),ScanWiFi.length<=0)return void window.alert(\"Not found wifi\");for(var n=[],e=0;e<ScanWiFi.length;e++)n[e]=ScanWiFi[e][0]+\" (\"+ScanWiFi[e][1]+\")\"+(ScanWiFi[e][2]?\"*\":\"\");SelectList(n,function(t,n){$(\"#ssid\").value=ScanWiFi[t][0];var e=$(\"#password\");ScanWiFi[t][2]?(e.focus(),e.select()):e.value=\"\"})}},t.open(\"GET\",\"{ROOT}/scan\",!0),t.send()});"
/*
var ScanWiFi = [];
$ = function (query) {return document.querySelectorAll(query).length == 1 ? document.querySelectorAll(query)[0] : document.querySelectorAll(query)};
show = function(e, s) { e.style.display = (s ? 'block' : 'none') }
var SelectList = function(list, onSelect) {
	show($(".box-list"), true);
	$("#list-content").innerHTML = '';
	for (var i=0;i<list.length;i++) {
		$("#list-content").innerHTML += '<li data-index="' + i + '">' + list[i] + '</li>';
	}
	if (typeof onSelect === 'function') {
		for (var i=0;i<$("#list-content > li").length;i++) {
			$("#list-content > li")[i].addEventListener("click", function () {
				show($(".box-list"), false);
				onSelect(this.getAttribute("data-index"), this.textContent);
			});
		}
	}
}

$("#scan-wifi").addEventListener("click", function () {
	show($("#loader"), true);
	
	var xhttp = new XMLHttpRequest();
	xhttp.onreadystatechange = function() {
		if (xhttp.readyState != 4) return;
		show($("#loader"), false);
		if (xhttp.status != 200) {
			window.alert("Error scan");
			return;
		}
		
		ScanWiFi = JSON.parse(xhttp.responseText);
		if (ScanWiFi.length <= 0) {
			window.alert("Not found wifi");
			return;
		}
		var ListWiFi = [];
		for (var i=0;i<ScanWiFi.length;i++) {
			ListWiFi[i] = ScanWiFi[i][0] + " (" + ScanWiFi[i][1] + ")" + (ScanWiFi[i][2] ? "*" : "");
		}
		SelectList(ListWiFi, function(index, SelectName) {
			$("#ssid").value = ScanWiFi[index][0];
			var p=$("#password")
			if (ScanWiFi[index][2]) {
				p.focus();
				p.select();
			}
			else p.value='';
		});
	};
	xhttp.open("GET", "{ROOT}/scan", true);
	xhttp.send();
});
*/
"</script>"
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
