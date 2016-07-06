#include "bc_setup.h"
#include "math.h"
#include "control.h"
#include "state.h"
#include "ui.h"
#if defined SERIAL_UI
  #include "ui_ser.h"
#elif defined BLE_UI
  #include "ui_ble.h"
#elif defined UNIT_TEST
  #include <ArduinoUnit.h>
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
ConfigParams configParams = ConfigParams();
Log logger = Log(configParams.eepromSize()); 
OperationalParams opParams;
DS18B20TemperatureSensor *sensors[] = {&opParams.water, &opParams.ambient};
OneWire oneWire = OneWire(ONE_WIRE_PIN);  // on pin 10 (a 4.7K pull-up resistor to +5V is necessary)
DS18B20Controller controller = DS18B20Controller(&oneWire, sensors, 2);

ExecutionContext context;
ControlActions controlActions = ControlActions(&context);
BoilerStateAutomaton automaton = BoilerStateAutomaton(&context);

#if defined BLE_UI
  BLEUI ui = BLEUI(&context);
#elif defined SERIAL_UI
  SerialUI ui = SerialUI(&context);
#else 
  NullUI ui = NullUI(&context);
#endif


void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  #if defined UNIT_TEST
    //Test::min_verbosity = TEST_VERBOSITY_ALL;
    Serial.println(F("Unit Testing."));
    
  #else
    logger.init();
    logger.logMessage(MSG_SYSTEM_INIT, 0, 0);
    configParams.load(); 

    context.log = &logger;
    context.config = &configParams;
    context.op = &opParams;
    context.controller = &controller;
    context.control = &controlActions;

    pinMode(HEATER_PIN, OUTPUT);
    controlActions.setupSensors();
    
    Serial.println(F("Starting."));
  #endif
 
}

void loop() {
  
  #if defined UNIT_TEST
    Test::run();
    
  #else
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
      context.control->initSensorReadout();
      
    } else if (cycleStage == CYCLE_STAGE_1 && elapsed >= TEMP_SENSOR_READOUT_WAIT) {
      cycleStage = CYCLE_STAGE_2;
      context.control->completeSensorReadout();
      
    } else if (cycleStage == CYCLE_STAGE_2) {
      cycleStage = CYCLE_STAGE_3;
      logTemperatureValues(&context);
    }
    
    UserCommand command;  
    memset(command.args, 0, CMD_ARG_BUF_SIZE);
    context.op->command = &command;
    ui.readUserCommand();
  
    EventCandidates cand = automaton.evaluate();
    if (cand != EVENT_NONE) {
      EventEnum event = processEventCandidates(cand);
      if (event != EVENT_NONE) {
        automaton.transition(event);
        
        ui.processReadWriteRequests(context.control->getPendingReadWriteRequests(), &automaton);
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
        Flags flags = (context->op->water.sensorStatus<<4) | (context->op->ambient.sensorStatus);
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

  NotifyProperties notify = NOTIFY_NONE;

  if (now - notificationTimeMillis >= MAX_USER_NOTIFICATION_INTERVAL) {
    #ifdef DEBUG_MAIN
      Serial.println(F("DEBUG_MAIN: notify status case 1"));
    #endif
    notify |= NOTIFY_TIME_IN_STATE | NOTIFY_TIME_HEATING;
  }
  
  if (notification.state != automaton->state()->id()) {
    notification.state = automaton->state()->id();
    #ifdef DEBUG_MAIN
      Serial.println(F("DEBUG_MAIN: notify status case 2"));
    #endif
    notify |= NOTIFY_STATE;
  }

  if (notification.waterSensorStatus != context->op->water.sensorStatus
    || abs(notification.waterTemp - context->op->water.currentTemp) > NOTIFICATION_TEMP_DELTA
  ) {
    notification.waterSensorStatus = context->op->water.sensorStatus;
    notification.waterTemp = context->op->water.currentTemp;
    #ifdef DEBUG_MAIN
      Serial.println(F("DEBUG_MAIN: notify status case 3"));
    #endif
    notify |= NOTIFY_WATER_SENSOR;
  }

  if (notification.ambientSensorStatus != context->op->ambient.sensorStatus
    || abs(notification.ambientTemp - context->op->ambient.currentTemp) > NOTIFICATION_TEMP_DELTA
  ) {
    notification.ambientSensorStatus = context->op->ambient.sensorStatus;
    notification.ambientTemp = context->op->ambient.currentTemp;
    #ifdef DEBUG_MAIN
      Serial.println(F("DEBUG_MAIN: notify status case 4"));
    #endif
    notify |= NOTIFY_AMBIENT_SENSOR;
  }

  if (notify) {
    notification.timeInState = now - context->op->currentStateStartMillis / 1000L;
    notification.heatingTime = heatingTotalMillis(context->op) / 1000L;
    notify |= NOTIFY_TIME_IN_STATE | NOTIFY_TIME_HEATING;
    
    notificationTimeMillis = now;
    ui.notifyStatusChange(&notification);
  }
}

void checkForNewLogEntries(ControlContext *context) {
  context->log->readUnnotifiedLogEntries();
  LogEntry e;
  while (context->log->nextLogEntry(e)) {
    ui.notifyNewLogEntry(e);
  }
}

