#include <string.h>
#include "ui.h"
#include "config.h"
#include "storage.h"

#define COMMAND_BUF_SIZE 50   // Size of the read buffer for incoming data
const char COMMAND_CHARS[] = " abcdefghijklmnopqrstuvwxyz"; // includes blank (first char)
const char INT_CHARS[] = "0123456789"; 
const char SENSOR_ID_CHARS[] = "0123456789abcdef-"; // includes dash

//#define DEBUG_UI

/*
 * CONFIG PARAMS
 */
const char STR_PARAM_TARGET_TEMP[] PROGMEM = "Target Temperature";
const char STR_PARAM_WATER_TEMP_SENSOR_ID[] PROGMEM = "Water Temp. Sensor ID";
const char STR_PARAM_AMBIENT_TEMP_SENSOR_ID[] PROGMEM = "Ambient Temp. Sensor ID";
const char STR_PARAM_HEATER_CUT_OUT_WATER_TEMP[] PROGMEM = "Heater Cut-out Temp.";
const char STR_PARAM_HEATER_BACK_OK_WATER_TEMP[] PROGMEM = "Heater Back-ok Temp.";
const char STR_PARAM_LOG_TEMP_DELTA[] PROGMEM = "Log Temp. Delta";
const char STR_PARAM_LOG_TIME_DELTA[] PROGMEM = "Log Time Delta [s]";
const char STR_PARAM_TANK_CAPACITY[] PROGMEM = "Tank Capacity [ml]";
const char STR_PARAM_HEATER_POWER[] PROGMEM = "Heater Power [W]";
const char STR_PARAM_INSULATION_FACTOR[] PROGMEM = "Insulation Factor";
const char STR_PARAM_UNDEF[] PROGMEM = "Undefined Config Param";

PGM_P getConfigParamNamePtr(ConfigParamEnum literal) {
  switch(literal) {
    case PARAM_TARGET_TEMP: return STR_PARAM_TARGET_TEMP;
    case PARAM_WATER_TEMP_SENSOR_ID: return STR_PARAM_WATER_TEMP_SENSOR_ID;
    case PARAM_AMBIENT_TEMP_SENSOR_ID: return STR_PARAM_AMBIENT_TEMP_SENSOR_ID;
    case PARAM_HEATER_CUT_OUT_WATER_TEMP: return STR_PARAM_HEATER_CUT_OUT_WATER_TEMP;
    case PARAM_HEATER_BACK_OK_WATER_TEMP: return STR_PARAM_HEATER_BACK_OK_WATER_TEMP;
    case PARAM_LOG_TEMP_DELTA: return STR_PARAM_LOG_TEMP_DELTA;
    case PARAM_LOG_TIME_DELTA: return STR_PARAM_LOG_TIME_DELTA;
    case PARAM_TANK_CAPACITY: return STR_PARAM_TANK_CAPACITY;
    case PARAM_HEATER_POWER: return STR_PARAM_HEATER_POWER;
    case PARAM_INSULATION_FACTOR: return STR_PARAM_INSULATION_FACTOR;
    default: return STR_PARAM_UNDEF;
  }
}

String getConfigParamName(ConfigParamEnum literal) {
  char buf[30];
  strcpy_P(buf, getConfigParamNamePtr(literal));
  return buf;
}

String formatFloat(float f) {
  char s[20];
  dtostrf((double)f, 6, 2, &s[0]); 
  return s;
}

String formatInt(unsigned short i) {
  char s[20];
  utoa(i, &s[0], 10);
  return s;
}

String getConfigParamValue(ConfigParams *all, ConfigParamEnum p) {
  switch(p) {
    case PARAM_TARGET_TEMP: 
      return formatTemperature(all->targetTemp);
    case PARAM_WATER_TEMP_SENSOR_ID: 
      return formatTempSensorID(all->waterTempSensorId);
    case PARAM_AMBIENT_TEMP_SENSOR_ID: 
      return formatTempSensorID(all->ambientTempSensorId);
    case PARAM_HEATER_CUT_OUT_WATER_TEMP: 
      return formatTemperature(all->heaterCutOutWaterTemp);
    case PARAM_HEATER_BACK_OK_WATER_TEMP: 
      return formatTemperature(all->heaterBackOkWaterTemp);
    case PARAM_LOG_TEMP_DELTA: 
      return formatTemperature(all->logTempDelta);
    case PARAM_LOG_TIME_DELTA:
      return formatInt(all->logTimeDelta);
    case PARAM_TANK_CAPACITY:
      return formatFloat(all->tankCapacity);
    case PARAM_HEATER_POWER:
      return formatFloat(all->heaterPower);
    case PARAM_INSULATION_FACTOR:
      return formatFloat(all->insulationFactor);
    default: 
      return "";
  }
}

