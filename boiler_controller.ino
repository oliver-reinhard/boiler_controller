#include "unit_test.h"
#ifdef UNIT_TEST
  #include <ArduinoUnit.h>
#endif
#include "math.h"
//#include "storage.h"
#include "control.h"
#include "state.h"
#include "test_state.h"

/*
 * GLOBALS
 */
Storage storage = Storage();
ConfigParams configParams;
OperationalParams opParams;
ControlActions controlActions = ControlActions();
ExecutionContext context;

BoilerStateAutomaton automaton = BoilerStateAutomaton(&context);

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  #ifndef UNIT_TEST
    Serial.println("Boiler setup MAIN");
    
    storage.getConfigParams(&configParams);
    storage.initLog();
    storage.logMessage(MSG_SYSTEM_INIT, 0, 0);

    context.storage = &storage;
    context.config = &configParams;
    context.op = &opParams;
    context.control = &controlActions;

    //
    // REMOVE AGAIN
    //
    TempSensorID water = {1,1,1,1,1,1,1,1};
    memcpy(&configParams.waterTempSensor, &water, TEMP_SENSOR_ID_BYTES);
    TempSensorID ambient = {2,2,2,2,2,2,2,2};
    memcpy(&configParams.ambientTempSensor, &ambient, TEMP_SENSOR_ID_BYTES);
    storage.updateConfigParams(&configParams);
    
  #endif
 
}

void loop() {
  #ifdef UNIT_TEST
    Test::run();
  #endif
  
  #ifndef UNIT_TEST
    context.control->readSensors(&context);
    context.control->readUserCommands(&context);
    
    EventCandidates cand = automaton.evaluate();
    if (cand != EVENT_NONE) {
      EventEnum event = processEventCandidates(cand);
      automaton.transition(event);
    }

    if (context.op->loggingValues) {
      logValues(&context);
    }

    delay(1000);
  #endif
}

EventEnum processEventCandidates(EventCandidates cand) {
  for(unsigned int i; i< NUM_EVENTS; i++) {
    if (cand & EVENT_PRIORITIES[i]) {
      return EVENT_PRIORITIES[i];
    }
  }
  return EVENT_NONE;
}

void logValues(ExecutionContext *context) {
  boolean logValues = false;
  Temperature water = UNDEFINED_TEMPERATURE;
  Temperature ambient = UNDEFINED_TEMPERATURE;
  
  if (context->op->water.sensorStatus == SENSOR_OK && abs(context->op->water.currentTemp - context->op->water.lastLoggedTemp) >= context->config->logTempDelta) {
    logValues = true;
    water = context->op->water.currentTemp;
    
  } else if (context->op->water.sensorStatus == SENSOR_NOK && context->op->water.lastLoggedTemp != UNDEFINED_TEMPERATURE) {
    logValues = true;;
  }
  
  if (context->op->ambient.sensorStatus == SENSOR_OK && abs(context->op->ambient.currentTemp - context->op->ambient.lastLoggedTemp) >= context->config->logTempDelta) {
    logValues = true;
    ambient = context->op->ambient.currentTemp;
    
  } else if (context->op->ambient.sensorStatus == SENSOR_NOK && context->op->ambient.lastLoggedTemp != UNDEFINED_TEMPERATURE) {
    logValues = true;
  }
  
  if (logValues) {
    Flags flags = context->op->water.sensorStatus<<4 | context->op->ambient.sensorStatus;
    Timestamp stamp = context->storage->logValues(context->op->water.currentTemp, context->op->ambient.currentTemp, flags);
    context->op->water.lastLoggedTemp = water;
    context->op->water.lastLoggedTime = stamp;
    context->op->ambient.lastLoggedTemp = ambient;
    context->op->ambient.lastLoggedTime = stamp;
  }
}

