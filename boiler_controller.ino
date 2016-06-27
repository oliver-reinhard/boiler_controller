#include "bc_setup.h"
#ifdef UNIT_TEST
  #include <ArduinoUnit.h>
#endif
#include "math.h"
#include "control.h"
#include "state.h"
#include "ui.h"
#ifdef SERIAL_UI
  #include "ui_ser.h"
#endif

//#define DEBUG_MAIN

#define CONTROL_CYCLE_DURATION          5000L// [ms]
#define TEMP_SENSOR_READOUT_WAIT         800L // [ms] = 750 ms + safety margin
#define MIN_USER_NOTIFICATION_INTERVAL  1000L // [ms] (notification only happens if relevant changes occurred)
#define MAX_USER_NOTIFICATION_INTERVAL 10000L // [ms] notify user after this period at latest
#define NOTIFICATION_TEMP_DELTA           20  // [°C * 100]

typedef enum {
  CYCLE_STAGE_0 = 0,
  CYCLE_STAGE_1 = 1,
  CYCLE_STAGE_2 = 2,
  CYCLE_STAGE_3 = 3
} CycleStageEnum;


/*
 * GLOBALS
 */
ConfigParams config = ConfigParams();
Log logger = Log(sizeof(ConfigParams)); 
OperationalParams opParams;
ControlActions controlActions = ControlActions();

ExecutionContext context;
BoilerStateAutomaton automaton = BoilerStateAutomaton(&context);

#ifdef BLE_UI
  BLEUI ui = BLEUI();
#endif
#ifdef SERIAL_UI
  SerialUI ui = SerialUI();
#endif
#ifdef UNIT_TEST
  AbstractUI ui = AbstractUI();
#endif


void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  #ifdef UNIT_TEST
    //Test::min_verbosity = TEST_VERBOSITY_ALL;
    Serial.println(F("Unit Testing."));
  #endif
  
  #ifndef UNIT_TEST 
    logger.init();
    logger.logMessage(MSG_SYSTEM_INIT, 0, 0);
    config.load(); 

    context.log = &logger;
    context.config = &config;
    context.op = &opParams;
    context.control = &controlActions;

    pinMode(HEATER_PIN, OUTPUT);
    controlActions.setupSensors(&context);
    
    Serial.println(F("Ready."));
  #endif
 
}

void loop() {
  
  #ifdef UNIT_TEST
    Test::run();
  #endif
  
  #ifndef UNIT_TEST
    static uint32_t controlCycleStart = 0L;
    static CycleStageEnum cycleStage = CYCLE_STAGE_0;
    static uint32_t lastUserNotificationCheck = 0L;
    
    uint32_t now = millis();
    uint32_t elapsed = now - controlCycleStart;
    if (controlCycleStart == 0L || elapsed >= CONTROL_CYCLE_DURATION) {
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
    ui.readUserCommand(&context);
  
    EventCandidates cand = automaton.evaluate();
    if (cand != EVENT_NONE) {
      EventEnum event = processEventCandidates(cand);
      if (event != EVENT_NONE) {
        automaton.transition(event);
        
        ui.processReadWriteRequests(context.control->getPendingReadWriteRequests(), &context, &automaton);
        context.control->clearPendingReadWriteRequests();
      }
    }

    if (now - lastUserNotificationCheck >= MIN_USER_NOTIFICATION_INTERVAL) {
      checkForStatusChange(&context, &automaton, now);
      checkForNewLogEntries(&context);
      lastUserNotificationCheck = now;
    }
    
    delay(100);
  #endif
}

EventEnum processEventCandidates(EventCandidates cand) {
  #ifdef DEBUG_MAIN
    Serial.print(F("DEBUG_MAIN: evaluation yields event candidates: 0x"));
    Serial.println(cand, HEX);
  #endif
  
  for(uint16_t i=0; i< NUM_EVENTS; i++) {
    #ifdef DEBUG_MAIN
      Serial.print(F("DEBUG_MAIN: event(i): 0x"));
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
    uint32_t time = millis();

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
        context->log->logValues(context->op->water.currentTemp, context->op->ambient.currentTemp, flags);
        context->op->water.lastLoggedTemp = water;
        context->op->water.lastLoggedTime = time;
        context->op->ambient.lastLoggedTemp = ambient;
        context->op->ambient.lastLoggedTime = time;
      }
    }
  }
}


void checkForStatusChange(ControlContext *context, BoilerStateAutomaton *automaton, uint32_t now) {
  // timepoint [ms] when this (= most recent) notification was sent to user:
  static uint32_t notificationTimeMillis;
  static StatusNotification notification;

  boolean notify = false;

  if (now - notificationTimeMillis >= MAX_USER_NOTIFICATION_INTERVAL) {
    #ifdef DEBUG_MAIN
      Serial.println(F("DEBUG_MAIN: notify status case 1"));
    #endif
    notify = true;
  }
  
  if (notification.state != automaton->state()->id()) {
    notification.state = automaton->state()->id();
    #ifdef DEBUG_MAIN
      Serial.println(F("DEBUG_MAIN: notify status case 2"));
    #endif
    notify = true;
  }

  if (notification.waterSensorStatus != context->op->water.sensorStatus
    || abs(notification.waterTemp - context->op->water.currentTemp) > NOTIFICATION_TEMP_DELTA
  ) {
    notification.waterSensorStatus = context->op->water.sensorStatus;
    notification.waterTemp = context->op->water.currentTemp;
    #ifdef DEBUG_MAIN
      Serial.println(F("DEBUG_MAIN: notify status case 3"));
    #endif
    notify = true;
  }

  if (notification.ambientSensorStatus != context->op->ambient.sensorStatus
    || abs(notification.ambientTemp - context->op->ambient.currentTemp) > NOTIFICATION_TEMP_DELTA
  ) {
    notification.ambientSensorStatus = context->op->ambient.sensorStatus;
    notification.ambientTemp = context->op->ambient.currentTemp;
    #ifdef DEBUG_MAIN
      Serial.println(F("DEBUG_MAIN: notify status case 4"));
    #endif
    notify = true;
  }

  if (notify) {
    notification.timeInState = now - context->op->currentStateStartMillis / 1000L;
    notification.heatingTime = heatingTotalMillis(context->op) / 1000L;
    notificationTimeMillis = now;
    ui.notifyStatusChange(notification);
  }
}

void checkForNewLogEntries(ControlContext *context) {
  context->log->readUnnotifiedLogEntries();
  LogEntry e;
  while (context->log->nextLogEntry(e)) {
    ui.notifyNewLogEntry(e);
  }
}