boolean parseTempSensorID(char *value, byte *id) {
  byte len = strspn(value, SENSOR_ID_CHARS);
  if (len == 3 * TEMP_SENSOR_ID_BYTES - 1) {
    byte pos = 0;
    for (byte i=0; i<TEMP_SENSOR_ID_BYTES; i++) {
      long n = strtol(&value[pos], NULL, 16);
      id[i] = (byte) n;
      pos += 3;
    }
    return true;
  } else {
    Serial.println("Invalid sensor ID");
    return false;
  }
}

boolean setConfigParamValue(ControlContext *context, ConfigParamEnum p, char *value) {
  switch(p) {
    case PARAM_TARGET_TEMP: 
      context->config->targetTemp = atoi(value);
      context->storage->logConfigParam(p, (float) context->config->targetTemp);
      return true;
    case PARAM_WATER_TEMP_SENSOR_ID:
      if (parseTempSensorID(value, &(context->config->waterTempSensorId[0]))) {
        context->storage->logConfigParam(p, 0.0);
        return true;
      }
      return false;
    case PARAM_AMBIENT_TEMP_SENSOR_ID:
      if (parseTempSensorID(value, &(context->config->ambientTempSensorId[0]))) {
        context->storage->logConfigParam(p, 0.0);
        return true;
      }
      return false;
    case PARAM_HEATER_CUT_OUT_WATER_TEMP: 
      context->config->heaterCutOutWaterTemp = atoi(value);
      context->storage->logConfigParam(p, (float) context->config->heaterCutOutWaterTemp);
      return true;
    case PARAM_HEATER_BACK_OK_WATER_TEMP: 
      context->config->heaterBackOkWaterTemp = atoi(value);
      context->storage->logConfigParam(p, (float) context->config->heaterBackOkWaterTemp);
      return true;
    case PARAM_LOG_TEMP_DELTA: 
      context->config->logTempDelta = atoi(value);
      context->storage->logConfigParam(p, (float) context->config->logTempDelta);
      return true;
    case PARAM_LOG_TIME_DELTA:
      context->config->logTimeDelta = atoi(value);
      context->storage->logConfigParam(p, (float) context->config->logTimeDelta);
      return true;
    case PARAM_TANK_CAPACITY:
      context->config->tankCapacity = (float) atof(value);
      context->storage->logConfigParam(p, context->config->tankCapacity);
      return true;
    case PARAM_HEATER_POWER:
      context->config->heaterPower = (float) atof(value);
      context->storage->logConfigParam(p, context->config->heaterPower);
      return true;
    case PARAM_INSULATION_FACTOR:
      context->config->insulationFactor = (float) atof(value);
      context->storage->logConfigParam(p, context->config->insulationFactor);
      return true;
    default:
      return false;
  }
}

/*
 * STATES
 */
const char STR_STATE_UNDEFINED[] PROGMEM = "Undefined";
const char STR_STATE_SAME[] PROGMEM = "Same";
const char STR_STATE_INIT[] PROGMEM = "Init";
const char STR_STATE_SENSORS_NOK[] PROGMEM = "Sensors NOK";
const char STR_STATE_READY[] PROGMEM = "Ready";
const char STR_STATE_IDLE[] PROGMEM = "Idle";
const char STR_STATE_RECORDING[] PROGMEM = "Recording";
const char STR_STATE_STANDBY[] PROGMEM = "Standby";
const char STR_STATE_HEATING[] PROGMEM = "Heating";
const char STR_STATE_OVERHEATED[] PROGMEM = "Overheated";
const char STR_STATE_UNDEF[] PROGMEM = "Undefined State";

