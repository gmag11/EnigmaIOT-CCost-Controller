/*
 Name:		EnigmaIOT_CCost_Controller.ino
 Created:	5/10/2020 6:13:20 PM
 Author:	gmartin
*/

#if !defined ESP8266 && !defined ESP32
#error Node only supports ESP8266 or ESP32 platform
#endif

#include <Arduino.h>
//#include <DebounceEvent.h>
#include <EnigmaIOTjsonController.h>
#include "CCostController.h"

#include <EnigmaIOTNode.h>
#include <espnow_hal.h>
#include <CayenneLPP.h>
#include <ArduinoJson.h>
#include <CurrentCostLib.h>

#ifdef TEST
#include <Ticker.h>
#endif

#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <ESPAsyncTCP.h> // Comment to compile for ESP32
#include <Hash.h>
#elif defined ESP32
#include <WiFi.h>
#include <SPIFFS.h>
//#include <AsyncTCP.h> // Comment to compile for ESP8266
#include <SPIFFS.h>
#include <Update.h>
#include <driver/adc.h>
#include "esp_wifi.h"
#endif
#include <ArduinoJson.h>
#include <Curve25519.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>
#include <DNSServer.h>
#include <FS.h>
#include <SoftwareSerial.h> // https://github.com/plerup/espsoftwareserial

#ifndef LED_BUILTIN
#define LED_BUILTIN 2 // ESP32 boards normally have a LED in GPIO3 or GPIO5
#endif // !LED_BUILTIN

// #define USE_SERIAL // Don't forget to set DEBUG_LEVEL to NONE if serial is disabled

#define BLUE_LED LED_BUILTIN

EnigmaIOTjsonController* controller;

#define RESET_PIN 13

void connectEventHandler () {
	DEBUG_WARN ("Connected");
}

void disconnectEventHandler (nodeInvalidateReason_t reason) {
	DEBUG_WARN ("Disconnected. Reason %d", reason);
}

bool sendUplinkData (const uint8_t* data, size_t len, nodePayloadEncoding_t payloadEncoding) {
	return EnigmaIOTNode.sendData (data, len, payloadEncoding);
}

void processRxData (const uint8_t* mac, const uint8_t* buffer, uint8_t length, nodeMessageType_t command, nodePayloadEncoding_t payloadEncoding) {
	if (controller->processRxCommand (mac, buffer, length, command, payloadEncoding)) {
		DEBUG_INFO ("Command processed");
	} else {
		DEBUG_WARN ("Command error");
	}
}

void wifiManagerExit (boolean status) {
	controller->configManagerExit (status);
}

void wifiManagerStarted () {
	controller->configManagerStart (&EnigmaIOTNode);
}

void setup () {

#ifdef USE_SERIAL
	Serial.begin (115200);
	delay (1000);
	Serial.println ();
#endif

	controller = (EnigmaIOTjsonController*)new CCostController ();

	EnigmaIOTNode.setLed (BLUE_LED);
	EnigmaIOTNode.setResetPin (RESET_PIN);
	EnigmaIOTNode.onConnected (connectEventHandler);
	EnigmaIOTNode.onDisconnected (disconnectEventHandler);
	EnigmaIOTNode.onDataRx (processRxData);
	EnigmaIOTNode.enableClockSync (false);
	EnigmaIOTNode.onWiFiManagerStarted (wifiManagerStarted);
	EnigmaIOTNode.onWiFiManagerExit (wifiManagerExit);

	if (!controller->loadConfig ()) {
		DEBUG_WARN ("Error reading config file");
		if (SPIFFS.format ())
			DEBUG_WARN ("SPIFFS Formatted");
	}

	EnigmaIOTNode.begin (&Espnow_hal, NULL, NULL, true, false);

	uint8_t macAddress[ENIGMAIOT_ADDR_LEN];
#ifdef ESP8266
	if (wifi_get_macaddr (STATION_IF, macAddress))
#elif defined ESP32
	if ((esp_wifi_get_mac (WIFI_IF_STA, macAddress) == ESP_OK))
#endif
	{
		EnigmaIOTNode.setNodeAddress (macAddress);
		char macStr[ENIGMAIOT_ADDR_LEN * 3];
		DEBUG_DBG ("Node address set to %s", mac2str (macAddress, macStr));
	} else {
		DEBUG_WARN ("Node address error");
	}

	controller->sendDataCallback (sendUplinkData);
	controller->setup ();

	DEBUG_DBG ("END setup");
}

void loop () {
	controller->loop ();
	EnigmaIOTNode.handle ();
}
