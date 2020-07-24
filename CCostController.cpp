// 
// 
// 

#include <SoftwareSerial.h> // https://github.com/plerup/espsoftwareserial
#include "CCostController.h"
#include "debug.h"
#include <functional>

using namespace std;
using namespace placeholders;

constexpr auto CONFIG_FILE = "/ccostconf.json"; ///< @brief CCost controller configuration file name

#ifdef DEBUG_SERIAL
#define SOFT_SERIAL_RX_PIN 5 // D1
#define SW_SERIAL_UNUSED_PIN -1
SoftwareSerial sserial (SOFT_SERIAL_RX_PIN, SW_SERIAL_UNUSED_PIN);
#endif

bool CCostController::processRxCommand (const uint8_t* mac, const uint8_t* buffer, uint8_t length, nodeMessageType_t command, nodePayloadEncoding_t payloadEncoding) {
	return true;
}


bool CCostController::sendCommandResp (const char* command, bool result) {
	return true;
}

bool CCostController::sendStartAnouncement () {
	return true;
}

void CCostController::processSensorEvent (uint8_t id, int watts, float tempr) {
	DEBUG_WARN ("---->  Event %d. Power: %d W. Temp: %.2f C\n", id, watts, tempr);

	sensorData.id = id;
	if (sensorData.id == 0) {
		sensorData.tempr = averageFilter->addValue (medianFilter->addValue (tempr));
		sensorData.raw_tempr = tempr;
	}

	sensorData.watts = watts;

	pendingData = true;
}

void CCostController::setup (void* data) {
	medianFilter = new FilterClass (MEDIAN_FILTER, 11);
	averageFilter = new FilterClass (AVERAGE_FILTER, 14);

#ifdef DEBUG_SERIAL
	Serial.begin (115200);
	pinMode (SOFT_SERIAL_RX_PIN, INPUT_PULLUP);
	sserial.begin (57600);
	currentCost.begin (sserial);
#else
	Serial.begin (57600);
	currentCost.begin (Serial);
#endif
	currentCost.onSensorEvent (std::bind (&CCostController::processSensorEvent, this, _1, _2, _3));


	sendStartAnouncement ();

	DEBUG_DBG ("Finish begin");

}

void CCostController::loop () {
	currentCost.handle ();

	if (pendingData) {
		pendingData = false;
		const size_t capacity = JSON_OBJECT_SIZE (4);
		DynamicJsonDocument json (capacity);
		char topic[10];
		snprintf (topic, 10, "sensor_%d", sensorData.id);

		json["sens"] = sensorData.id;
		json["w"] = sensorData.watts;
		if (sensorData.id == 0) {
			json["tempr"] = sensorData.tempr;
			json["raw_tempr"] = sensorData.raw_tempr;
		}

		sendJson (json);
	}
}

CCostController::~CCostController () {
	delete(medianFilter);
	delete(averageFilter);
}

void CCostController::configManagerStart (EnigmaIOTNodeClass* node) {
	enigmaIotNode = node;
	DEBUG_INFO ("==== CCost Controller Configuration start ====");
}

void CCostController::configManagerExit (bool status) {
	DEBUG_INFO ("==== CCost Controller Configuration result ====");
}

bool CCostController::loadConfig () {
	return true;
}

bool CCostController::saveConfig () {
	return true;
}