#include "ui_ble.h"

#define DEBUG_BLE true

const char BC_DEVICE_NAME[] = "Boiler Controller";
const char BC_CONTROLLER_SERVICE_ID[] = "4C-EF-DD-58-CB-95-44-50-90-FB-F4-04-DC-20-2F-7C";

void BLEUI::setup() {
  ble.assertOK(ble.begin(DEBUG_BLE), F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));

  /* Perform a factory reset to make sure everything is in a known state */
  ble.assertOK(ble.factoryReset(), F("Could not factory reset"));

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  /* Print Bluefruit information */
  ble.info();

  // this line is particularly required for Flora, but is a good idea anyways for the super long lines ahead!
  // ble.setInterCharWriteDelay(5); // 5 ms

  ble.setGattDeviceName(BC_DEVICE_NAME);
  
  controllerServiceId =      ble.addGattService(BC_CONTROLLER_SERVICE_ID);
  stateCharId =              ble.addGattCharacteristic(0x0001, CHAR_PROP_READ | CHAR_PROP_NOTIFY, 1, 1);
  timeInStateCharId =        ble.addGattCharacteristic(0x0002, CHAR_PROP_READ | CHAR_PROP_NOTIFY, 4, 4);  // milliseconds
  timeHeatingCharId =        ble.addGattCharacteristic(0x0006, CHAR_PROP_READ | CHAR_PROP_NOTIFY, 4, 4);
  targetTempCharId =         ble.addGattCharacteristic(0x0003, CHAR_PROP_READ | CHAR_PROP_WRITE,  2, 2);
  waterSensorCharId =        ble.addGattCharacteristic(0x0004, CHAR_PROP_READ | CHAR_PROP_NOTIFY, 2, 2);
  ambientSensorCharId =      ble.addGattCharacteristic(0x0005, CHAR_PROP_READ | CHAR_PROP_NOTIFY, 4, 4);

  /* Add the Heart Rate Service to the advertising data (needed for Nordic apps to detect the service) */
  ble.assertOK(ble.sendCommandCheckOK( F("AT+GAPSETADVDATA=02-01-06-05-02-0d-18-0a-18")), F("Could not set advertising data"));

  /* Reset the device for the new service setting changes to take effect */
  ble.reset();
}

void BLEUI::readUserCommand() {
  if (context != NULL) { } // prevent 'unused parameter' warning
}

void BLEUI::processReadWriteRequests(ReadWriteRequests requests, BoilerStateAutomaton *automaton) {
  if (requests != READ_WRITE_NONE || context != NULL || automaton != NULL) { } // prevent 'unused parameter' warning
}

void BLEUI::notifyStatusChange(StatusNotification *notification) {
  if (notification->notifyProperties & NOTIFY_TIME_IN_STATE) {
    ble.setGattCharacteristicValue(timeInStateCharId, notification->timeInState);
  }
  if (notification->notifyProperties & NOTIFY_TIME_HEATING) {
    ble.setGattCharacteristicValue(timeHeatingCharId, notification->heatingTime);
  }
  if (notification->notifyProperties & NOTIFY_STATE) {
    ble.setGattCharacteristicValue(stateCharId, notification->state);
  }
  if (notification->notifyProperties & NOTIFY_WATER_SENSOR) {
    int32_t waterSensor = (notification->waterTemp << 8) | notification->waterSensorStatus;
    ble.setGattCharacteristicValue(waterSensorCharId, waterSensor);
  }
  if (notification->notifyProperties & NOTIFY_AMBIENT_SENSOR) {
    int32_t ambientSensor = (notification->ambientTemp << 8) | notification->ambientSensorStatus;
    ble.setGattCharacteristicValue(ambientSensorCharId, ambientSensor);
  }
}


void BLEUI::notifyNewLogEntry(LogEntry entry) {
  if (entry.timestamp != UNDEFINED_TIMESTAMP) { } // prevent 'unused parameter' warning
}

