#include "control.h"

// #define DEBUG_CONTROL

uint32_t heatingTotalMillis(OperationalParams *op) {
  uint32_t duration = op->heatingAccumulatedMillis;
  if (op->heatingStartMillis != 0L) {
    duration += millis() - op->heatingStartMillis;
  }
  return duration;
}

/*
 * OPERATIONAL PARAMS
 */

void OperationalParams::swapTempSensorIDs() {
  TempSensorID tempId;
  memcpy(tempId, water.id, TEMP_SENSOR_ID_BYTES);
  water.setId(ambient.id);
  ambient.setId(tempId);
  
  SensorStatusEnum tempStatus = water.sensorStatus;
  water.sensorStatus = ambient.sensorStatus;
  ambient.sensorStatus = tempStatus;      
}


/*
 * CONTROL ACTIONS
 */

void ControlActions::modifyConfig() {
  UserRequest *request = &(context->op->request);
  UserCommandEnum command = request->command;
  if (command == CMD_CONFIG_SET_VALUE) {
    boolean success = setConfigParamValue(request->param, request->intValue, request->floatValue);
    if (success) {
      context->config->save();
    }
    userFeedback->commandExecuted(success);
    
  } else if (command == CMD_CONFIG_SWAP_IDS) {
    swapTempSensorIDs();
    userFeedback->commandExecuted(true);
    
  } else if (command == CMD_CONFIG_CLEAR_IDS) {
    clearTempSensorIDs();
    userFeedback->commandExecuted(true);
    
  } else if (command == CMD_CONFIG_ACK_IDS) {
    boolean success = confirmTempSensorIDs();
    userFeedback->commandExecuted(success);
    
  } else {
    userFeedback->commandExecuted(false);
  }
}
 
boolean ControlActions::setConfigParamValue(ConfigParamEnum p, int32_t intValue, float floatValue) {
  ConfigParams *config = context->config;
  Log *log = context->log;
  switch(p) {
    case PARAM_TARGET_TEMP: 
      config->targetTemp = (Temperature) intValue;
      log->logConfigParam(p, (float) config->targetTemp);
      return true;
      
    case PARAM_HEATER_CUT_OUT_WATER_TEMP: 
      config->heaterCutOutWaterTemp = (Temperature) intValue;
      log->logConfigParam(p, (float) config->heaterCutOutWaterTemp);
      return true;
      
    case PARAM_HEATER_BACK_OK_WATER_TEMP: 
      config->heaterBackOkWaterTemp = (Temperature)intValue;
      log->logConfigParam(p, (float) config->heaterBackOkWaterTemp);
      return true;
      
    case PARAM_LOG_TEMP_DELTA: 
      config->logTempDelta = (Temperature) intValue;
      log->logConfigParam(p, (float) config->logTempDelta);
      return true;
      
    case PARAM_LOG_TIME_DELTA:
      config->logTimeDelta = (uint16_t) intValue;
      log->logConfigParam(p, (float) config->logTimeDelta);
      return true;
      
    case PARAM_TANK_CAPACITY:
      config->tankCapacity = floatValue;
      log->logConfigParam(p, config->tankCapacity);
      return true;
      
    case PARAM_HEATER_POWER:
      config->heaterPower = floatValue;
      log->logConfigParam(p, config->heaterPower);
      return true;
      
    default:
      return false;
  }
}

void ControlActions::swapTempSensorIDs() {
  context->op->swapTempSensorIDs();
}

void ControlActions::clearTempSensorIDs() {
  context->config->setTempSensorIDs((uint8_t*) UNDEFINED_SENSOR_ID, (uint8_t*) UNDEFINED_SENSOR_ID);
  context->config->save();
  context->log->logConfigParam(PARAM_WATER_TEMP_SENSOR_ID, 0.0);
  context->log->logConfigParam(PARAM_AMBIENT_TEMP_SENSOR_ID, 0.0);
  context->log->logMessage(MSG_TEMP_SENSOR_IDS_CLEARED, 0, 0);
}

