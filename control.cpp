#include "control.h"
#include "message.h"

 void ControlActions::readSensors(ControlContext *context) {
   if (context->op->water.sensorStatus == SENSOR_INITIALISING && ! memcmp(context->config->waterTempSensor, &UNDEFINED_SENSOR_ID, TEMP_SENSOR_ID_BYTES)) {
     context->op->water.sensorStatus = SENSOR_ID_UNDEFINED;
     context->storage->logMessage(MSG_WATER_TEMP_SENSOR_ID_UNDEF, 0, 0);
   }

   
   if (context->op->ambient.sensorStatus == SENSOR_INITIALISING && ! memcmp(context->config->ambientTempSensor, &UNDEFINED_SENSOR_ID, TEMP_SENSOR_ID_BYTES)) {
     context->op->ambient.sensorStatus = SENSOR_ID_UNDEFINED;
     context->storage->logMessage(MSG_AMBIENT_TEMP_SENSOR_ID_UNDEF, 0, 0);
   }
 }

 void ControlActions::readUserCommands(ControlContext *context) {
    if (context == NULL) { } // prevent warning "unused parameter ..."
 }

  void ControlActions::heat(boolean on, ControlContext *context) {
    context->op->heating = on;
    if (on) {
      // turn heater on
    } else {
      // turn heater off
    }
  }

  void ControlActions::logValues(boolean on, ControlContext *context) {
    context->op->loggingValues = on;
    if (on) {
      // turn logging on
    } else {
      // turn logging off
    }
  }

  void ControlActions::setConfigParam() {
    // TODO
  }
  
  void ControlActions::getLog() {
    // TODO
  }
  
  void ControlActions::getConfig() {
    // TODO
  }
  
  void ControlActions::getStat() {
    // TODO
  }
