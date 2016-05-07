#include <string.h>
#include "ui.h"
#include "config.h"

#define CMD_BUF_SIZE 40   // Size of the read buffer for incoming data
const char COMMAND_ARG_CHARS[] = "-0123456789"; // number
const char COMMAND_CHARS[] = " abcdefghijklmnopqrstuvwxyz";

#define DEBUG_UI

const char STR_PARAM_TARGET_TEMP[] PROGMEM = "Target Temperature";
const char STR_PARAM_WATER_TEMP_SENSOR_ID[] PROGMEM = "Water Temp. Sensor ID";
const char STR_PARAM_AMBIENT_TEMP_SENSOR_ID[] PROGMEM = "Ambient Temp. Sensor ID";
const char STR_PARAM_HEATER_CUT_OUT_WATER_TEMP[] PROGMEM = "Heater Cut-out Temp.";
const char STR_PARAM_HEATER_BACK_OK_WATER_TEMP[] PROGMEM = "Heater Back-ok Temp.";
const char STR_PARAM_LOG_TEMP_DELTA[] PROGMEM = "Log Temp. Delta";
const char STR_PARAM_LOG_TIME_DELTA[] PROGMEM = "Log Time Delta [ms]";
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

void readUserCommands(ControlContext *context) {
  context->op->userCommand = CMD_NONE;
  char command[CMD_BUF_SIZE+1];
  memset(command, 0, CMD_BUF_SIZE);
  if( Serial.peek() < 0 ) {
    return;
  }
  delay(2);

  byte count = 0;
  do {
    count += Serial.readBytes(&command[count], CMD_BUF_SIZE);
    delay(2);
  } while( (count < CMD_BUF_SIZE) && !(Serial.peek() < 0) );
  #ifdef DEBUG_UI
    Serial.print("DEBUG_UI: read cmd string: '");
    Serial.print(command);
    Serial.print("', len: ");
    Serial.println(count);
  #endif

  // count the command characters up to the trailing numeric argument (if any):
  byte commandLength = strspn(command, COMMAND_CHARS);
  
  // find trailing numeric argument (long):
  long argument = NO_CMD_ARGUMENT;
  char *number = strpbrk(command, COMMAND_ARG_CHARS);
  if (number != NULL && number != command) {
    argument = atol(number);
    #ifdef DEBUG_UI
      Serial.print("DEBUG_UI: cmd arg: ");
      Serial.println(argument);
    #endif
    commandLength--;
    command[commandLength] = '\0';
  }

  context->op->userCommand = parseUserCommand(command, commandLength);
  context->op->userCommandArgument = argument;
  
  #ifdef DEBUG_UI
    Serial.print("DEBUG_UI: parsed cmd: ");
    Serial.print(context->op->userCommand, HEX);
    Serial.print(": ");
    Serial.println(getUserCommandName((UserCommandEnum) context->op->userCommand));
  #endif
}

void printSensorId(byte addr[]) {
  Serial.write('{');
  for(byte i = 0; i < TEMP_SENSOR_ID_BYTES; i++) {
    Serial.write(' ');
    Serial.print(addr[i], HEX);
  }
  Serial.println(" }");
}

