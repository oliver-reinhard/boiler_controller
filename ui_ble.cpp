#include "ui_ble.h"

#define DEBUG_BLE_MODULE false

//#define DEBUG_BLE_UI

const char BC_DEVICE_NAME[] = "Boiler Controller";

const uint8_t BC_CONTROLLER_SERVICE_UUID128[] = { 0x4c, 0xef, 0xdd, 0x58, 0xcb, 0x95, 0x44, 0x50, 0x90, 0xfb, 0xf4, 0x04, 0xdc, 0x20, 0x2f, 0x7c};
///const uint16_t BC_CONTROLLER_SERVICE_SHORT_UUID16 = 0x4cef;

const int8_t USER_CMD_MAX_SIZE = sizeof(UserCommandID) + USER_CMD_PARAMETER_MAX_SIZE;

/** Service ID */
const int8_t CONTROLLER_SID = 1;

/* Status Characteristics IDs */
const int8_t STATE_CID = 1;
const int8_t TIME_IN_STATE_CID = 2;
const int8_t TIME_HEATING_CID = 3;
const int8_t TIME_TO_GO_CID = 4;
const int8_t ACCEPTED_USER_CMDS_CID = 5;
const int8_t USER_REQUEST_CID = 6;
const int8_t WATER_SENSOR_CID = 7;
const int8_t AMBIENT_SENSOR_CID = 8;

/* Configuration Characteristics IDs */
const int8_t TARGET_TEMP_CID = 9;

 /* Log Characteristics IDs */
const int8_t LOG_ENTRY_CID = 10;
  
/* Status Characteristics */
const char STR_SVC_CONTROLLER[]           PROGMEM = "Controller";

/* Status Characteristics */
const char STR_CHAR_STATE[]               PROGMEM = "State";
const char STR_CHAR_TIME_IN_STATE[]       PROGMEM = "Time in State";
const char STR_CHAR_TIME_HEATING[]        PROGMEM = "Time Heating";
const char STR_CHAR_ACCEPTED_USER_CMDS[]  PROGMEM = "Accepted User Cmds";
const char STR_CHAR_USER_REQUEST[]        PROGMEM = "User Request";
const char STR_CHAR_WATER_SENSOR[]        PROGMEM = "Water Temp";
const char STR_CHAR_AMBIENT_SENSOR[]      PROGMEM = "Ambient Temp";
const char STR_TIME_TO_GO[]               PROGMEM = "Time to Go";

/* Configuration Characteristics */
const char STR_CHAR_TARGET_TEMP[]         PROGMEM = "Target Temp";

 /* Log Characteristics */
const char STR_CHAR_LOG_ENTRY[]           PROGMEM = "Log Entry";

static ExecutionContext *bleContext;

/*
 * CALLBACKS
 */
void deviceConnected(void) {
  #ifdef DEBUG_BLE_UI
    Serial.println(F("DEBUG_BLE_UI: Device connected"));
  #endif
}

void deviceDisconnected(void) {
  #ifdef DEBUG_BLE_UI
    Serial.println(F("DEBUG_BLE_UI: Device disconnected"));
  #endif
}

void bleGattRX(int32_t cid, uint8_t data[], uint16_t /*len*/) {
  #ifdef DEBUG_BLE_UI
    Serial.print( F("DEBUG_BLE_UI: Callback for "));
    Serial.print(cid);
    Serial.print(F(", len = "));
    Serial.print(len);
  #endif

  switch(cid) {
    case USER_REQUEST_CID: 
      {
        UserCommandID cmd;
        memcpy(&cmd, data, sizeof(UserCommandID));
        #ifdef DEBUG_BLE_UI
          Serial.print(", cmd = ");
          Serial.println(cmd);
        #endif
        bleContext->op->request.setCommand(cmd);
      }
      break;
    case TARGET_TEMP_CID:
      {
        Temperature targetTemp;
        memcpy(&targetTemp, data, sizeof(Temperature));
        #ifdef DEBUG_BLE_UI
          Serial.print(", target Temp = ");
          Serial.println(targetTemp);
        #endif
        bleContext->op->request.setParamValue(PARAM_TARGET_TEMP, (int32_t) targetTemp);
      }
      break;
    default:
      // ignore
      #ifdef DEBUG_BLE_UI
        Serial.println(" ... IGNORED");
      #endif
      break;
  }
}


