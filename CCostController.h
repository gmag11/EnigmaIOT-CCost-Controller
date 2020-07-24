// CCostController.h

#ifndef _CCOSTCONTROLLER_h
#define _CCOSTCONTROLLER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

//#define DEBUG_SERIAL

#ifdef ESP32
#include <SPIFFS.h>
#endif

#include <EnigmaIOTjsonController.h>
#include <CurrentCostLib.h>

typedef struct {
	uint8_t id;
	int watts;
	float tempr;
	float raw_tempr;
} powerSensor;

//#define NUM_SENSORS 2

class CCostController : EnigmaIOTjsonController {
protected:
	FilterClass* medianFilter;
	FilterClass* averageFilter;
	powerSensor sensorData;
	bool pendingData = false;

public:
	void setup (void* data = NULL);
	bool processRxCommand (const uint8_t* mac, const uint8_t* buffer, uint8_t length, nodeMessageType_t command, nodePayloadEncoding_t payloadEncoding);
	void loop ();
	~CCostController ();
	/**
	 * @brief Called when wifi manager starts config portal
	 * @param enigmaIotGw Pointer to EnigmaIOT gateway instance
	 */
	void configManagerStart (EnigmaIOTNodeClass* node);

	/**
	 * @brief Called when wifi manager exits config portal
	 * @param status `true` if configuration was successful
	 */
	void configManagerExit (bool status);

	/**
	 * @brief Loads output module configuration
	 * @return Returns `true` if load was successful. `false` otherwise
	 */
	bool loadConfig ();

protected:
	/**
	  * @brief Saves output module configuration
	  * @return Returns `true` if save was successful. `false` otherwise
	  */
	bool saveConfig ();
	bool sendCommandResp (const char* command, bool result);
	bool sendStartAnouncement ();

	void processSensorEvent (uint8_t id, int watts, float tempr);
};

#endif

