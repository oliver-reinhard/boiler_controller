#include "unit_test.h"
#ifdef UNIT_TEST
  #include <ArduinoUnit.h>
  #include "test_state.h"
#endif
#include "math.h"
#include "control.h"
#include "state.h"
#include "ui.h"

//#define DEBUG_MAIN

#define CONTROL_CYCLE_DURATION   5000L  // [ms]
#define TEMP_SENSOR_READOUT_WAIT 800L   // [ms] = 750 ms + safety margin

typedef enum {
  CYCLE_STAGE_0 = 0,
  CYCLE_STAGE_1 = 1,
  CYCLE_STAGE_2 = 2,
  CYCLE_STAGE_3 = 3
} CycleStageEnum;


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
CycleStageEnum cycleStage = CYCLE_STAGE_0;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  #ifndef UNIT_TEST    
    storage.getConfigParams(&configParams);
    storage.initLog();
    storage.logMessage(MSG_SYSTEM_INIT, 0, 0);

    context.storage = &storage;
    context.config = &configParams;
    context.op = &opParams;
    context.control = &controlActions;

    pinMode(HEATER_PIN, OUTPUT);
    controlActions.setupSensors(&context);
    
    Serial.println("Ready.");
  #endif
 
}

void loop() {
  unsigned long elapsed = millis() - controlCycleStart;
  if (controlCycleStart == 0L || elapsed > CONTROL_CYCLE_DURATION) {
    controlCycleStart = millis();
    elapsed = 0L;
    cycleStage = CYCLE_STAGE_0;
  }
  
  #ifdef UNIT_TEST
    Test::run();
  #endif
  
  #ifndef UNIT_TEST
    if (cycleStage == CYCLE_STAGE_0 && elapsed == 0L) {
      cycleStage = CYCLE_STAGE_1;
      context.control->initSensorReadout(&context);
      
    } else if (cycleStage == CYCLE_STAGE_1 && elapsed >= TEMP_SENSOR_READOUT_WAIT) {
      cycleStage = CYCLE_STAGE_2;
      context.control->completeSensorReadout(&context);
      
    } else if (cycleStage == CYCLE_STAGE_2) {
      cycleStage = CYCLE_STAGE_3;
      controlActions.logTemperatureValues(&context);
    }
    
    UserCommand command;  
    memset(command.args, 0, CMD_ARG_BUF_SIZE);
    context.op->command = &command;
    readUserCommand(&context);
  
    EventCandidates cand = automaton.evaluate();
    if (cand != EVENT_NONE) {
      EventEnum event = processEventCandidates(cand);
      if (event != EVENT_NONE) {
        automaton.transition(event);
        
        processReadWriteRequests(context.control->getPendingReadWriteRequests(), &context, &automaton);
        context.control->clearPendingReadWriteRequests();
      }
    }
    
    delay(100);
  #endif
}

EventEnum processEventCandidates(EventCandidates cand) {
  #ifdef DEBUG_MAIN
    Serial.print("DEBUG_MAIN: cand: 0x");
    Serial.println(cand, HEX);
  #endif
  for(unsigned int i=0; i< NUM_EVENTS; i++) {
    #ifdef DEBUG_MAIN
      Serial.print("DEBUG_MAIN: event(i): 0x");
      Serial.println(EVENT_PRIORITIES[i], HEX);
    #endif
    if (cand & EVENT_PRIORITIES[i]) {
      return EVENT_PRIORITIES[i];
    }
  }
  return EVENT_NONE;
}