boolean ControlActions::confirmTempSensorIDs() {
  if (context->op->water.sensorStatus == SENSOR_ID_AUTO_ASSIGNED || context->op->ambient.sensorStatus == SENSOR_ID_AUTO_ASSIGNED) {
    context->config->setTempSensorIDs(context->op->water.id, context->op->ambient.id);
    context->config->save();
    context->log->logConfigParam(PARAM_WATER_TEMP_SENSOR_ID, 0.0);
    context->log->logConfigParam(PARAM_AMBIENT_TEMP_SENSOR_ID, 0.0);
    
    context->op->water.confirmId();
    context->op->ambient.confirmId();
    return true;
  }
  return false;
}


uint8_t ControlActions::setupSensors() {
  DS18B20TemperatureSensor *waterSensor = &context->op->water;
  waterSensor->setId(context->config->waterTempSensorId);
  waterSensor->rangeMin = WATER_MIN_TEMP;
  waterSensor->rangeMax = WATER_MAX_TEMP;
  
  DS18B20TemperatureSensor *ambientSensor = &context->op->ambient;
  ambientSensor->setId(context->config->ambientTempSensorId);
  ambientSensor->rangeMin = AMBIENT_MIN_TEMP;
  ambientSensor->rangeMax = AMBIENT_MAX_TEMP;
  
  uint8_t matched = context->controller->setupSensors();
  
  if (waterSensor->sensorStatus == SENSOR_INITIALISING) {
    logSensorSetupIssue(waterSensor, MSG_WATER_TEMP_SENSOR_SILENT); 
  } else if (waterSensor->sensorStatus == SENSOR_ID_UNDEFINED) {
    logSensorSetupIssue(waterSensor, MSG_WATER_TEMP_SENSOR_ID_UNDEF); 
  } else if (waterSensor->sensorStatus == SENSOR_ID_AUTO_ASSIGNED) {
    logSensorSetupIssue(waterSensor, MSG_WATER_TEMP_SENSOR_ID_AUTO); 
  }
  
  if (ambientSensor->sensorStatus == SENSOR_INITIALISING) {
    logSensorSetupIssue(ambientSensor, MSG_AMBIENT_TEMP_SENSOR_SILENT); 
  } else if (ambientSensor->sensorStatus == SENSOR_ID_UNDEFINED) {
    logSensorSetupIssue(ambientSensor, MSG_AMBIENT_TEMP_SENSOR_ID_UNDEF); 
  } else if (ambientSensor->sensorStatus == SENSOR_ID_AUTO_ASSIGNED) {
   logSensorSetupIssue(ambientSensor, MSG_AMBIENT_TEMP_SENSOR_ID_AUTO); 
  } 

   #ifdef DEBUG_CONTROL
     Serial.print(F("DEBUG_CONTROL: Sensors matched or found = "));
     Serial.println(matched);
   #endif
  return matched;
}
void ControlActions::initSensorReadout() {
  context->controller->initSensorReadout();
}


void ControlActions::completeSensorReadout() {
  context->controller->completeSensorReadout();
}

void ControlActions::heat(boolean on) {
  context->op->heating = on;
  if (on) {
    digitalWrite(HEATER_PIN, HIGH);
  } else {
    digitalWrite(HEATER_PIN, LOW);
  }
}

void ControlActions::logSensorSetupIssue(DS18B20TemperatureSensor *sensor, ControlMessageEnum msg) {
  context->log->logMessage(msg, 0, 0); 
  #ifdef DEBUG_CONTROL
    Serial.print(F("DEBUG_CONTROL: "));
    Serial.print(sensor->label);
    Serial.print(F(" sensor status = "));
    Serial.println(sensor->sensorStatus);
  #else
    if (sensor != NULL) { } // prevent 'unused parameter' warning
  #endif
}