void processInfoRequests(InfoRequests requests, ControlContext *context, BoilerStateAutomaton *automaton) {
  if (requests & SEND_HELP) {
    Serial.println("Commands:");
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
  
  if (requests & SEND_LOG) {
    unsigned short entriesToReturn = 5; // default
    if (context->op->userCommandArgument == 0) {
      entriesToReturn = 0;
    } else if (context->op->userCommandArgument > 0) {
      entriesToReturn = context->op->userCommandArgument;
    }
    
    LogReader r = context->storage->getReader(entriesToReturn);
    LogEntry e;
    while (context->storage->getLogEntry(&r, &e)) {
      Serial.print(formatTimestamp(e.timestamp));
      Serial.print("  ");
      LogTypeEnum type = (LogTypeEnum)e.type;
      switch (type) {
        case LOG_VALUES:
          LogValuesData lvd;
          memcpy(&lvd, &(e.data), sizeof(LogValuesData));
          Serial.print("V  water:");
          Serial.print(getSensorStatusName((SensorStatusEnum)(lvd.flags>>4)));
          Serial.print(" ");
          Serial.print(formatTemperature(lvd.water));
          Serial.print(", ambient:");
          Serial.print(getSensorStatusName((SensorStatusEnum)(lvd.flags&0x0F)));
          Serial.print(" ");
          Serial.println(formatTemperature(lvd.ambient));
          break;
          
        case LOG_STATE:
          LogStateData lsd;
          memcpy(&lsd, &e.data, sizeof(LogStateData));
          Serial.print("S  ");
          Serial.print(getStateName((StateEnum)lsd.previous));
          Serial.print(" -> <");
          Serial.println(getEventName((EventEnum)lsd.event));
          Serial.print("> -> ");
          Serial.print(getStateName((StateEnum)lsd.current));
          break;
          
        case LOG_MESSAGE:
          LogMessageData lmd;
          memcpy(&lmd, &e.data, sizeof(LogMessageData));
          Serial.print("M  msg:");
          Serial.print(lmd.id);
          Serial.print(", p1:");
          Serial.print(lmd.params[0]);
          Serial.print(", p2:");
          Serial.println(lmd.params[1]);
          break;
          
        case LOG_CONFIG:
          Serial.println("--- LogConfigData ---");
          break;
        default:
          Serial.println("--- Unknown ---");
      }
    }
  }
  
  if (requests & SEND_CONFIG) {
    char colon[] = ": ";
    Serial.print(getConfigParamName(PARAM_TARGET_TEMP));
    Serial.print(colon);
    Serial.println(formatTemperature(context->config->targetTemp));
    
    Serial.print(getConfigParamName(PARAM_WATER_TEMP_SENSOR_ID));
    Serial.print(colon);
    printSensorId(context->config->waterTempSensorId);
    
    Serial.print(getConfigParamName(PARAM_AMBIENT_TEMP_SENSOR_ID));
    Serial.print(colon);
    printSensorId(context->config->ambientTempSensorId);
    
    Serial.print(getConfigParamName(PARAM_HEATER_CUT_OUT_WATER_TEMP));
    Serial.print(colon);
    Serial.println(formatTemperature(context->config->heaterCutOutWaterTemp));
    
    Serial.print(getConfigParamName(PARAM_HEATER_BACK_OK_WATER_TEMP));
    Serial.print(colon);
    Serial.println(formatTemperature(context->config->heaterBackOkWaterTemp));
    
    Serial.print(getConfigParamName(PARAM_LOG_TEMP_DELTA));
    Serial.print(colon);
    Serial.println(formatTemperature(context->config->logTempDelta));
    
    Serial.print(getConfigParamName(PARAM_LOG_TIME_DELTA));
    Serial.print(colon);
    Serial.println(context->config->logTimeDelta);
    
    Serial.print(getConfigParamName(PARAM_TANK_CAPACITY));
    Serial.print(colon);
    Serial.println(context->config->tankCapacity);
    
    Serial.print(getConfigParamName(PARAM_HEATER_POWER));
    Serial.print(colon);
    Serial.println(context->config->heaterPower);
    
    Serial.print(getConfigParamName(PARAM_INSULATION_FACTOR));
    Serial.print(colon);
    Serial.println(context->config->insulationFactor);
  }
  
  if (requests & SEND_STAT) {
    Serial.print("State: ");
    Serial.print(getStateName(automaton->state()->id()));
    Serial.print(", Time in state [s]: ");
    Serial.println((millis() - context->op->currentStateStartMillis) / 1000L);
    
    Serial.print("Water:   ");
    Serial.print(getSensorStatusName(context->op->water.sensorStatus));
    if (context->op->water.sensorStatus == SENSOR_OK) {
      Serial.print(", ");
      Serial.print(formatTemperature(context->op->water.currentTemp));
    }
    Serial.println();
    
    Serial.print("Ambient: ");
    Serial.print(getSensorStatusName(context->op->ambient.sensorStatus));
    if (context->op->ambient.sensorStatus == SENSOR_OK) {
      Serial.print(", ");
      Serial.print(formatTemperature(context->op->ambient.currentTemp));
    }
    Serial.println();

    if (context->op->heatingStartMillis != 0L || context->op->heatingTotalMillis != 0L) {
      unsigned long duration = context->op->heatingTotalMillis;
      if (context->op->heatingStartMillis != 0L) {
        duration += millis() - context->op->heatingStartMillis;
      }
      Serial.print("Accumulated heating time [s]: ");
      Serial.println(duration / 1000L);
    }
  }
}

