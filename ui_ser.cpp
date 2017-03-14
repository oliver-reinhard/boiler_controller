#include "ui_ser.h"
#include "config.h"

// #define DEBUG_UI

#define CMD_LINE_BUF_SIZE 24   // Size of the read buffer for incoming data
const char CMD_CHARS[] = " abcdefghijklmnopqrstuvwxyz?"; // includes blank (first char)
const char INT_CHARS[] = "0123456789"; 

// See the original definition of F(.) in WString.h
#define FP(progmem_string_ref) (reinterpret_cast<const __FlashStringHelper *>(progmem_string_ref))

const char STR_NONE[] PROGMEM = "None";
const char STR_ILLEGAL[] PROGMEM = "Illegal";

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

#if defined(ARDUINO_SAMD_ZERO) || defined(ARDUINO_SAMD_MKR1000)
// dtostrf is missing from AVR library for SAMD ...
// see https://github.com/adafruit/Adafruit_MQTT_Library/blob/master/Adafruit_MQTT.cpp
static char *dtostrf (double val, signed char width, unsigned char prec, char *sout) {
  char fmt[20];
  sprintf(fmt, "%%%d.%df", width, prec);
  sprintf(sout, fmt, val);
  return sout;
}
#endif

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
      return formatDS18B20_SensorID(all->waterTempSensorId, buf);
    case PARAM_AMBIENT_TEMP_SENSOR_ID: 
      return formatDS18B20_SensorID(all->ambientTempSensorId, buf);
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
    case EVENT_INFO: return F("Info");
    case EVENT_CONFIG_MODIFY: return F("Config Modify");
    case EVENT_CONFIG_RESET: return F("Config Reset");
    case EVENT_REC_ON: return F("Rec On");
    case EVENT_REC_OFF: return F("Rec Off");
    case EVENT_HEAT_ON: return F("Heat On");
    case EVENT_HEAT_OFF: return F("Heat Off");
    case EVENT_HEAT_RESET: return F("Heat Reset");
    case EVENT_TEMP_OVER: return F("Temp Over");
    case EVENT_TEMP_OK: return F("Temp OK");
    default: return FP(STR_ILLEGAL);
  } 
}


/*
 * COMMANDS
 */
const char STR_CMD_INFO_HELP[]        PROGMEM = "help";
const char STR_CMD_INFO_STAT[]        PROGMEM = "stat";
const char STR_CMD_INFO_CONFIG[]      PROGMEM = "config";
const char STR_CMD_INFO_LOG[]         PROGMEM = "log";        // + <param>
const char STR_CMD_CONFIG_SET_VALUE[] PROGMEM = "config set"; // + <id> <value>
const char STR_CMD_CONFIG_SWAP_IDS[]  PROGMEM = "config swqp ids";
const char STR_CMD_CONFIG_CLEAR_IDS[] PROGMEM = "config clr ids";
const char STR_CMD_CONFIG_ACK_IDS[]   PROGMEM = "config ack ids";
const char STR_CMD_CONFIG_RESET_ALL[] PROGMEM = "config reset";
const char STR_CMD_REC_ON[]           PROGMEM = "rec on";
const char STR_CMD_REC_OFF[]          PROGMEM = "rec off";
const char STR_CMD_HEAT_ON[]          PROGMEM = "heat on";
const char STR_CMD_HEAT_OFF[]         PROGMEM = "heat off";
const char STR_CMD_HEAT_RESET[]       PROGMEM = "heat reset";

#define MAX_CMD_NAME_LEN 15  // not including trailing \0