PGM_P getStateNamePtr(StateEnum literal) {
  switch(literal) {
    case STATE_UNDEFINED: return STR_STATE_UNDEFINED;
    case STATE_SAME: return STR_STATE_SAME;
    case STATE_INIT: return STR_STATE_INIT;
    case STATE_SENSORS_NOK: return STR_STATE_SENSORS_NOK;
    case STATE_READY: return STR_STATE_READY;
    case STATE_IDLE: return STR_STATE_IDLE;
    case STATE_RECORDING: return STR_STATE_RECORDING;
    case STATE_STANDBY: return STR_STATE_STANDBY;
    case STATE_HEATING: return STR_STATE_HEATING;
    case STATE_OVERHEATED: return STR_STATE_OVERHEATED;
    default: return STR_STATE_UNDEF;
  }
}

String getStateName(StateEnum literal) {
  char buf[20];
  strcpy_P(buf, getStateNamePtr(literal));
  return buf;
}

/*
 * EVENTS
 */
const char STR_EVENT_NONE[] PROGMEM = "None";
const char STR_EVENT_READY[] PROGMEM = "Ready";
const char STR_EVENT_SENSORS_NOK[] PROGMEM = "Sensors NOK";
const char STR_EVENT_SET_CONFIG[] PROGMEM = "Set Config";
const char STR_EVENT_REC_ON[] PROGMEM = "Rec On";
const char STR_EVENT_REC_OFF[] PROGMEM = "Rec Off";
const char STR_EVENT_GET_CONFIG[] PROGMEM = "Get Config";
const char STR_EVENT_GET_LOG[] PROGMEM = "Get Log";
const char STR_EVENT_GET_STAT[] PROGMEM = "Get Stat";
const char STR_EVENT_HEAT_ON[] PROGMEM = "Heat On";
const char STR_EVENT_HEAT_OFF[] PROGMEM = "Heat Off";
const char STR_EVENT_TEMP_OVER[] PROGMEM = "Temp Over";
const char STR_EVENT_TEMP_OK[] PROGMEM = "Temp OK";
const char STR_EVENT_RESET[] PROGMEM = "Reset";
const char STR_EVENT_UNDEF[] PROGMEM = "Undefined Event";
    
PGM_P getEventNamePtr(EventEnum literal) {
  switch(literal) {
    case EVENT_NONE: return STR_EVENT_NONE;
    case EVENT_READY: return STR_EVENT_READY;
    case EVENT_SENSORS_NOK: return STR_EVENT_SENSORS_NOK;
    case EVENT_SET_CONFIG: return STR_EVENT_SET_CONFIG;
    case EVENT_REC_ON: return STR_EVENT_REC_ON;
    case EVENT_REC_OFF: return STR_EVENT_REC_OFF;
    case EVENT_GET_CONFIG: return STR_EVENT_GET_CONFIG;
    case EVENT_GET_LOG: return STR_EVENT_GET_LOG;
    case EVENT_GET_STAT: return STR_EVENT_GET_STAT;
    case EVENT_HEAT_ON: return STR_EVENT_HEAT_ON;
    case EVENT_HEAT_OFF: return STR_EVENT_HEAT_OFF;
    case EVENT_TEMP_OVER: return STR_EVENT_TEMP_OVER;
    case EVENT_TEMP_OK: return STR_EVENT_TEMP_OK;
    case EVENT_RESET: return STR_EVENT_RESET;
    default: return STR_EVENT_UNDEF;
  } 
}

String getEventName(EventEnum literal) {
  char buf[20];
  strcpy_P(buf, getEventNamePtr(literal));
  return buf;
}

/*
 * COMMANDS
 */
const char STR_CMD_NONE[] PROGMEM = "None";
const char STR_CMD_SET_CONFIG[] PROGMEM = "set config";
const char STR_CMD_REC_ON[] PROGMEM = "rec on";
const char STR_CMD_REC_OFF[] PROGMEM = "rec off";
const char STR_CMD_HELP[] PROGMEM = "help";
const char STR_CMD_GET_LOG[] PROGMEM = "get log";
const char STR_CMD_GET_CONFIG[] PROGMEM = "get config";
const char STR_CMD_GET_STAT[] PROGMEM = "get stat";
const char STR_CMD_HEAT_ON[] PROGMEM = "heat on";
const char STR_CMD_HEAT_OFF[] PROGMEM = "heat off";
const char STR_CMD_RESET[] PROGMEM = "reset";
const char STR_CMD_UNDEF[] PROGMEM = "Undefined Command";
    
