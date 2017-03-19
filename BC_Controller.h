#include <ACF_FRAM.h>
#include <BC_Control.h>
#include "BC_UI.h"

// #define DEBUG_MAIN

// If the controller doesn't want to proceed out of state INIT (0), or if the serial command line does not process user input, then 
// - uncomment the '#define ERASE_CONFIG' line
// - upload the sketch
// - 'stat' to see that sensor ids are 'Auto-Assigned'
// - 'config ack ids'
// - 'stat' to see that the *individual* sensor ids are 'OK'
// - 'config' to see that sensor ids are not 00-00-00...
// - comment the '#define ERASE_CONFIG' line
// - upload the sketch
// 
// #define ERASE_CONFIG

#define CONTROL_CYCLE_DURATION          5000L // [ms]
#define TEMP_SENSOR_READOUT_WAIT         800L // [ms] = 750 ms + safety margin
#define MIN_USER_NOTIFICATION_INTERVAL  1000L // [ms] (notification only happens if relevant changes occurred)
#define MAX_USER_NOTIFICATION_INTERVAL 10000L // [ms] notify user after this period at the latest
#define NOTIFICATION_TEMP_DELTA           20  // [°C * 100]

enum class CycleStage {
  STAGE_0 = 0,
  STAGE_1 = 1,
  STAGE_2 = 2,
  STAGE_3 = 3
};


class BC_Controller {
  protected:

    FRAMStore configStore = FRAMStore(sizeof(ConfigParams));
    FRAMStore logStore = FRAMStore(&configStore, 1024);
    
    ConfigParams configParams = ConfigParams(&configStore);
    Log logger = Log(&logStore); 
    
    OperationalParams opParams = OperationalParams();
    
    DS18B20_Sensor *sensors[2] = {&opParams.water, &opParams.ambient};
    
    OneWire oneWire = OneWire(ONE_WIRE_PIN);  // on pin 10 (a 4.7K pull-up resistor to +5V is necessary)
    DS18B20_Controller controller = DS18B20_Controller(&oneWire, sensors, 2);
    
    ExecutionContext context = ExecutionContext();
    BoilerStateAutomaton automaton = BoilerStateAutomaton();
    
    ControlActions *controlActions = NULL;
    AbstractUI *ui = NULL;

  public:
    BC_Controller() {}
    
    void init(AbstractUI *ui) {
      this->ui = ui;
      
      bool connected = configStore.init();
      if(!connected) {
        // program halts, never returns:
        write_S_O_S(F("FRAM not connected"), __LINE__);
      }
      
      logger.init();
      logger.logMessage(MSG_SYSTEM_INIT, 0, 0);
    
      // Erease the config params (do this e.g. after the physical layout has changed)
      #ifdef ERASE_CONFIG
        configParams.reset(); 
      #endif
      
      configParams.load(); 
      
      controlActions = new ControlActions(&context, ui);
    
      context.log = &logger;
      context.config = &configParams;
      context.op = &opParams;
      context.controller = &controller;
      context.control = controlActions;
    
      automaton.init(&context);
      
      pinMode(HEATER_PIN, OUTPUT);
      context.control->setupSensors();
      
      context.op->request.clear();
      
      ui->init(&context);
    }  

