#include "unit_test.h"
#ifdef UNIT_TEST
  #include <ArduinoUnit.h>
  #include "test_state.h"
#endif
#include "math.h"
#include "control.h"
#include "state.h"

#define CONTROL_CYCLE_DURATION   5000L  // [ms]
#define TEMP_SENSOR_READOUT_WAIT 800L   // [ms] = 750 ms + safety margin

/*
 * GLOBALS
 */
Storage storage = Storage();
ConfigParams configParams;
OperationalParams opParams;
ControlActions controlActions = ControlActions();
ExecutionContext context;

BoilerStateAutomaton automaton = BoilerStateAutomaton(&context);

unsigned long controlCycleStart = 0L;

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
    // REMOVE THIS
    //
    TempSensorID water   = {0x28, 0x8C, 0x8C, 0x79, 0x06, 0x00, 0x00, 0x89};
    memcpy(&configParams.waterTempSensorId, &water, TEMP_SENSOR_ID_BYTES);
    TempSensorID ambient = {0x28, 0x7C, 0x28, 0x79, 0x06, 0x00, 0x00, 0xD7};
    memcpy(&configParams.ambientTempSensorId, &ambient, TEMP_SENSOR_ID_BYTES);
    storage.updateConfigParams(&configParams);
    //
    // END REMOVE
    //

    pinMode(HEATER_PIN, OUTPUT);
    controlActions.setupSensors(&context);
  #endif
 
}

void loop() {
  unsigned long elapsed = millis() - controlCycleStart;
  if (controlCycleStart == 0L || elapsed > CONTROL_CYCLE_DURATION) {
    controlCycleStart = millis();
    elapsed = 0L;
  }
  
  #ifdef UNIT_TEST
    Test::run();
  #endif
  
  #ifndef UNIT_TEST
    if (elapsed == 0L) {
      context.control->initSensorReadout(&context);
    } else if (elapsed >= TEMP_SENSOR_READOUT_WAIT) {
      context.control->completeSensorReadout(&context);
      context.control->readUserCommands(&context);
    
      EventCandidates cand = automaton.evaluate();
      if (cand != EVENT_NONE) {
        EventEnum event = processEventCandidates(cand);
        automaton.transition(event);
      }
  
      controlActions.logValues(&context);

      delay(CONTROL_CYCLE_DURATION - elapsed + 1);
    }
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