PGM_P getUserCommandNamePtr(UserCommandEnum literal) {
  switch(literal) {
    case CMD_NONE: return STR_CMD_NONE;
    case CMD_SET_CONFIG: return STR_CMD_SET_CONFIG;
    case CMD_REC_ON: return STR_CMD_REC_ON;
    case CMD_REC_OFF: return STR_CMD_REC_OFF;
    case CMD_HELP: return STR_CMD_HELP;
    case CMD_GET_LOG: return STR_CMD_GET_LOG;
    case CMD_GET_CONFIG: return STR_CMD_GET_CONFIG;
    case CMD_GET_STAT: return STR_CMD_GET_STAT;
    case CMD_HEAT_ON: return STR_CMD_HEAT_ON;
    case CMD_HEAT_OFF: return STR_CMD_HEAT_OFF;
    case CMD_RESET: return STR_CMD_RESET;
    default: return STR_CMD_UNDEF;
  }
}

String getUserCommandName(UserCommandEnum literal) {
  char buf[20];
  strcpy_P(buf, getUserCommandNamePtr(literal));
  return buf;
}

/*
 * SENSOR STATUS
 */
const char STR_SENSOR_INITIALISING[] PROGMEM = "Init";
const char STR_SENSOR_OK[] PROGMEM = "OK";
const char STR_SENSOR_NOK[] PROGMEM = "NOK";
const char STR_SENSOR_ID_UNDEFINED[] PROGMEM = "No ID";
const char STR_SENSOR_UNDEF[] PROGMEM = "Undefined Sensor Status";
    
PGM_P getSensorStatusNamePtr(SensorStatusEnum literal) {
  switch(literal) {
    case SENSOR_INITIALISING: return STR_SENSOR_INITIALISING;
    case SENSOR_OK: return STR_SENSOR_OK;
    case SENSOR_NOK: return STR_SENSOR_NOK;
    case SENSOR_ID_UNDEFINED: return STR_SENSOR_ID_UNDEFINED;
    default: return STR_SENSOR_UNDEF;
  }
}

String getSensorStatusName(SensorStatusEnum literal) {
  char buf[24];
  strcpy_P(buf, getSensorStatusNamePtr(literal));
  return buf;
}

/*
 * USER COMMANDS
 */
UserCommandEnum parseUserCommand(char buf[], byte bufSize) {
  switch (bufSize) {
    case 1:
      if (!strcmp_P(buf, "?")) {
        return CMD_HELP;
      } 
      break;
    case 4:
      if (!strcmp_P(buf, STR_CMD_HELP)) {
        return CMD_HELP;
      } 
      break;
    case 5:
      if (!strcmp_P(buf, STR_CMD_RESET)) {
        return CMD_RESET;
      } 
      break;
    case 6:
      if (!strcmp_P(buf, STR_CMD_REC_ON)) {
        return CMD_REC_ON;
      } 
      break;
    case 7:
      if (!strcmp_P(buf, STR_CMD_REC_OFF)) {
        return CMD_REC_OFF;
      } else if (!strcmp_P(buf, STR_CMD_HEAT_ON)) {
        return CMD_HEAT_ON;
      } else if (!strcmp_P(buf, STR_CMD_GET_LOG)) {
        return CMD_GET_LOG;
      }
      break;
    case 8:      
      if (!strcmp_P(buf, STR_CMD_HEAT_OFF)) {
        return CMD_HEAT_OFF;
      } else if (!strcmp_P(buf, STR_CMD_GET_STAT)) {
        return CMD_GET_STAT;
      } 
      break;
    case 10:
      if (!strcmp_P(buf, STR_CMD_SET_CONFIG)) {
        return CMD_SET_CONFIG;
      } if (!strcmp_P(buf, STR_CMD_GET_CONFIG)) {
        return CMD_GET_CONFIG;
      }
      break;
    default:
      break; 
  }
  return CMD_NONE;
}

void printError(String msg) {
  Serial.print(F("Error: "));
  Serial.print(msg);
  Serial.println(F("."));
}