PGM_P getUserCommandNamePtr(UserCommandEnum literal) {
  switch(literal) {
    case CMD_NONE: return STR_NONE;
    case CMD_INFO_HELP: return STR_CMD_INFO_HELP;
    case CMD_INFO_STAT: return STR_CMD_INFO_STAT;
    case CMD_INFO_CONFIG: return STR_CMD_INFO_CONFIG;
    case CMD_INFO_LOG: return STR_CMD_INFO_LOG;
    case CMD_CONFIG_SET_VALUE: return STR_CMD_CONFIG_SET_VALUE;
    case CMD_CONFIG_SWAP_IDS: return STR_CMD_CONFIG_SWAP_IDS;
    case CMD_CONFIG_CLEAR_IDS: return STR_CMD_CONFIG_CLEAR_IDS;
    case CMD_CONFIG_ACK_IDS: return STR_CMD_CONFIG_ACK_IDS;
    case CMD_CONFIG_RESET_ALL: return STR_CMD_CONFIG_RESET_ALL;
    case CMD_REC_ON: return STR_CMD_REC_ON;
    case CMD_REC_OFF: return STR_CMD_REC_OFF;
    case CMD_HEAT_ON: return STR_CMD_HEAT_ON;
    case CMD_HEAT_OFF: return STR_CMD_HEAT_OFF;
    case CMD_HEAT_RESET: return STR_CMD_HEAT_RESET;
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
const __FlashStringHelper *getSensorStatusName(DS18B20_StatusEnum literal) {
  switch(literal) {
    case DS18B20_SENSOR_INITIALISING:     return F("Init");
    case DS18B20_SENSOR_ID_AUTO_ASSIGNED: return F("Auto ID");
    case DS18B20_SENSOR_ID_UNDEFINED:     return F("No ID");
    case DS18B20_SENSOR_OK:               return F("OK");
    case DS18B20_SENSOR_NOK:              return F("NOK");
    default: return FP(STR_ILLEGAL);
  }
}

/*
 * USER COMMANDS
 */
UserCommandEnum parseUserCommand(char cmd[], uint8_t cmdLen) {
  switch (cmdLen) {
    case 1:
      if (!strcmp(cmd, "?")) {
        return CMD_INFO_HELP;
      } 
      break;
    case 3:
      if (!strcmp_P(cmd, STR_CMD_INFO_LOG)) {
        return CMD_INFO_LOG;
      }
      break;
    case 4:
      if (!strcmp_P(cmd, STR_CMD_INFO_HELP)) {
        return CMD_INFO_HELP;
      } else if (!strcmp_P(cmd, STR_CMD_INFO_STAT)) {
        return CMD_INFO_STAT;
      } 
      break;
    case 6:
      if (!strcmp_P(cmd, STR_CMD_INFO_CONFIG)) {
        return CMD_INFO_CONFIG;
      } else if (!strcmp_P(cmd, STR_CMD_REC_ON)) {
        return CMD_REC_ON;
      }  
      break;
    case 7:
      if (!strcmp_P(cmd, STR_CMD_REC_OFF)) {
        return CMD_REC_OFF;
      } else if (!strcmp_P(cmd, STR_CMD_HEAT_ON)) {
        return CMD_HEAT_ON;
      }
      break;
    case 8:      
      if (!strcmp_P(cmd, STR_CMD_HEAT_OFF)) {
        return CMD_HEAT_OFF;
      } 
      break;
    case 10:      
      if (!strcmp_P(cmd, STR_CMD_CONFIG_SET_VALUE)) {
        return CMD_CONFIG_SET_VALUE;
      } else if (!strcmp_P(cmd, STR_CMD_HEAT_RESET)) {
        return CMD_HEAT_RESET;
      } 
      break;
    case 12:      
      if (!strcmp_P(cmd, STR_CMD_CONFIG_RESET_ALL)) {
        return CMD_CONFIG_RESET_ALL;
      } 
      break;
    case 14:      
      if (!strcmp_P(cmd, STR_CMD_CONFIG_CLEAR_IDS)) {
        return CMD_CONFIG_CLEAR_IDS;
      } else if (!strcmp_P(cmd, STR_CMD_CONFIG_ACK_IDS)) {
        return CMD_CONFIG_ACK_IDS;
      } 
      break;
    case 15:      
      if (!strcmp_P(cmd, STR_CMD_CONFIG_SWAP_IDS)) {
        return CMD_CONFIG_SWAP_IDS;
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


char *readCommandLine(char buf[]) {
  uint8_t count = 0;
  do {
    count += Serial.readBytes(&buf[count], CMD_LINE_BUF_SIZE);
    delay(2);
  } while( (count < CMD_LINE_BUF_SIZE) && Serial.available());
  #ifdef DEBUG_UI
    Serial.print(F("DEBUG_UI: read cmd line: '"));
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
  return lower;
}

      
void SerialUI::readUserRequest() {
  if( ! Serial.available()) {
    return;
  }
  delay(2);
  
  char buf[CMD_LINE_BUF_SIZE+1];
  // fill buffer with 0's => always \0-terminated!
  memset(buf, 0, CMD_LINE_BUF_SIZE+1);
  char *cmdLine = readCommandLine(buf);
  
  UserRequest *request = &(context->op->request);
  
  // count the request characters up to the trailing numeric arguments (if any):
  uint8_t cmdLen = strspn(cmdLine, CMD_CHARS);
  
  // set request args as anything following the command:
  char *args = &cmdLine[cmdLen];
  
  // ignore trailing space (if any):
  if (isspace(cmdLine[cmdLen - 1])) {
    cmdLen--;
  }
  cmdLine[cmdLen] = '\0';
  request->command = parseUserCommand(cmdLine, cmdLen);  
  
  #ifdef DEBUG_UI
    Serial.print(F("DEBUG_UI: parsed cmd: 0x"));
    Serial.print(request->command, HEX);
    Serial.print(F(": "));
    char cmdName[MAX_CMD_NAME_LEN];
    Serial.print(getUserCommandName((UserCommandEnum) request->command, cmdName));
    Serial.print(F(", args: '"));
    Serial.print(args);
    Serial.println(F("'"));
  #endif
  if (request->command == CMD_NONE) {
    printError(F("Illegal command (try: help or ?)"));
  }

  // parse command args where applicable:
  if (request->command == CMD_CONFIG_SET_VALUE) {
    // determine length of config param id (=number):
    uint8_t len = strspn(args, INT_CHARS);
    args[len] = '\0';
    UserCommandID id = atoi(args);
    char *paramValue = &args[len+1];

    if (id < NUM_CONFIG_PARAMS) {
      ConfigParamEnum param = (ConfigParamEnum)id;
      ConfigParamTypeEnum type = context->config->paramType(param);
      request->param = param;
      if (type == PARAM_TYPE_TEMPERATURE) {
        request->intValue = atoi(paramValue);
      } else if (type == PARAM_TYPE_FLOAT) {
        request->floatValue = atof(paramValue);
      } else {
        printError(F("Illegal value"));
      }
      
    } else {
      printError(F("Unknown config parameter"));
    }
    
  } else if (request->command == CMD_INFO_LOG) {
    // determine length of number of log entries to return (if any):
    request->intValue = -1L;
    uint8_t len = strspn(args, INT_CHARS);
    if (len > 0) {
      args[len] = '\0';
      request->intValue = atoi(args);
    }
  }
  
  #ifdef DEBUG_UI
    Serial.print(F("DEBUG_UI: UserRequest = {"));
    Serial.print(request->command, HEX);
    Serial.print(',');
    Serial.print(request->param);
    Serial.print(',');
    Serial.print(request->intValue);
    Serial.print(',');
    Serial.print(request->floatValue);
    Serial.print(',');
    Serial.print(request->event);
    Serial.println('}');
  #endif
}

/*
 * COMMAND EXECUTION
 */

void SerialUI::commandExecuted(boolean success) {
  if (success) {
    Serial.println(F("* command ok."));
  } else {
    Serial.println(F("* command failed."));
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
        Serial.print(getSensorStatusName((DS18B20_StatusEnum)(data.flags>>4)));
        Serial.print(F(" "));
        Serial.print(formatTemperature(data.water, buf));
        Serial.print(F(", ambient:"));
        Serial.print(getSensorStatusName((DS18B20_StatusEnum)(data.flags&0x0F)));
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
      
void SerialUI::provideUserInfo(BoilerStateAutomaton *automaton) {
  UserCommandEnum request = context->op->request.command;
  #ifdef DEBUG_UI
    Serial.print(F("DEBUG_UI: processing info request: 0x"));
    Serial.println(request, HEX);
  #endif
  
  char buf[32];
  OperationalParams *op = context->op;
        
  if (request == CMD_INFO_HELP) {
    Serial.println(F("Accepted Commands:"));
    UserCommands commands = automaton->acceptedUserCommands();
    uint16_t cmd = 0x1;
    for(uint8_t i=0; i< NUM_USER_COMMANDS; i++) {
      if (commands & cmd) {
        Serial.print("  - ");
        Serial.print(getUserCommandName((UserCommandEnum) cmd, buf));
        if (cmd == CMD_CONFIG_SET_VALUE) {
          Serial.print(F(" <param-id> <value>"));
        } else if (cmd == CMD_INFO_LOG) {
          Serial.print(F(" [<result-lines>]   (0 -> all)"));
        }
        Serial.println();

      }
      cmd = cmd << 1;
    }
    
  } else if (request == CMD_INFO_STAT) {
    Serial.print(F("State: "));
    Serial.print(getStateName(automaton->state()->id()));
    Serial.print(F(", Time in state [s]: "));
    Serial.println((millis() - op->currentStateStartMillis) / 1000L);
    
    Serial.print(F("Water:   "));
    Serial.print(getSensorStatusName(op->water.sensorStatus));
    if (op->water.sensorStatus == DS18B20_SENSOR_OK  || op->water.sensorStatus == DS18B20_SENSOR_ID_AUTO_ASSIGNED) {
      Serial.print(F(", "));
      Serial.print(formatTemperature(op->water.currentTemp, buf));
    }
    Serial.println();
    
    Serial.print(F("Ambient: "));
    Serial.print(getSensorStatusName(op->ambient.sensorStatus));
    if (op->ambient.sensorStatus == DS18B20_SENSOR_OK || op->ambient.sensorStatus == DS18B20_SENSOR_ID_AUTO_ASSIGNED) {
      Serial.print(F(", "));
      Serial.print(formatTemperature(op->ambient.currentTemp, buf));
    }
    Serial.println();

    TimeMills duration = heatingTotalMillis(op);
    if (duration != 0L) {
      Serial.print(F("Accumulated heating time [s]: "));
      Serial.println(duration / 1000L);
    }
    
  } else if (request == CMD_INFO_LOG) {
    uint16_t entriesToReturn;
    if (op->request.intValue == 0) {
      entriesToReturn = 0;
    } else if (op->request.intValue > 0) {
      entriesToReturn = op->request.intValue;
    } else {
      entriesToReturn = 5; // default
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
    
  } else if (request == CMD_INFO_CONFIG) {
    for(uint8_t i=0; i<NUM_CONFIG_PARAMS; i++) {
      Serial.print(i);
      Serial.print(F(" - "));
      ConfigParamEnum p = (ConfigParamEnum) i;
      Serial.print(getConfigParamName(p));
      Serial.print(F(": "));
      Serial.println(getConfigParamValue(context->config, p, buf));
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


