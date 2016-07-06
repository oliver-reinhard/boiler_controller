#include "ui_ser.h"
#include "config.h"

// #define DEBUG_UI

#define COMMAND_BUF_SIZE 24   // Size of the read buffer for incoming data
const char COMMAND_CHARS[] = " abcdefghijklmnopqrstuvwxyz"; // includes blank (first char)
const char INT_CHARS[] = "0123456789"; 

// See the original definition of F(.) in WString.h
#define FP(progmem_string_ref) (reinterpret_cast<const __FlashStringHelper *>(progmem_string_ref))

const char STR_NONE[] PROGMEM = "None";
const char STR_ILLEGAL[] PROGMEM = "Illegal";

const char STR_CONFIG_SENSOR_ID_SWAP[] PROGMEM = "swap";
const char STR_CONFIG_SENSOR_ID_CLR[]  PROGMEM = "clr";
const char STR_CONFIG_SENSOR_ID_ACK[]  PROGMEM = "ack";

/*
 * CONFIG PARAM NAMES
 */
const __FlashStringHelper *getConfigParamName(ConfigParamEnum literal) {
  switch(literal) {
    case PARAM_TARGET_TEMP: return F("Target Temp");
    case PARAM_WATER_TEMP_SENSOR_ID: return F("Water Temp Sensor ID");
    case PARAM_AMBIENT_TEMP_SENSOR_ID: return F("Ambient Temp Sensor ID");
    case PARAM_HEATER_CUT_OUT_WATER_TEMP: return F("Heater Cut-out Temp");
    case PARAM_HEATER_BACK_OK_WATER_TEMP: return F("Heater Back-ok Temp");
    case PARAM_LOG_TEMP_DELTA: return F("Log Temp Delta");
    case PARAM_LOG_TIME_DELTA: return F("Log Time Delta [s]");
    case PARAM_TANK_CAPACITY: return F("Tank Capacity [ml]");
    case PARAM_HEATER_POWER: return F("Heater Power [W]");
    default: return F("Undef");
  }
}

#define MAX_FLOAT_STR_LEN 20

char *formatFloat(float f, char s[]) {
  dtostrf((double)f, 6, 2, s); 
  return s;
}

#define MAX_INT_STR_LEN 8  // from 16-bit integer

char *formatInt(uint16_t i, char s[]) {
  utoa(i, s, 10);
  return s;
}

char *getConfigParamValue(ConfigParams *all, ConfigParamEnum p, char buf[]) {
  switch(p) {
    case PARAM_TARGET_TEMP: 
      return formatTemperature(all->targetTemp, buf);
    case PARAM_WATER_TEMP_SENSOR_ID: 
      return formatTempSensorID(all->waterTempSensorId, buf);
    case PARAM_AMBIENT_TEMP_SENSOR_ID: 
      return formatTempSensorID(all->ambientTempSensorId, buf);
    case PARAM_HEATER_CUT_OUT_WATER_TEMP: 
      return formatTemperature(all->heaterCutOutWaterTemp, buf);
    case PARAM_HEATER_BACK_OK_WATER_TEMP: 
      return formatTemperature(all->heaterBackOkWaterTemp, buf);
    case PARAM_LOG_TEMP_DELTA: 
      return formatTemperature(all->logTempDelta, buf);
    case PARAM_LOG_TIME_DELTA:
      return formatInt(all->logTimeDelta, buf);
    case PARAM_TANK_CAPACITY:
      return formatFloat(all->tankCapacity, buf);
    case PARAM_HEATER_POWER:
      return formatFloat(all->heaterPower, buf);
    default: 
      buf[0] = '\0';
      return buf;
  }
}