void SerialUI::readUserCommand(ControlContext *context) {
  char buf[COMMAND_BUF_SIZE+1];
  // fill buffer with 0's => always \0-terminated!
  memset(buf, 0, COMMAND_BUF_SIZE);
  if( Serial.peek() < 0 ) {
    return;
  }
  delay(2);

  byte count = 0;
  do {
    count += Serial.readBytes(&buf[count], COMMAND_BUF_SIZE);
    delay(2);
  } while( (count < COMMAND_BUF_SIZE) && !(Serial.peek() < 0) );
  #ifdef DEBUG_UI
    Serial.print(F("DEBUG_UI: read cmd string: '"));
    Serial.print(buf);
    Serial.print(F("', len: "));
    Serial.println(count);
  #endif

  // remove multiple consecutive spaces
  byte len = 0;
  boolean prevSpace = false;
  for (byte i = 0; i< count; i++) {
     if (isspace(buf[i]) && prevSpace) {
        // skip
     } else {
      buf[len++] = buf[i];
     }
     prevSpace = isspace(buf[i]);
  }
  buf[len] = '\0';
  
  // convert to lower case:
  char *lower = strlwr(buf);
  
  // count the command characters up to the trailing numeric arguments (if any):
  byte commandLength = strspn(lower, COMMAND_CHARS);
  if (len > commandLength) {
    strcpy(context->op->command->args, &lower[commandLength]);
  }
  
  if (isspace(lower[commandLength - 1])) {
    commandLength--;
  }
  lower[commandLength] = '\0';
  context->op->command->command = parseUserCommand(buf, commandLength);  
  
  #ifdef DEBUG_UI
    Serial.print(F("DEBUG_UI: parsed cmd: "));
    Serial.print(context->op->command->command, HEX);
    Serial.print(F(": "));
    Serial.print(getUserCommandName((UserCommandEnum) context->op->command->command));
    Serial.print(F(", args: '"));
    Serial.print(context->op->command->args);
    Serial.println(F("'"));
  #endif
  if (context->op->command->command == CMD_NONE) {
    printError("Illegal command (try: help or ?)");
  }
}

void printLogEntry(LogEntry *e) {
  Serial.print(formatTimestamp(e->timestamp));
  Serial.print("  ");
  LogTypeEnum type = (LogTypeEnum)e->type;
  switch (type) {
    case LOG_VALUES:
      {
        LogValuesData data;
        memcpy(&data, &(e->data), sizeof(LogValuesData));
        Serial.print(F("V  water:"));
        Serial.print(getSensorStatusName((SensorStatusEnum)(data.flags>>4)));
        Serial.print(F(" "));
        Serial.print(formatTemperature(data.water));
        Serial.print(F(", ambient:"));
        Serial.print(getSensorStatusName((SensorStatusEnum)(data.flags&0x0F)));
        Serial.print(F(" "));
        Serial.println(formatTemperature(data.ambient));
      }
      break;
      
    case LOG_STATE:
      {
        LogStateData data;
        memcpy(&data, &e->data, sizeof(LogStateData));
        Serial.print(F("S  "));
        Serial.print(getStateName((StateEnum)data.previous));
        Serial.print(F(" -> ["));
        Serial.print(getEventName((EventEnum)data.event));
        Serial.print(F("] -> "));
        Serial.println(getStateName((StateEnum)data.current));
      }
      break;
      
    case LOG_MESSAGE:
      {
        LogMessageData data;
        memcpy(&data, &e->data, sizeof(LogMessageData));
        Serial.print(F("M  msg:"));
        Serial.print(data.id);
        Serial.print(F(", p1:"));
        Serial.print(data.params[0]);
        Serial.print(F(", p2:"));
        Serial.println(data.params[1]);
      }
      break;
      
    case LOG_CONFIG:
      {
        LogConfigParamData data;
        memcpy(&data, &e->data, sizeof(LogConfigParamData));
        Serial.print(F("C  param:")); 
          ConfigParamEnum param = (ConfigParamEnum) data.id;
          Serial.print(getConfigParamName(param));
        Serial.print(F(" = "));
        Serial.println(data.newValue);
      }
      break;
      
    default:
      Serial.println(e->type);
      printError("Unsupported LogTypeID");
  }
}

/*
 * READ and WRITE REQUESTS
 */
