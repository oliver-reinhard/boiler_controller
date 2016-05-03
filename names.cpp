#include "names.h"

String getName(ConfigParamEnum literal) {
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

String getName(StateEnum literal) {
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

String getName(EventEnum literal) {
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

String getName(UserCommandEnum literal) {
  switch(literal) {
    case CMD_NONE: return "None";
    case CMD_SET_CONFIG: return "Set Config";
    case CMD_REC_ON: return "Rec On";
    case CMD_REC_OFF: return "Rec Off";
    case CMD_GET_LOG: return "Get Log";
    case CMD_GET_CONFIG: return "Get Config";
    case CMD_GET_STAT: return "Get Stat";
    case CMD_HEAT_ON: return "Heat On";
    case CMD_HEAT_OFF: return "Heat Off";
    case CMD_RESET: return "Reset";
    default: return "Undefined User Command";
  }
}

String getName(SensorStatusEnum literal) {
  switch(literal) {
    case SENSOR_INITIALISING: return "Init";
    case SENSOR_OK: return "OK";
    case SENSOR_NOK: return "NOK";
    case SENSOR_ID_UNDEFINED: return "No ID";
    default: return "Undefined Sensor Status";
  }
}