boolean setConfigParamValue(ExecutionContext *context, ConfigParamEnum p, char *value) {
  ConfigParams *config = context->config;
  Log *log = context->log;
  switch(p) {
    case PARAM_TARGET_TEMP: 
      config->targetTemp = atoi(value);
      log->logConfigParam(p, (float) config->targetTemp);
      return true;
    case PARAM_WATER_TEMP_SENSOR_ID:
    case PARAM_AMBIENT_TEMP_SENSOR_ID:
      if (!strcmp_P(value, STR_CONFIG_SENSOR_ID_SWAP)) {
        context->control->swapTempSensorIDs();
        return true;
      } else if (!strcmp_P(value, STR_CONFIG_SENSOR_ID_CLR)) {
        context->control->clearTempSensorIDs();
        return true;
      } else if (!strcmp_P(value, STR_CONFIG_SENSOR_ID_ACK)) {
        return context->control->confirmTempSensorIDs();
      } 
      return false;
    case PARAM_HEATER_CUT_OUT_WATER_TEMP: 
      config->heaterCutOutWaterTemp = atoi(value);
      log->logConfigParam(p, (float) config->heaterCutOutWaterTemp);
      return true;
    case PARAM_HEATER_BACK_OK_WATER_TEMP: 
      config->heaterBackOkWaterTemp = atoi(value);
      log->logConfigParam(p, (float) config->heaterBackOkWaterTemp);
      return true;
    case PARAM_LOG_TEMP_DELTA: 
      config->logTempDelta = atoi(value);
      log->logConfigParam(p, (float) config->logTempDelta);
      return true;
    case PARAM_LOG_TIME_DELTA:
      config->logTimeDelta = atoi(value);
      log->logConfigParam(p, (float) config->logTimeDelta);
      return true;
    case PARAM_TANK_CAPACITY:
      config->tankCapacity = (float) atof(value);
      log->logConfigParam(p, config->tankCapacity);
      return true;
    case PARAM_HEATER_POWER:
      config->heaterPower = (float) atof(value);
      log->logConfigParam(p, config->heaterPower);
      return true;
    default:
      return false;
  }
}

/*
 * STATES
 */
const __FlashStringHelper *getStateName(StateEnum literal) {
  switch(literal) {
    case STATE_UNDEFINED: return F("Undef");
    case STATE_SAME: return F("Same");
    case STATE_INIT: return F("Init");
    case STATE_SENSORS_NOK: return F("Sensors NOK");
    case STATE_READY: return F("Ready");
    case STATE_IDLE: return F("Idle");
    case STATE_RECORDING: return F("Recording");
    case STATE_STANDBY: return F("Standby");
    case STATE_HEATING: return F("Heating");
    case STATE_OVERHEATED: return F("Overheated");
    default: return FP(STR_ILLEGAL);
  }
}

/*
 * EVENTS
 */  
const __FlashStringHelper *getEventName(EventEnum literal) {
  switch(literal) {
    case EVENT_NONE: return FP(STR_NONE);
    case EVENT_READY: return F("Ready");
    case EVENT_SENSORS_NOK: return F("Sensors NOK");
    case EVENT_SET_CONFIG: return F("Set Config");
    case EVENT_REC_ON: return F("Rec On");
    case EVENT_REC_OFF: return F("Rec Off");
    case EVENT_GET_CONFIG: return F("Get Config");
    case EVENT_GET_LOG: return F("Get Log");
    case EVENT_GET_STAT: return F("Get Stat");
    case EVENT_HEAT_ON: return F("Heat On");
    case EVENT_HEAT_OFF: return F("Heat Off");
    case EVENT_TEMP_OVER: return F("Temp Over");
    case EVENT_TEMP_OK: return F("Temp OK");
    case EVENT_RESET: return F("Reset");
    default: return FP(STR_ILLEGAL);
  } 
}


/*
 * COMMANDS
 */
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

#define MAX_CMD_NAME_LEN 12

PGM_P getUserCommandNamePtr(UserCommandEnum literal) {
  switch(literal) {
    case CMD_NONE: return STR_NONE;
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
    default: return STR_ILLEGAL;
  }
}

char *getUserCommandName(UserCommandEnum literal, char buf[]) {
  strcpy_P(buf, getUserCommandNamePtr(literal));
  return buf;
}

/*
 * SENSOR STATUS
 */
