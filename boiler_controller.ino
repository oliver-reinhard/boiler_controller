#include "b_setup.h"
#ifdef UNIT_TEST
  #include <ArduinoUnit.h>
  #include "test_state.h"
#endif
#include "math.h"
#include "control.h"
#include "state.h"
#include "ui.h"

//#define DEBUG_MAIN

#define CONTROL_CYCLE_DURATION      5000L  // [ms]
#define TEMP_SENSOR_READOUT_WAIT    800L   // [ms] = 750 ms + safety margin
#define USER_NOTIFICATION_INTERVAL  1000L  // [ms] (notification only happens is relevant changes occurred)

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

#ifdef BLE_UI
  BLEUI useUI = BLEUI();
#endif
#ifndef BLE_UI
  SerialUI useUI = SerialUI();
#endif

AbstractUI *ui = &useUI;


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
  
  #ifdef UNIT_TEST
    Test::run();
  #endif
  
  #ifndef UNIT_TEST
    static unsigned long controlCycleStart = 0L;
    static CycleStageEnum cycleStage = CYCLE_STAGE_0;
    static unsigned long lastUserNotification = 0L;
    
    unsigned long now = millis();
    unsigned long elapsed = now - controlCycleStart;
    if (controlCycleStart == 0L || elapsed > CONTROL_CYCLE_DURATION) {
      controlCycleStart = now;
      elapsed = 0L;
      cycleStage = CYCLE_STAGE_0;
    }
  
    if (cycleStage == CYCLE_STAGE_0 && elapsed == 0L) {
      cycleStage = CYCLE_STAGE_1;
      context.control->initSensorReadout(&context);
      
    } else if (cycleStage == CYCLE_STAGE_1 && elapsed >= TEMP_SENSOR_READOUT_WAIT) {
      cycleStage = CYCLE_STAGE_2;
      context.control->completeSensorReadout(&context);
      
    } else if (cycleStage == CYCLE_STAGE_2) {
      cycleStage = CYCLE_STAGE_3;
      logTemperatureValues(&context);
    }
    
    UserCommand command;  
    memset(command.args, 0, CMD_ARG_BUF_SIZE);
    context.op->command = &command;
    ui->readUserCommand(&context);
  
    EventCandidates cand = automaton.evaluate();
    if (cand != EVENT_NONE) {
      EventEnum event = processEventCandidates(cand);
      if (event != EVENT_NONE) {
        automaton.transition(event);
        
        ui->processReadWriteRequests(context.control->getPendingReadWriteRequests(), &context, &automaton);
        context.control->clearPendingReadWriteRequests();
      }
    }

    if (now - lastUserNotification >= USER_NOTIFICATION_INTERVAL) {
      notifyStatusChange(&context, &automaton);
      notifyNewLogEntry(&context);
      lastUserNotification = now;
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


/*
 * Checks whether
 * - logging is turned on or off
 * - values have changed sufficiently to warrant logging (context->config->logTempDelta)
 * - enough time has elapsed for a new logging record (context->config->logTimeDelta)
 */
void logTemperatureValues(ControlContext *context) {
  if (context->op->loggingValues) {
    unsigned long time = millis();

    if (time - context->op->water.lastLoggedTime > context->config->logTimeDelta * 1000L || time - context->op->ambient.lastLoggedTime > context->config->logTimeDelta * 1000L) {
      boolean logValuesNow = false;
      Temperature water = UNDEFINED_TEMPERATURE;
      Temperature ambient = UNDEFINED_TEMPERATURE;
      
      if (context->op->water.sensorStatus == SENSOR_OK && abs(context->op->water.currentTemp - context->op->water.lastLoggedTemp) >= context->config->logTempDelta) {
        logValuesNow = true;
        water = context->op->water.currentTemp;
        
      } else if (context->op->water.sensorStatus == SENSOR_NOK && context->op->water.lastLoggedTemp != UNDEFINED_TEMPERATURE) {
        logValuesNow = true;;
      }
      
      if (context->op->ambient.sensorStatus == SENSOR_OK && abs(context->op->ambient.currentTemp - context->op->ambient.lastLoggedTemp) >= context->config->logTempDelta) {
        logValuesNow = true;
        ambient = context->op->ambient.currentTemp;
        
      } else if (context->op->ambient.sensorStatus == SENSOR_NOK && context->op->ambient.lastLoggedTemp != UNDEFINED_TEMPERATURE) {
        logValuesNow = true;
      }
      
      if (logValuesNow) {
        Flags flags = context->op->water.sensorStatus<<4 | context->op->ambient.sensorStatus;
        context->storage->logValues(context->op->water.currentTemp, context->op->ambient.currentTemp, flags);
        context->op->water.lastLoggedTemp = water;
        context->op->water.lastLoggedTime = time;
        context->op->ambient.lastLoggedTemp = ambient;
        context->op->ambient.lastLoggedTime = time;
      }
    }
  }
}


void notifyStatusChange(ControlContext *context, BoilerStateAutomaton *automaton) {
  // timepoint [ms] when this (= most recent) notification was sent to user:
  static unsigned long notificationTimeMillis;
  static StatusNotification notification;

  unsigned long now = millis();
  boolean notify = false;

  if (now - notificationTimeMillis >= MAX_USER_NOTIFICATION_INTERVAL) {
    notification.timeInStateMillis = now - context->op->currentStateStartMillis;
    notification.heatingTimeMillis = heatingTotalMillis(context->op);
    notify = true;
  }
  
  if (notification.state != automaton->state()->id()) {
    notification.state = automaton->state()->id();
    notify = true;
  }

  if (notification.waterSensorStatus != context->op->water.sensorStatus
    || abs(notification.waterTemp - context->op->water.currentTemp) > NOTIFICATION_TEMP_DELTA
  ) {
    notification.waterSensorStatus = context->op->water.sensorStatus;
    notification.waterTemp = context->op->water.currentTemp;
    notify = true;
  }

  if (notification.ambientSensorStatus != context->op->ambient.sensorStatus
    || abs(notification.ambientTemp - context->op->ambient.currentTemp) > NOTIFICATION_TEMP_DELTA
  ) {
    notification.ambientSensorStatus = context->op->ambient.sensorStatus;
    notification.ambientTemp = context->op->ambient.currentTemp;
    notify = true;
  }

  if (notify) {
    ui->notifyStatusChange(notification);
  }
}

void notifyNewLogEntry(ControlContext *context) {
  if (context != NULL) { } // prevent 'unused parameter' warning
  LogEntry e;
  ui->notifyNewLogEntry(e);
}