    void loop() {
      static TimeMills controlCycleStart = 0L;
      static CycleStage cycleStage = CycleStage::STAGE_0;
      static TimeMills lastUserNotificationCheck = 0L;
      
      TimeMills now = millis();
      TimeMills elapsed = now - controlCycleStart;
      if (controlCycleStart == 0L || elapsed >= CONTROL_CYCLE_DURATION) {
        controlCycleStart = now;
        elapsed = 0L;
        cycleStage = CycleStage::STAGE_0;
      }
    
      if (cycleStage == CycleStage::STAGE_0 && elapsed == 0L) {
        cycleStage = CycleStage::STAGE_1;
        context.control->initSensorReadout();
        
      } else if (cycleStage == CycleStage::STAGE_1 && elapsed >= TEMP_SENSOR_READOUT_WAIT) {
        cycleStage = CycleStage::STAGE_2;
        context.control->completeSensorReadout();
        
      } else if (cycleStage == CycleStage::STAGE_2) {
        cycleStage = CycleStage::STAGE_3;
        logTemperatureValues(&context);
      }
    
      if (context.op->request.command == CMD_NONE) {
        ui->readUserRequest();
        context.op->request.event = automaton.commandToEvent(context.op->request.command);
      }
    
      EventSet cand = automaton.evaluate(context.op->request.event);
      if (cand != Events::NONE) {
        Event event = processEventCandidates(cand);
        if (event != Events::NONE) {
          #ifdef DEBUG_MAIN
            Serial.print(F("DEBUG_MAIN: Processing event: " );
            Serial.println(event.name());
          #endif
          
          automaton.transition(event);
          
          #ifdef DEBUG_MAIN
            Serial.print(F("DEBUG_MAIN: Processed event: " );
            Serial.println(event.name());
          #endif
    
          if (event == context.op->request.event) {
            // the user's command was chosen as the event with the highest priority
            
            if (context.op->request.event == Events::INFO) {
              ui->provideUserInfo(&automaton);
            }
          }
        }
      }
      
      context.op->request.clear();
    
      if (now - lastUserNotificationCheck >= MIN_USER_NOTIFICATION_INTERVAL) {
        checkForStatusChange(&context, &automaton, now);
        checkForNewLogEntries(&context);
        lastUserNotificationCheck = now;
      }
      
      delay(100);
    }

  protected:
    Event processEventCandidates(EventSet candidates) {
      #ifdef DEBUG_MAIN
        Serial.print(F("DEBUG_MAIN: evaluation yields event candidates: 0x"));
        Serial.println(candidates.events(), HEX);
      #endif
      
      for(uint16_t i=0; i< Events::NUM_EVENTS; i++) {
        #ifdef DEBUG_MAIN
          Serial.print(F("DEBUG_MAIN: event(i): 0x"));
          Serial.println(Events::EVENT_PRIORITIES[i]->id(), HEX);
        #endif
        if (candidates & *Events::EVENT_PRIORITIES[i]) {
          return *Events::EVENT_PRIORITIES[i];
        }
      }
      return Events::NONE;
    }
    
    
    /*
     * Checks whether
     * - logging is turned on or off
     * - values have changed sufficiently to warrant logging (context->config->logTempDelta)
     * - enough time has elapsed for a new logging record (context->config->logTimeDelta)
     */
    void logTemperatureValues(ExecutionContext *context) {
      if (context->op->loggingValues) {
        TimeMills time = millis();
    
        if (time - context->op->water.lastLoggedTime > context->config->logTimeDelta * 1000L || time - context->op->ambient.lastLoggedTime > context->config->logTimeDelta * 1000L) {
          boolean logValuesNow = false;
          ACF_Temperature water = ACF_UNDEFINED_TEMPERATURE;
          ACF_Temperature ambient = ACF_UNDEFINED_TEMPERATURE;
          
          if (context->op->water.sensorStatus == DS18B20_SENSOR_OK && abs(context->op->water.currentTemp - context->op->water.lastLoggedTemp) >= context->config->logTempDelta) {
            logValuesNow = true;
            water = context->op->water.currentTemp;
            
          } else if (context->op->water.sensorStatus == DS18B20_SENSOR_NOK && context->op->water.lastLoggedTemp != ACF_UNDEFINED_TEMPERATURE) {
            logValuesNow = true;;
          }
          
          if (context->op->ambient.sensorStatus == DS18B20_SENSOR_OK && abs(context->op->ambient.currentTemp - context->op->ambient.lastLoggedTemp) >= context->config->logTempDelta) {
            logValuesNow = true;
            ambient = context->op->ambient.currentTemp;
            
          } else if (context->op->ambient.sensorStatus == DS18B20_SENSOR_NOK && context->op->ambient.lastLoggedTemp != ACF_UNDEFINED_TEMPERATURE) {
            logValuesNow = true;
          }
          
          if (logValuesNow) {
            T_Flags flags = (context->op->water.sensorStatus<<4) | (context->op->ambient.sensorStatus);
            context->log->logValues(context->op->water.currentTemp, context->op->ambient.currentTemp, flags);
            context->op->water.lastLoggedTemp = water;
            context->op->water.lastLoggedTime = time;
            context->op->ambient.lastLoggedTemp = ambient;
            context->op->ambient.lastLoggedTime = time;
          }
        }
      }
    }
    
    
    void checkForStatusChange(ExecutionContext *context, BoilerStateAutomaton *automaton, TimeMills now) {
      // timepoint [ms] when this (= most recent) notification was sent to user:
      static TimeMills notificationTimeMillis;
      static StatusNotification notification;
    
      NotifyProperties notify = NOTIFY_NONE;
      StateID currentState = automaton->state()->id();
      
      if (notification.state != currentState) {
        notification.state = currentState;
        notification.acceptedUserCommands = automaton->acceptedUserCommands();
        #ifdef DEBUG_MAIN
          Serial.println(F("DEBUG_MAIN: notify status case 1"));
        #endif
        notify |= NOTIFY_STATE;
        notify |= NOTIFY_TIME_IN_STATE;
      }
    
      if (now - notificationTimeMillis >= MAX_USER_NOTIFICATION_INTERVAL) {
        #ifdef DEBUG_MAIN
          Serial.println(F("DEBUG_MAIN: notify status case 2"));
        #endif
        notify |= NOTIFY_TIME_IN_STATE;
        notify |= NOTIFY_TIME_TO_GO;
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
        notify |= NOTIFY_TIME_TO_GO;
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
    
      if (notify & NOTIFY_TIME_IN_STATE) {
        // currentStateStartMillis is set at state change which can be later than when 'now' was set => negative time:
        notification.timeInState = now < context->op->currentStateStartMillis ? 0L : (now - context->op->currentStateStartMillis) / 1000L;
        notificationTimeMillis = now;
        
        TimeSeconds heatingTotalTime = heatingTotalMillis(context->op) / 1000L;
        if (heatingTotalTime != notification.heatingTime) {
          notification.heatingTime = heatingTotalTime;
          #ifdef DEBUG_MAIN
            Serial.println(F("DEBUG_MAIN: notify status case 5"));
          #endif
          notify |= NOTIFY_TIME_HEATING;
        }
      }
    
      if (notify & NOTIFY_TIME_TO_GO) {
        TimeSeconds timeToGo;
        if (currentState == States::IDLE || (currentState == States::STANDBY && heatingTotalMillis(context->op) == 0)) { // we're recording but haven't started heating yet
          // the original time to go is only calculated in state IDLE:
          context->op->originalTimeToGo = context->originalTimeToGo();
          timeToGo = context->op->originalTimeToGo;
        } else if (NOTIFY_TIME_HEATING) {
          timeToGo = context->op->originalTimeToGo - notification.heatingTime;
        } else {
          timeToGo = context->op->originalTimeToGo;
        }
    
        if (timeToGo != notification.timeToGo) {
          notification.timeToGo = timeToGo;
        } else {
          // don't notify:
          notify &= ~NOTIFY_TIME_TO_GO;
        }
      }
    
      if (notify) {
        notification.notifyProperties = notify;
        ui->notifyStatusChange(&notification);
      }
    }
    
    void checkForNewLogEntries(ControlContext *context) {
      context->log->readUnnotifiedLogEntries();
      LogEntry e;
      while (context->log->nextLogEntry(e)) {
        ui->notifyNewLogEntry(e);
      }
    }
};