const __FlashStringHelper *getSensorStatusName(SensorStatusEnum literal) {
  switch(literal) {
    case SENSOR_INITIALISING:     return F("Init");
    case SENSOR_ID_AUTO_ASSIGNED: return F("Auto ID");
    case SENSOR_ID_UNDEFINED:     return F("No ID");
    case SENSOR_OK:               return F("OK");
    case SENSOR_NOK:              return F("NOK");
    default: return FP(STR_ILLEGAL);
  }
}

/*
 * USER COMMANDS
 */
UserCommandEnum parseUserCommand(char buf[], uint8_t bufSize) {
  switch (bufSize) {
    case 1:
      if (!strcmp(buf, "?")) {
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

void printError(const __FlashStringHelper *err) {
  Serial.print(F("Error: "));
  Serial.print(err);
  Serial.println(F("."));
}


void SerialUI::setup() {
  if (context != NULL) { } // prevent 'unused parameter' warning
  // empty
}
      
void SerialUI::readUserCommand() {
  char buf[COMMAND_BUF_SIZE+1];
  // fill buffer with 0's => always \0-terminated!
  memset(buf, 0, COMMAND_BUF_SIZE);
  if( Serial.peek() < 0 ) {
    return;
  }
  delay(2);

  uint8_t count = 0;
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
  uint8_t len = 0;
  boolean prevSpace = false;
  for (uint8_t i = 0; i< count; i++) {
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
  
  OperationalParams *op = context->op;
  // count the command characters up to the trailing numeric arguments (if any):
  uint8_t commandLength = strspn(lower, COMMAND_CHARS);
  if (len > commandLength) {
    strcpy(op->command->args, &lower[commandLength]);
  }
  
  if (isspace(lower[commandLength - 1])) {
    commandLength--;
  }
  lower[commandLength] = '\0';
  op->command->command = parseUserCommand(buf, commandLength);  
  
  #ifdef DEBUG_UI
    Serial.print(F("DEBUG_UI: parsed cmd: 0x"));
    Serial.print(op->command->command, HEX);
    Serial.print(F(": "));
    char cmdName[MAX_CMD_NAME_LEN];
    Serial.print(getUserCommandName((UserCommandEnum) op->command->command, cmdName));
    Serial.print(F(", args: '"));
    Serial.print(op->command->args);
    Serial.println(F("'"));
  #endif
  if (op->command->command == CMD_NONE) {
    printError(F("Illegal command (try: help or ?)"));
  }
}

void printLogEntry(LogEntry *e) {
  char buf[30];
  Serial.print(formatTimestamp(e->timestamp, buf));
  Serial.print("  ");
  LogDataTypeEnum type = (LogDataTypeEnum)e->type;
  switch (type) {
    case LOG_DATA_TYPE_MESSAGE:
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
      
    case LOG_DATA_TYPE_VALUES:
      {
        LogValuesData data;
        memcpy(&data, &(e->data), sizeof(LogValuesData));
        Serial.print(F("V  water:"));
        Serial.print(getSensorStatusName((SensorStatusEnum)(data.flags>>4)));
        Serial.print(F(" "));
        Serial.print(formatTemperature(data.water, buf));
        Serial.print(F(", ambient:"));
        Serial.print(getSensorStatusName((SensorStatusEnum)(data.flags&0x0F)));
        Serial.print(F(" "));
        Serial.println(formatTemperature(data.ambient, buf));
      }
      break;
      
    case LOG_DATA_TYPE_STATE:
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
      
    case LOG_DATA_TYPE_CONFIG:
      {
        LogConfigParamData data;
        memcpy(&data, &e->data, sizeof(LogConfigParamData));
        Serial.print(F("C  param:")); 
        ConfigParamEnum param = (ConfigParamEnum) data.id;
        Serial.print(getConfigParamName(param));
        Serial.print(F(" = "));
        Serial.println(formatFloat(data.newValue, buf));  // line uses 1 KByte without the formatFloat!
      }
      break;
      
    default:
      Serial.println(e->type);
      printError(F("Unsupported LogTypeID"));
  }
}

/*
 * READ and WRITE REQUESTS
 */
void SerialUI::processReadWriteRequests(ReadWriteRequests requests, BoilerStateAutomaton *automaton) {
  #ifdef DEBUG_UI
    Serial.print(F("DEBUG_UI: processing request: 0x"));
    Serial.println(requests, HEX);
  #endif
  
  char buf[32];
        
  if (requests & READ_HELP) {
    Serial.println(F("Accepted Commands:"));
    UserCommands commands = automaton->userCommands();
    uint16_t cmd = 0x1;
    for(uint8_t i=0; i< NUM_USER_COMMANDS; i++) {
      if (commands & cmd) {
        Serial.print("  - ");
        Serial.println(getUserCommandName((UserCommandEnum) cmd, buf));
      }
      cmd = cmd << 1;
    }
  }
  
  OperationalParams *op = context->op;
  
  if (requests & READ_STAT) {
    Serial.print(F("State: "));
    Serial.print(getStateName(automaton->state()->id()));
    Serial.print(F(", Time in state [s]: "));
    Serial.println((millis() - op->currentStateStartMillis) / 1000L);
    
    Serial.print(F("Water:   "));
    Serial.print(getSensorStatusName(op->water.sensorStatus));
    if (op->water.sensorStatus == SENSOR_OK  || op->water.sensorStatus == SENSOR_ID_AUTO_ASSIGNED) {
      Serial.print(F(", "));
      Serial.print(formatTemperature(op->water.currentTemp, buf));
    }
    Serial.println();
    
    Serial.print(F("Ambient: "));
    Serial.print(getSensorStatusName(op->ambient.sensorStatus));
    if (op->ambient.sensorStatus == SENSOR_OK || op->ambient.sensorStatus == SENSOR_ID_AUTO_ASSIGNED) {
      Serial.print(F(", "));
      Serial.print(formatTemperature(op->ambient.currentTemp, buf));
    }
    Serial.println();

    uint32_t duration = heatingTotalMillis(op);
    if (duration != 0L) {
      Serial.print(F("Accumulated heating time [s]: "));
      Serial.println(duration / 1000L);
    }
  }
  
  if (requests & READ_LOG) {
    uint16_t entriesToReturn = 5; // default
    uint8_t len = strspn(op->command->args, INT_CHARS);
    if (len > 0) {
      op->command->args[len] = '\0';
      int n = atoi(op->command->args);
      if (n == 0) {
        entriesToReturn = 0;
      } else if (n > 0) {
        entriesToReturn = n;
      }
    }
    
    Serial.print(F("Log contains "));
    Serial.print(context->log->currentLogEntries());
    Serial.print(F(" entries (= "));
    Serial.print((int16_t) (100L * context->log->currentLogEntries() / context->log->maxLogEntries()));
    Serial.print(F("% full), showing "));
    Serial.println(entriesToReturn);
    
    context->log->readMostRecentLogEntries(entriesToReturn);
    LogEntry e;
    while (context->log->nextLogEntry(e)) {
      printLogEntry(&e);
    }
  }
  
  if (requests & READ_CONFIG) {
    for(uint8_t i=0; i<NUM_CONFIG_PARAMS; i++) {
      Serial.print(i);
      Serial.print(F(" - "));
      ConfigParamEnum p = (ConfigParamEnum) i;
      Serial.print(getConfigParamName(p));
      Serial.print(F(": "));
      Serial.println(getConfigParamValue(context->config, p, buf));
    }
  }
  
  if (requests & WRITE_CONFIG) {
    // determine length of config param id (=number):
    uint8_t len = strspn(context->op->command->args, INT_CHARS);
    context->op->command->args[len] = '\0';
    int16_t id = atoi(context->op->command->args);
    char *paramValue = &context->op->command->args[len+1];

    if (id < NUM_CONFIG_PARAMS) {
      ConfigParamEnum param = (ConfigParamEnum)id;
      if (setConfigParamValue(context, param, paramValue)) {
        context->config->save();
      } else {
        printError(F("Illegal value"));
      }
      
    } else {
      printError(F("Unknown config parameter"));
    }
  }
  Serial.println();
}

  
void SerialUI::notifyStatusChange(StatusNotification *) {
  Serial.println(F("* status notification"));
}

void SerialUI::notifyNewLogEntry(LogEntry) {
  Serial.println(F("* new log entry"));
}


