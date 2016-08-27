#include "ui_ble.h"

#define DEBUG_BLE_MODULE false

#define DEBUG_BLE_UI

const char BC_DEVICE_NAME[] = "Boiler Controller";
const char BC_CONTROLLER_SERVICE_ID[] = "4C-EF-DD-58-CB-95-44-50-90-FB-F4-04-DC-20-2F-7C";

const int8_t USER_CMD_MAX_SIZE = sizeof(UserCommandID) + USER_CMD_PARAMETER_MAX_SIZE;

void BLEUI::setup() {
  ble.assertOK(ble.begin(DEBUG_BLE_MODULE), F("Couldn't find Bluefruit, make sure it's in CMD mode & check wiring?"));

  /* Perform a factory reset to make sure everything is in a known state */
  ble.assertOK(ble.factoryReset(), F("Could not factory reset"));

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  /* Print Bluefruit information */
  ble.info();

  // this line is particularly required for Flora, but is a good idea anyways for the super long lines ahead!
  // ble.setInterCharWriteDelay(5); // 5 ms

  ble.setGattDeviceName(BC_DEVICE_NAME);

  // service
  controllerSID =           ble.addGattService(BC_CONTROLLER_SERVICE_ID);

  // status
  stateCID =                ble.addGattCharacteristic(0x0001, CHAR_PROP_READ | CHAR_PROP_NOTIFY, sizeof(StateID), sizeof(StateID));
  timeInStateCID =          ble.addGattCharacteristic(0x0002, CHAR_PROP_READ | CHAR_PROP_NOTIFY, 4, 4);  // milliseconds
  timeHeatingCID =          ble.addGattCharacteristic(0x0003, CHAR_PROP_READ | CHAR_PROP_NOTIFY, 4, 4);  // milliseconds
  acceptedUserCommandsCID = ble.addGattCharacteristic(0x0004, CHAR_PROP_READ | CHAR_PROP_NOTIFY, sizeof(UserCommands), sizeof(UserCommands));
  userRequestCID =          ble.addGattCharacteristic(0x0005, CHAR_PROP_WRITE,  sizeof(UserCommandID), USER_CMD_MAX_SIZE); 
  waterSensorCID =          ble.addGattCharacteristic(0x0006, CHAR_PROP_READ | CHAR_PROP_NOTIFY, 4, 4);
  ambientSensorCID =        ble.addGattCharacteristic(0x0007, CHAR_PROP_READ | CHAR_PROP_NOTIFY, 4, 4);
  
  // configuration
  configChangedCID =        ble.addGattCharacteristic(0x1000, CHAR_PROP_WRITE,  4, 4); // a random number; its change indicates that one of the config params was changed
  targetTempCID =           ble.addGattCharacteristic(0x1001, CHAR_PROP_READ | CHAR_PROP_WRITE,  sizeof(Temperature), sizeof(Temperature));
  
  // logs
  logEntryCID =             ble.addGattCharacteristic(0x2000, CHAR_PROP_NOTIFY, sizeof(LogEntry), sizeof(LogEntry));

  ble.assertOK(waterSensorCID > 0, F("waterSensorCID undefined"));
  
  /* Add the Heart Rate Service to the advertising data (needed for Nordic apps to detect the service) */
  ble.assertOK(ble.sendCommandCheckOK( F("AT+GAPSETADVDATA=02-01-06-05-02-0d-18-0a-18")), F("Could not set advertising data"));

  /* Reset the device for the new service setting changes to take effect */
  ble.reset();
}

void BLEUI::readUserRequest() {
  byte cmd[USER_CMD_MAX_SIZE];
  uint16_t len = ble.getGattCharacteristicValue(userRequestCID, cmd, USER_CMD_MAX_SIZE);
  UserCommandID cmdId;
  memcpy(&cmdId, cmd, sizeof(UserCommandID));
  if (cmdId != CMD_NONE) {
    context->op->request.command = (UserCommandEnum) cmdId;

    if (len > sizeof(UserCommandID)) {
      // 
      // TODO get params and store somehow as context->op->request->args
      //
    }

    // reset characteristic value to CMD_NONE:
    ble.setGattCharacteristicValue(userRequestCID, (uint32_t)CMD_NONE);
    
    #ifdef DEBUG_BLE_UI
      Serial.print(F("DEBUG_BLE_UI: user request received: "));
      Serial.println(cmdId);
    #endif
    
  } else {
    uint32_t configChangedStamp;
    ble.getGattCharacteristicValue(configChangedCID, &configChangedStamp);
    if (configChangedStamp != lastConfigChangedStamp) {
      // 
      // TODO check all config characteristics and store new value in config
      //

      // store new stamp:
      lastConfigChangedStamp = configChangedStamp;

      #ifdef DEBUG_BLE_UI
        Serial.print(F("DEBUG_BLE_UI: new config-changed stamp: "));
        Serial.println(configChangedStamp);
      #endif
    }
  }
}

void BLEUI::provideUserInfo(BoilerStateAutomaton *automaton) {
  if (automaton != NULL) { } // prevent 'unused parameter' warning
  /*
   * ***** TODO ******3
   */
}

void BLEUI::commandExecuted(boolean success) { 
  /*
   * ***** TODO ******3
   */
  if (success) {
    Serial.println(F("* command ok."));
  } else {
    Serial.println(F("* command failed."));
  }
}
      
void BLEUI::notifyStatusChange(StatusNotification *notification) {
  boolean notified = false;

  if (notification->notifyProperties & NOTIFY_STATE) {
    ble.setGattCharacteristicValue(stateCID,(uint32_t) notification->state);
    ble.setGattCharacteristicValue(acceptedUserCommandsCID, (uint32_t)notification->acceptedUserCommands);
    notified = true;
  }
  if (notification->notifyProperties & NOTIFY_TIME_IN_STATE) {
    ble.setGattCharacteristicValue(timeInStateCID, notification->timeInState);
    notified = true;
  }
  if (notification->notifyProperties & NOTIFY_TIME_HEATING) {
    ble.setGattCharacteristicValue(timeHeatingCID, notification->heatingTime);
    notified = true;
  }
  if (notification->notifyProperties & NOTIFY_WATER_SENSOR) {
    int32_t waterSensor = notification->waterTemp;
    waterSensor = (waterSensor << 8) | notification->waterSensorStatus;
    ble.setGattCharacteristicValue(waterSensorCID, waterSensor);
    notified = true;
  }
  if (notification->notifyProperties & NOTIFY_AMBIENT_SENSOR) {
    int32_t ambientSensor = notification->ambientTemp;
    ambientSensor = (ambientSensor << 8) | notification->ambientSensorStatus;
    ble.setGattCharacteristicValue(ambientSensorCID, ambientSensor);
    notified = true;
  }
  if (notified) {
    #ifdef DEBUG_BLE_UI
      Serial.println(F("DEBUG_BLE_UI: status notified via BLE"));
    #endif
  }
}


void BLEUI::notifyNewLogEntry(LogEntry entry) {
  ble.setGattCharacteristicValue(logEntryCID, (byte *) &entry, sizeof(LogEntry));
  #ifdef DEBUG_BLE_UI
    Serial.println(F("DEBUG_BLE_UI: new log entry notified via BLE"));
  #endif
}

