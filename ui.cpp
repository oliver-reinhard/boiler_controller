#include <string.h>
#include "ui.h"
#include "config.h"

#define CMD_BUF_SIZE 128   // Size of the read buffer for incoming data

String getConfigParamName(ConfigParamEnum literal) {
  switch(literal) {
    case PARAM_TARGET_TEMP: return "Target Temperature";
    case PARAM_WATER_TEMP_SENSOR_ID: return "Water Temp. Sensor ID";
    case PARAM_AMBIENT_TEMP_SENSOR_ID: return "Ambient Temp. Sensor ID";
    case PARAM_HEATER_CUT_OUT_WATER_TEMP: return "Heater Cut-out Temp.";
    case PARAM_HEATER_BACK_OK_WATER_TEMP: return "Heater Back-ok Temp.";
    case PARAM_LOG_TEMP_DELTA: return "Log Temp. Delta";
    case PARAM_LOG_TIME_DELTA: return "Log Time Delta";
    case PARAM_TANK_CAPACITY: return "Tank Capacity";
    case PARAM_HEATER_POWER: return "Heater Power";
    case PARAM_INSULATION_FACTOR: return "Insulation Factor";
    default: return "Undefined Config Param";
  }
}

String getStateName(StateEnum literal) {
  switch(literal) {
    case STATE_UNDEFINED: return "Undefined";
    case STATE_SAME: return "Same";
    case STATE_INIT: return "Init";
    case STATE_SENSORS_NOK: return "Sensors NOK";
    case STATE_READY: return "Ready";
    case STATE_IDLE: return "Idle";
    case STATE_RECORDING: return "Recording";
    case STATE_STANDBY: return "Standby";
    case STATE_HEATING: return "Heating";
    case STATE_OVERHEATED: return "Overheated";
    default: return "Undefined State";
  }
}

String getEventName(EventEnum literal) {
  switch(literal) {
    case EVENT_NONE: return "None";
    case EVENT_READY: return "Ready";
    case EVENT_SENSORS_NOK: return "Sensors NOK";
    case EVENT_SET_CONFIG: return "Set Config";
    case EVENT_REC_ON: return "Rec On";
    case EVENT_REC_OFF: return "Rec Off";
    case EVENT_GET_CONFIG: return "Get Config";
    case EVENT_GET_LOG: return "Get Log";
    case EVENT_GET_STAT: return "Get Stat";
    case EVENT_HEAT_ON: return "Heat On";
    case EVENT_HEAT_OFF: return "Heat Off";
    case EVENT_TEMP_OVER: return "Temp Over";
    case EVENT_TEMP_OK: return "Temp OK";
    case EVENT_RESET: return "Reset";
    default: return "Undefined Event";
  } 
}

String getUserCommandName(UserCommandEnum literal) {
  switch(literal) {
    case CMD_NONE: return "None";
    case CMD_SET_CONFIG: return "set config";
    case CMD_REC_ON: return "rec on";
    case CMD_REC_OFF: return "rec off";
    case CMD_HELP: return "help";
    case CMD_GET_LOG: return "get log";
    case CMD_GET_CONFIG: return "get config";
    case CMD_GET_STAT: return "get stat";
    case CMD_HEAT_ON: return "heat on";
    case CMD_HEAT_OFF: return "heat off";
    case CMD_RESET: return "reset";
    default: return "Undefined User Command";
  }
}

String getSensorStatusName(SensorStatusEnum literal) {
  switch(literal) {
    case SENSOR_INITIALISING: return "Init";
    case SENSOR_OK: return "OK";
    case SENSOR_NOK: return "NOK";
    case SENSOR_ID_UNDEFINED: return "No ID";
    default: return "Undefined Sensor Status";
  }
}

UserCommandEnum parseUserCommand(char buf[], byte size) {
  switch (size) {
    case 2:
      if (!strcmp(buf, "?")) {
        return CMD_HELP;
      } 
      break;
    case 5:
      if (!strcmp(buf, "help")) {
        return CMD_HELP;
      } 
      break;
    case 6:
      if (!strcmp(buf, "reset")) {
        return CMD_RESET;
      } 
      break;
    case 7:
      if (!strcmp(buf, "rec on")) {
        return CMD_REC_ON;
      } 
      break;
    case 8:
      if (!strcmp(buf, "rec off")) {
        return CMD_REC_OFF;
      } else if (!strcmp(buf, "heat on")) {
        return CMD_HEAT_ON;
      } else if (!strcmp(buf, "get log")) {
        return CMD_GET_LOG;
      }
      break;
    case 9:
      if (!strcmp(buf, "heat off")) {
        return CMD_HEAT_OFF;
      } else if (!strcmp(buf, "get stat")) {
        return CMD_GET_STAT;
      } 
      break;
    case 10:
      if (!strcmp(buf, "set gonfig")) {
        return CMD_SET_CONFIG;
      } if (!strcmp(buf, "get config")) {
        return CMD_GET_CONFIG;
      }
      break;
    default:
      break; 
  }
  return CMD_NONE;
}

void readUserCommands(ControlContext *context) {
  context->op->userCommands = CMD_NONE;
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
  
  context->op->userCommands = parseUserCommand(command, count);
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
    // TODO
  }
  
  if (requests & SEND_CONFIG) {
    // TODO
  }
  
  if (requests & SEND_STAT) {
    Serial.print("State:   ");
    Serial.println(getStateName(automaton->state()->id()));
    
    Serial.print("Water: ");
    Serial.print(getSensorStatusName(context->op->water.sensorStatus));
    if (context->op->water.sensorStatus == SENSOR_OK) {
      Serial.print(", ");
      Serial.println(context->op->water.currentTemp / 100);
      Serial.print("°C");
    }
    Serial.println();
    
    Serial.print("Ambient: ");
    Serial.print(getSensorStatusName(context->op->ambient.sensorStatus));
    if (context->op->ambient.sensorStatus == SENSOR_OK) {
      Serial.print(", ");
      Serial.println(context->op->ambient.currentTemp / 100);
      Serial.print("°C");
    }
    Serial.println();
  }
}

