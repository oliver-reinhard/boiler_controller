#include "control.h"

// #define DEBUG_CONTROL


void OperationalParams::swapTempSensorIDs() {
  TempSensorID tempId;
  memcpy(tempId, water.id, TEMP_SENSOR_ID_BYTES);
  water.setId(ambient.id);
  ambient.setId(tempId);
  
  SensorStatusEnum tempStatus = water.sensorStatus;
  water.sensorStatus = ambient.sensorStatus;
  ambient.sensorStatus = tempStatus;      
}
    
uint32_t heatingTotalMillis(OperationalParams *op) {
  uint32_t duration = op->heatingAccumulatedMillis;
  if (op->heatingStartMillis != 0L) {
    duration += millis() - op->heatingStartMillis;
  }
  return duration;
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


void ControlActions::setConfigParam() {
  pendingRequests |= WRITE_CONFIG;
}

void ControlActions::requestHelp() {
  pendingRequests |= READ_HELP;
}

void ControlActions::requestLog() {
  pendingRequests |= READ_LOG;
}

void ControlActions::requestConfig() {
  pendingRequests |= READ_CONFIG;
}

void ControlActions::requestStat() {
  pendingRequests |= READ_STAT;
}

ReadWriteRequests ControlActions::getPendingReadWriteRequests() {
  return pendingRequests;
}

void ControlActions::clearPendingReadWriteRequests() {
  pendingRequests = READ_WRITE_NONE;
}

