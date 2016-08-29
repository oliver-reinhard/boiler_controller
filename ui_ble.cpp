#include "ui_ble.h"

#define DEBUG_BLE_MODULE false

#define DEBUG_BLE_UI

const char BC_DEVICE_NAME[] = "Boiler Controller";
const uint8_t BC_CONTROLLER_SERVICE_ID[] = { 0x4c, 0xef, 0xdd, 0x58, 0xcb, 0x95, 0x44, 0x50, 0x90, 0xfb, 0xf4, 0x04, 0xdc, 0x20, 0x2f, 0x7c};
const int8_t USER_CMD_MAX_SIZE = sizeof(UserCommandID) + USER_CMD_PARAMETER_MAX_SIZE;


void BLEUI::setup() {
  ASSERT(ble.begin(DEBUG_BLE_MODULE), "Couldn't find Bluefruit, make sure it's in CMD mode & check wiring");

  /* Perform a factory reset to make sure everything is in a known state */
  ASSERT(ble.factoryReset(), "Could not factory reset");

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  /* Print Bluefruit information */
  ble.info();

  // this line is particularly required for Flora, but is a good idea anyways for the super long lines ahead!
  // ble.setInterCharWriteDelay(5); // 5 ms

  setDeviceName(BC_DEVICE_NAME);

  // service
  controllerSID =           gatt.addService((uint8_t *)BC_CONTROLLER_SERVICE_ID);

  // status
  stateCID =                gatt.addCharacteristic(0x0001, GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_NOTIFY, sizeof(StateID), sizeof(StateID), BLE_DATATYPE_AUTO);
  timeInStateCID =          gatt.addCharacteristic(0x0002, GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_NOTIFY, 4, 4, BLE_DATATYPE_AUTO);  // milliseconds
  timeHeatingCID =          gatt.addCharacteristic(0x0003, GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_NOTIFY, 4, 4, BLE_DATATYPE_AUTO);  // milliseconds
  acceptedUserCommandsCID = gatt.addCharacteristic(0x0004, GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_NOTIFY, sizeof(UserCommands), sizeof(UserCommands), BLE_DATATYPE_AUTO);
  userRequestCID =          gatt.addCharacteristic(0x0005, GATT_CHARS_PROPERTIES_WRITE,  sizeof(UserCommandID), USER_CMD_MAX_SIZE, BLE_DATATYPE_AUTO); 
  waterSensorCID =          gatt.addCharacteristic(0x0006, GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_NOTIFY, 4, 4, BLE_DATATYPE_AUTO);
  ambientSensorCID =        gatt.addCharacteristic(0x0007, GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_NOTIFY, 4, 4, BLE_DATATYPE_AUTO);
  
  // configuration
  configChangedCID =        gatt.addCharacteristic(0x1000, GATT_CHARS_PROPERTIES_WRITE,  4, 4, BLE_DATATYPE_AUTO); // a random number; its change indicates that one of the config params was changed
  targetTempCID =           gatt.addCharacteristic(0x1001, GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_WRITE,  sizeof(Temperature), sizeof(Temperature), BLE_DATATYPE_AUTO);
  
  // logs
  logEntryCID =             gatt.addCharacteristic(0x2000, GATT_CHARS_PROPERTIES_NOTIFY, sizeof(LogEntry), sizeof(LogEntry), BLE_DATATYPE_AUTO);

  ASSERT(waterSensorCID > 0, "waterSensorCID undefined");
  
  ///* Add the Heart Rate Service to the advertising data (needed for Nordic apps to detect the service) */
  //ASSERT(ble.sendCommandCheckOK( F("AT+GAPSETADVDATA=02-01-06-05-02-0d-18-0a-18")), F("Could not set advertising data"));

  /* Reset the device for the new service setting changes to take effect */
  ble.reset();
}

void BLEUI::readUserRequest() {
  byte cmd[USER_CMD_MAX_SIZE];
  uint16_t len;
  gatt.getChar(userRequestCID, cmd, USER_CMD_MAX_SIZE);
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
    gatt.setChar(userRequestCID, (uint32_t)CMD_NONE);
    
    #ifdef DEBUG_BLE_UI
      Serial.print(F("DEBUG_BLE_UI: user request received: "));
      Serial.println(cmdId);
    #endif
    
  } else {
    uint32_t configChangedStamp = gatt.getCharInt32(configChangedCID);
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

void BLEUI::setDeviceName(const char *name) {
  ble.print( F("AT+GAPDEVNAME="));
  ble.println(name); // execute command
  ASSERT(ble.waitForOK(), "set device name");
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
    gatt.setChar(stateCID,(uint32_t) notification->state);
    gatt.setChar(acceptedUserCommandsCID, (uint32_t)notification->acceptedUserCommands);
    notified = true;
  }
  if (notification->notifyProperties & NOTIFY_TIME_IN_STATE) {
    gatt.setChar(timeInStateCID, notification->timeInState);
    notified = true;
  }
  if (notification->notifyProperties & NOTIFY_TIME_HEATING) {
    gatt.setChar(timeHeatingCID, notification->heatingTime);
    notified = true;
  }
  if (notification->notifyProperties & NOTIFY_WATER_SENSOR) {
    int32_t waterSensor = notification->waterTemp;
    waterSensor = (waterSensor << 8) | notification->waterSensorStatus;
    gatt.setChar(waterSensorCID, waterSensor);
    notified = true;
  }
  if (notification->notifyProperties & NOTIFY_AMBIENT_SENSOR) {
    int32_t ambientSensor = notification->ambientTemp;
    ambientSensor = (ambientSensor << 8) | notification->ambientSensorStatus;
    gatt.setChar(ambientSensorCID, ambientSensor);
    notified = true;
  }
  if (notified) {
    #ifdef DEBUG_BLE_UI
      Serial.println(F("DEBUG_BLE_UI: status notified via BLE"));
    #endif
  }
}


void BLEUI::notifyNewLogEntry(LogEntry entry) {
  gatt.setChar(logEntryCID, (byte *) &entry, sizeof(LogEntry));
  #ifdef DEBUG_BLE_UI
    Serial.println(F("DEBUG_BLE_UI: new log entry notified via BLE"));
  #endif
}