/*
 * BLEUI
 */
void BLEUI::setDeviceName(const char *name) {
  ble.print( F("AT+GAPDEVNAME="));
  ble.println(name); // execute command
  ASSERT(ble.waitForOK(), "set device name");
}

void BLEUI::addServiceChecked(const uint8_t uuid128[], const uint8_t sid, PGM_P description, uint16_t line) {
  uint8_t returnedSid = gatt.addService((uint8_t *) uuid128);
  returnedSid == sid ? (void)0 : write_S_O_S((reinterpret_cast<const __FlashStringHelper *>(description)), line);
}

void BLEUI::addCharacteristicChecked(const uint16_t uuid16, const uint8_t cid, uint8_t properties, uint8_t minLen, uint8_t maxLen, BLEDataType_t datatype, PGM_P description, uint16_t line) {
  char buf[20];
  strncpy_P(buf, description, sizeof(buf));
  buf[sizeof(buf)-1] = '\0';
  uint8_t returnedCid = gatt.addCharacteristic(uuid16, properties, minLen, maxLen, datatype, buf);
  //Serial.print("Expected cid="); Serial.print(cid);
  //Serial.print(", returned cid="); Serial.println(returnedCid);
  // Check –– write_S_O_S WILL NEVER RETURN !
  returnedCid == cid ? (void)0 : write_S_O_S((reinterpret_cast<const __FlashStringHelper *>(description)), line);
}

void BLEUI::setup() {
  
  bleContext = context; // this is a sin ... but the callbacks are static and global
  
  ASSERT(ble.begin(DEBUG_BLE_MODULE), "Couldn't find Bluefruit, make sure it's in CMD mode & check wiring");

  /* Perform a factory reset to make sure everything is in a known state */
  ASSERT(ble.factoryReset(), "Could not factory reset");

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  #ifdef DEBUG_BLE_UI
    /* Print Bluefruit information */
    ble.info();
  #endif

  // this line is particularly required for Flora, but is a good idea anyways for the super long lines ahead!
  // ble.setInterCharWriteDelay(5); // 5 ms

  setDeviceName(BC_DEVICE_NAME);

  // service
  addServiceChecked(BC_CONTROLLER_SERVICE_UUID128, CONTROLLER_SID, STR_SVC_CONTROLLER, __LINE__);

  // status
  addCharacteristicChecked(0x0001, STATE_CID, GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_NOTIFY, sizeof(StateID), sizeof(StateID), BLE_DATATYPE_AUTO, STR_CHAR_STATE, __LINE__);
  addCharacteristicChecked(0x0002, TIME_IN_STATE_CID, GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_NOTIFY, 4, 4, BLE_DATATYPE_AUTO, STR_CHAR_TIME_IN_STATE, __LINE__);  // milliseconds
  addCharacteristicChecked(0x0003, TIME_HEATING_CID, GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_NOTIFY, 4, 4, BLE_DATATYPE_AUTO, STR_CHAR_TIME_HEATING, __LINE__);  // milliseconds
  addCharacteristicChecked(0x0004, TIME_TO_GO_CID, GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_NOTIFY, 4, 4, BLE_DATATYPE_AUTO, STR_TIME_TO_GO, __LINE__);
  addCharacteristicChecked(0x0005, ACCEPTED_USER_CMDS_CID, GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_NOTIFY, sizeof(UserCommands), sizeof(UserCommands), BLE_DATATYPE_AUTO, STR_CHAR_ACCEPTED_USER_CMDS, __LINE__);
  addCharacteristicChecked(0x0006, USER_REQUEST_CID, GATT_CHARS_PROPERTIES_WRITE,  sizeof(UserCommandID), USER_CMD_MAX_SIZE, BLE_DATATYPE_AUTO, STR_CHAR_USER_REQUEST, __LINE__); 
  addCharacteristicChecked(0x0007, WATER_SENSOR_CID, GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_NOTIFY, 4, 4, BLE_DATATYPE_AUTO, STR_CHAR_WATER_SENSOR, __LINE__);
  addCharacteristicChecked(0x0008, AMBIENT_SENSOR_CID, GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_NOTIFY, 4, 4, BLE_DATATYPE_AUTO, STR_CHAR_AMBIENT_SENSOR, __LINE__);
  
  // configuration
  addCharacteristicChecked(0x1000, TARGET_TEMP_CID, GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_WRITE,  sizeof(Temperature), sizeof(Temperature), BLE_DATATYPE_AUTO, STR_CHAR_TARGET_TEMP, __LINE__);
  gatt.setChar(TARGET_TEMP_CID, context->config->targetTemp);
  
  // logs
  addCharacteristicChecked(0x2000, LOG_ENTRY_CID, GATT_CHARS_PROPERTIES_NOTIFY, sizeof(LogEntry), sizeof(LogEntry), BLE_DATATYPE_AUTO, STR_CHAR_LOG_ENTRY, __LINE__);
  
  //uint8_t advdata[] { 0x02, 0x01, 0x06, 0x05, 0x02, 0x09, 0x18, 0x0a, 0x18 };
  uint8_t advdata[] { 0x02, 0x01, 0x06, 
    0x03, 0x02, BC_CONTROLLER_SERVICE_UUID128[1], BC_CONTROLLER_SERVICE_UUID128[0]  // 0x4c, 0xef /////// 03 = # bytes, 02 = 16-bit UUID, 0xef4c = UUID => use type for 128-bit UUID !!!!!!!
  };
  ble.setAdvData( advdata, sizeof(advdata) );
  
  /* Reset the device for the new service setting changes to take effect */
  ble.reset();
  
  ble.setConnectCallback(deviceConnected);
  ble.setDisconnectCallback(deviceDisconnected);
  ble.setBleGattRxCallback(USER_REQUEST_CID, bleGattRX);
  ble.setBleGattRxCallback(TARGET_TEMP_CID, bleGattRX);
}

