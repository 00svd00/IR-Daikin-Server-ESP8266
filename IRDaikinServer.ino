/*
* IRDaikinServer : Simple http interface to control Daikin A/C units via IR
* Version 0.1 July, 2017
* Copyright 2017 Andrea Cucci
*
* Dependencies:
*	- Arduino core for ESP8266 WiFi chip (https://github.com/esp8266/Arduino)
*	- IRremote ESP8266 Library (https://github.com/markszabo/IRremoteESP8266)
*
*/
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <ir_Daikin.h>
#include "IRDaikinServer.h"

#define DEBUG // If defined debug output is activated

#ifdef DEBUG
#define DEBUG_PRINT(x)		Serial.print(x)
#define DEBUG_BEGIN(x)		Serial.begin(x)
#define DEBUG_PRINTLN(x)	Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_BEGIN(x)
#define DEBUG_PRINTLN(x)
#endif 

IRDaikinESP daikinir(D2);  // An IR LED is controlled by GPIO pin 4 (D2)

const char* ssid = "...";
const char* password = "...";

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater; // Optional web interface to remotely update the firmware

setting_t fan_speeds[6] = { { "Auto", DAIKIN_FAN_AUTO },{ "Slowest", 1 },{ "Slow", 2 },{ "Medium", 3 },{ "Fast", 4 },{ "Fastest", 5 } };
setting_t modes[5] = { { "Cool", DAIKIN_COOL }, { "Heat", DAIKIN_HEAT }, { "Fan", DAIKIN_FAN }, { "Auto", DAIKIN_AUTO }, { "Dry", DAIKIN_DRY } };
setting_t on_off[2] = { { "On", 1 }, { "Off", 0 } };

String getSelection(String name, int min, int max, int selected, setting_t* list) {
	String ret = "<select name=\""+name+"\">";
	for (int i = min; i <= max; i++) {
		ret += "<option ";
		if (list[i].value == selected)
			ret += "selected ";
		ret += "value = " + String(list[i].value) + " > " + String(list[i].name) + "</option>";
	}
	return ret += "</select><br\>";
}

void handleRoot() {
	handleFileRead("/index.html");
}

void handleCmd() {
	String argName;
	uint8_t arg;
	for (uint8_t i = 0; i < server.args(); i++) {
		argName = server.argName(i);
		arg = strtoul(server.arg(i).c_str(), NULL, 10);
		if (argName == "temp") { daikinir.setTemp(arg); } 
		else if (argName == "fan") { daikinir.setFan(arg); }
		else if (argName == "power") { daikinir.setPower(arg); }
		else if (argName == "powerful") { daikinir.setPowerful(arg); }
		else if (argName == "quiet") { daikinir.setQuiet(arg); }
		else if (argName == "eco") { daikinir.setEcono(arg); }
		else if (argName == "swingh") { daikinir.setSwingHorizontal(arg); }
		else if (argName == "swingv") { daikinir.setSwingVertical(arg); }
		else if (argName == "mode") { daikinir.setMode(arg); }
		DEBUG_PRINT(argName);
		DEBUG_PRINT(" ");
		DEBUG_PRINTLN(arg);
	}
	daikinir.send();
	handleRoot();
}

String getContentType(String filename) { // convert the file extension to the MIME type
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  return "text/plain";
}

bool handleFileRead(String path) {
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  if (LittleFS.exists(path)) {
    File file = LittleFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  Serial.println("\tFile Not Found");
  return false;                                        
}

void setup(void) {	
	DEBUG_BEGIN(115200);

	daikinir.begin();

	// Begin LittleFS
  	if (!LittleFS.begin()) {
    	Serial.println("An Error has occurred while mounting LittleFS");
    	return;
  	}

	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
	DEBUG_PRINTLN("");

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		DEBUG_PRINT(".");
	}
	DEBUG_PRINTLN("");
	DEBUG_PRINT("Connected to ");
	DEBUG_PRINTLN(ssid);
	DEBUG_PRINT("IP address: ");
	DEBUG_PRINTLN(WiFi.localIP());

	server.on("/", handleRoot);
	server.on("/cmd", handleCmd);

	server.onNotFound([]() {
    	if (!handleFileRead(server.uri())) {
      		server.send(404, "text/plain", "404: Not Found");
		}
	});

	httpUpdater.setup(&server);
	server.begin();
	DEBUG_PRINTLN("HTTP server started");

	if (MDNS.begin("daikin", WiFi.localIP())) {
		DEBUG_PRINTLN("MDNS responder started");
	}
}

void loop(void) {
	server.handleClient();
}