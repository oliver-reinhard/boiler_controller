#include "control.h"

// #define DEBUG_CONTROL


uint32_t heatingTotalMillis(OperationalParams *op) {
  uint32_t duration = op->heatingAccumulatedMillis;
  if (op->heatingStartMillis != 0L) {
    duration += millis() - op->heatingStartMillis;
  }
  return duration;
}


uint8_t ControlActions::setupSensors() {
  context->op->water.setId(context->config->waterTempSensorId);
  context->op->water.rangeMin = WATER_MIN_TEMP;
  context->op->water.rangeMax = WATER_MAX_TEMP;
  
  context->op->ambient.setId(context->config->ambientTempSensorId);
  context->op->ambient.rangeMin = AMBIENT_MIN_TEMP;
  context->op->ambient.rangeMax = AMBIENT_MAX_TEMP;
  
  uint8_t matched = context->controller->setupSensors();
  
  if (context->op->water.sensorStatus == SENSOR_ID_UNDEFINED) {
   context->log->logMessage(MSG_WATER_TEMP_SENSOR_ID_UNDEF, 0, 0); 
   #ifdef DEBUG_CONTROL
     Serial.println(F("DEBUG_CONTROL: Water temp sensor ID undefined"));
   #endif
  } else if (context->op->water.sensorStatus == SENSOR_ID_AUTO_ASSIGNED) {
   context->log->logMessage(MSG_WATER_TEMP_SENSOR_ID_AUTO, 0, 0); 
   #ifdef DEBUG_CONTROL
     Serial.println(F("DEBUG_CONTROL: Water temp sensor ID assigned automatically."));
   #endif
  }
  
  if (context->op->ambient.sensorStatus == SENSOR_ID_UNDEFINED) {
   context->log->logMessage(MSG_AMBIENT_TEMP_SENSOR_ID_UNDEF, 0, 0); 
   #ifdef DEBUG_CONTROL
     Serial.println(F("DEBUG_CONTROL: Ambient temp sensor ID undefined"));
   #endif
  } else if (context->op->ambient.sensorStatus == SENSOR_ID_AUTO_ASSIGNED) {
   context->log->logMessage(MSG_AMBIENT_TEMP_SENSOR_ID_AUTO, 0, 0); 
   #ifdef DEBUG_CONTROL
     Serial.println(F("DEBUG_CONTROL: Ambient temp sensor ID assigned automatically."));
   #endif
  }

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