void BLEUI::readUserRequest() {
  ble.update(100); // ms
}


void BLEUI::provideUserInfo(BoilerStateAutomaton *automaton) {
  if (automaton != NULL) { } // prevent 'unused parameter' warning
  /*
   * ***** TODO ******
   */
}

void BLEUI::commandExecuted(boolean success) { 
  /*
   * ***** TODO ******
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
    gatt.setChar(STATE_CID,notification->state);
    gatt.setChar(ACCEPTED_USER_CMDS_CID, notification->acceptedUserCommands);
    notified = true;
  }
  if (notification->notifyProperties & NOTIFY_TIME_IN_STATE) {
    gatt.setChar(TIME_IN_STATE_CID, notification->timeInState);
    notified = true;
  }
  if (notification->notifyProperties & NOTIFY_TIME_HEATING) {
    gatt.setChar(TIME_HEATING_CID, notification->heatingTime);
    notified = true;
  }
  if (notification->notifyProperties & NOTIFY_TIME_TO_GO) {
    gatt.setChar(TIME_TO_GO_CID, notification->timeToGo);
    notified = true;
  }
  if (notification->notifyProperties & NOTIFY_WATER_SENSOR) {
    int32_t waterSensor = notification->waterTemp;
    waterSensor = (waterSensor << 8) | notification->waterSensorStatus;
    gatt.setChar(WATER_SENSOR_CID, waterSensor);
    notified = true;
  }
  if (notification->notifyProperties & NOTIFY_AMBIENT_SENSOR) {
    int32_t ambientSensor = notification->ambientTemp;
    ambientSensor = (ambientSensor << 8) | notification->ambientSensorStatus;
    gatt.setChar(AMBIENT_SENSOR_CID, ambientSensor);
    notified = true;
  }
  if (notified) {
    #ifdef DEBUG_BLE_UI
      Serial.println(F("DEBUG_BLE_UI: status notified via BLE"));
    #endif
  }
}


void BLEUI::notifyNewLogEntry(LogEntry entry) {
  gatt.setChar(LOG_ENTRY_CID, (byte *) &entry, sizeof(LogEntry));
  #ifdef DEBUG_BLE_UI
    Serial.println(F("DEBUG_BLE_UI: new log entry notified via BLE"));
  #endif
}
