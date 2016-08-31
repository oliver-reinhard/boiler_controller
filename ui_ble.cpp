#include "ui_ble.h"

#define DEBUG_BLE_MODULE false

#define DEBUG_BLE_UI

const char BC_DEVICE_NAME[] = "Boiler Controller";
const uint16_t BC_CONTROLLER_SERVICE_ID[] = { 0x4c, 0xef, 0xdd, 0x58, 0xcb, 0x95, 0x44, 0x50, 0x90, 0xfb, 0xf4, 0x04, 0xdc, 0x20, 0x2f, 0x7c};
const int8_t USER_CMD_MAX_SIZE = sizeof(UserCommandID) + USER_CMD_PARAMETER_MAX_SIZE;


/* Status Characteristics */
const char STR_SVC_CONTROLLER[]           PROGMEM = "Controller";

/* Status Characteristics */
const char STR_CHAR_STATE[]               PROGMEM = "State";
const char STR_CHAR_TIME_IN_STATE[]       PROGMEM = "Tine in State";
const char STR_CHAR_TIME_HEATING[]        PROGMEM = "Time Heating";
const char STR_CHAR_ACCEPTED_USER_CMDS[]  PROGMEM = "Accepted User Cmds";
const char STR_CHAR_USER_REQUEST[]        PROGMEM = "User Request";
const char STR_CHAR_WATER_SENSOR[]        PROGMEM = "Water Temp";
const char STR_CHAR_AMBIENT_SENSOR[]      PROGMEM = "Ambient Temp";

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
    Serial.print(F("DEBUG_BLE_UI: Device connected"));
  #endif
}

void deviceDisconnected(void) {
  #ifdef DEBUG_BLE_UI
    Serial.print(F("DEBUG_BLE_UI: Device disconnected"));
  #endif
}

void bleGattRX(int32_t cid, uint8_t data[], uint16_t len) {
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
        bleContext->op->request.command = (UserCommandEnum)cmd; /// !!!!!!!! USER INPUT : CHECK BEFORE CONVERSION !!!!!!!!
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
        bleContext->op->request.command = CMD_CONFIG_SET_VALUE;
        bleContext->op->request.param = PARAM_TARGET_TEMP;
        bleContext->op->request.intValue = targetTemp;
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


void BLEUI::addServiceChecked(const uint16_t uuid128[], const uint8_t sid, PGM_P description, uint16_t line) {
  uint8_t returnedSid = gatt.addService((uint16_t) uuid128);
  returnedSid == sid ? (void)0 : write_S_O_S((reinterpret_cast<const __FlashStringHelper *>(description)), line);
}


void BLEUI::addCharacteristicChecked(const uint16_t uuid16, const uint8_t cid, uint8_t properties, uint8_t minLen, uint8_t maxLen, BLEDataType_t datatype, PGM_P description, uint16_t line) {
  char buf[20];
  strncpy_P(buf, description, sizeof(buf));
  buf[sizeof(buf)-1] = '\0';
  uint8_t returnedCid = gatt.addCharacteristic(uuid16, properties, minLen, maxLen, datatype, buf);
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

  /* Print Bluefruit information */
  ble.info();

  // this line is particularly required for Flora, but is a good idea anyways for the super long lines ahead!
  // ble.setInterCharWriteDelay(5); // 5 ms

  setDeviceName(BC_DEVICE_NAME);

  // service
  addServiceChecked(BC_CONTROLLER_SERVICE_ID, CONTROLLER_SID, STR_SVC_CONTROLLER, __LINE__);

  // status
  addCharacteristicChecked(0x0001, STATE_CID, GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_NOTIFY, sizeof(StateID), sizeof(StateID), BLE_DATATYPE_AUTO, STR_CHAR_STATE, __LINE__);
  addCharacteristicChecked(0x0002, TIME_IN_STATE_CID, GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_NOTIFY, 4, 4, BLE_DATATYPE_AUTO, STR_CHAR_TIME_IN_STATE, __LINE__);  // milliseconds
  addCharacteristicChecked(0x0003, TIME_HEATING_CID, GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_NOTIFY, 4, 4, BLE_DATATYPE_AUTO, STR_CHAR_TIME_HEATING, __LINE__);  // milliseconds
  addCharacteristicChecked(0x0004, ACCEPTED_USER_CMDS_CID, GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_NOTIFY, sizeof(UserCommands), sizeof(UserCommands), BLE_DATATYPE_AUTO, STR_CHAR_ACCEPTED_USER_CMDS, __LINE__);
  addCharacteristicChecked(0x0005, USER_REQUEST_CID, GATT_CHARS_PROPERTIES_WRITE,  sizeof(UserCommandID), USER_CMD_MAX_SIZE, BLE_DATATYPE_AUTO, STR_CHAR_USER_REQUEST, __LINE__); 
  addCharacteristicChecked(0x0006, WATER_SENSOR_CID, GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_NOTIFY, 4, 4, BLE_DATATYPE_AUTO, STR_CHAR_WATER_SENSOR, __LINE__);
  addCharacteristicChecked(0x0007, AMBIENT_SENSOR_CID, GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_NOTIFY, 4, 4, BLE_DATATYPE_AUTO, STR_CHAR_AMBIENT_SENSOR, __LINE__);
  
  // configuration
  addCharacteristicChecked(0x1000, TARGET_TEMP_CID, GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_WRITE,  sizeof(Temperature), sizeof(Temperature), BLE_DATATYPE_AUTO, STR_CHAR_TARGET_TEMP, __LINE__);
  
  // logs
  addCharacteristicChecked(0x2000, LOG_ENTRY_CID, GATT_CHARS_PROPERTIES_NOTIFY, sizeof(LogEntry), sizeof(LogEntry), BLE_DATATYPE_AUTO, STR_CHAR_LOG_ENTRY, __LINE__);
  
  /* Reset the device for the new service setting changes to take effect */
  ble.reset();

  
  ble.setConnectCallback(deviceConnected);
  ble.setDisconnectCallback(deviceDisconnected);
  ble.setBleGattRxCallback(USER_REQUEST_CID, bleGattRX);
  ble.setBleGattRxCallback(TARGET_TEMP_CID, bleGattRX);
}

void BLEUI::readUserRequest() {
  ble.update(100); // ms
  /*
  byte cmd[USER_CMD_MAX_SIZE];
  uint16_t len;
  gatt.getChar(USER_REQUEST_CID, cmd, USER_CMD_MAX_SIZE);
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
    gatt.setChar(USER_REQUEST_CID, (uint32_t)CMD_NONE);
    
    #ifdef DEBUG_BLE_UI
      Serial.print(F("DEBUG_BLE_UI: user request received: "));
      Serial.println(cmdId);
    #endif
  }
  */
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