void SerialUI::processReadWriteRequests(ReadWriteRequests requests, ControlContext *context, BoilerStateAutomaton *automaton) {
  if (requests & READ_HELP) {
    Serial.println(F("Accepted Commands:"));
    UserCommands commands = automaton->userCommands();
    unsigned short cmd = 0x1;
    for(byte i=0; i< NUM_USER_COMMANDS; i++) {
      if (commands & cmd) {
        Serial.print("  - ");
        Serial.println(getUserCommandName((UserCommandEnum) cmd));
      }
      cmd = cmd << 1;
    }
  }
  
  if (requests & READ_STAT) {
    Serial.print(F("State: "));
    Serial.print(getStateName(automaton->state()->id()));
    Serial.print(F(", Time in state [s]: "));
    Serial.println((millis() - context->op->currentStateStartMillis) / 1000L);
    
    Serial.print(F("Water:   "));
    Serial.print(getSensorStatusName(context->op->water.sensorStatus));
    if (context->op->water.sensorStatus == SENSOR_OK) {
      Serial.print(F(", "));
      Serial.print(formatTemperature(context->op->water.currentTemp));
    }
    Serial.println();
    
    Serial.print(F("Ambient: "));
    Serial.print(getSensorStatusName(context->op->ambient.sensorStatus));
    if (context->op->ambient.sensorStatus == SENSOR_OK) {
      Serial.print(", ");
      Serial.print(formatTemperature(context->op->ambient.currentTemp));
    }
    Serial.println();

    unsigned long duration = heatingTotalMillis(context->op);
    if (duration != 0L) {
      Serial.print(F("Accumulated heating time [s]: "));
      Serial.println(duration / 1000L);
    }
  }
  
  if (requests & READ_LOG) {
    unsigned short entriesToReturn = 5; // default
    byte len = strspn(context->op->command->args, INT_CHARS);
    if (len > 0) {
      context->op->command->args[len] = '\0';
      int n = atoi(context->op->command->args);
      if (n == 0) {
        entriesToReturn = 0;
      } else if (n > 0) {
        entriesToReturn = n;
      }
    }
    
    Serial.print(F("Log contains "));
    Serial.print(context->storage->currentLogEntries());
    Serial.print(F(" entries (="));
    Serial.print((short) (100L * context->storage->currentLogEntries() / context->storage->maxLogEntries()));
    Serial.print(F("% full), showing "));
    Serial.println(entriesToReturn);
    
    context->storage->readMostRecentLogEntries(entriesToReturn);
    LogEntry e;
    while (context->storage->nextLogEntry(&e)) {
      printLogEntry(&e);
    }
  }
  
  if (requests & READ_CONFIG) {
    for(byte i=0; i<NUM_CONFIG_PARAMS; i++) {
      Serial.print(i);
      Serial.print(F(" - "));
      ConfigParamEnum p = (ConfigParamEnum) i;
      Serial.print(getConfigParamName(p));
      Serial.print(F(": "));
      Serial.println(getConfigParamValue(context->config, p));
    }
  }
  
  if (requests & WRITE_CONFIG) {
    // determine length of config param id (=number):
    byte len = strspn(context->op->command->args, INT_CHARS);
    context->op->command->args[len] = '\0';
    int id = atoi(context->op->command->args);
    char *paramValue = &context->op->command->args[len+1];

    if (id < NUM_CONFIG_PARAMS) {
      ConfigParamEnum param = (ConfigParamEnum)id;
      if (setConfigParamValue(context, param, paramValue)) {
        context->storage->updateConfigParams(context->config);
      } else {
      printError("Illegal value");
      }
      
    } else {
      // display 'invalid':
      printError(getConfigParamName((ConfigParamEnum)NUM_CONFIG_PARAMS));
    }
  }
  Serial.println();
}

  
void SerialUI::notifyStatusChange(StatusNotification notification) {
  if (notification.timeInState == 0L) { } // prevent 'unused parameter' warning
  Serial.println(F("* status notification"));
}

void SerialUI::notifyNewLogEntry(LogEntry entry) {
  if (entry.timestamp != UNDEFINED_TIMESTAMP) { } // prevent 'unused parameter' warning
  Serial.println(F("* new log entry"));
}



void BLEUI::readUserCommand(ControlContext *context) {
  if (context != NULL) { } // prevent 'unused parameter' warning
}

void BLEUI::processReadWriteRequests(ReadWriteRequests requests, ControlContext *context, BoilerStateAutomaton *automaton) {
  if (requests != READ_WRITE_NONE || context != NULL || automaton != NULL) { } // prevent 'unused parameter' warning
}

void BLEUI::notifyStatusChange(StatusNotification notification) {
  if (notification.timeInState == 0L) { } // prevent 'unused parameter' warning
}

void BLEUI::notifyNewLogEntry(LogEntry entry) {
  if (entry.timestamp != UNDEFINED_TIMESTAMP) { } // prevent 'unused parameter' warning
}

