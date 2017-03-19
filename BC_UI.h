#ifndef BOILER_UI_H_INCLUDED
  #define BOILER_UI_H_INCLUDED

  #include <BC_Control.h>
  #include <BC_State.h>

  typedef enum {
    NOTIFY_NONE = 0x0,
    NOTIFY_STATE = 0x1,
    NOTIFY_TIME_IN_STATE = 0x2,
    NOTIFY_TIME_HEATING = 0x4,
    NOTIFY_TIME_TO_GO = 0x8,
    NOTIFY_WATER_SENSOR = 0x10,
    NOTIFY_AMBIENT_SENSOR = 0x20
  } NotifyPropertyEnum;

  // bitwise "OR" of NotifyPropertyEnum literals:
  typedef uint16_t NotifyProperties;
  
  struct StatusNotification {
    // the bits of notifyProperties tell which values have changed:
    NotifyProperties notifyProperties;
    StateID state = STATE_UNDEFINED;
    UserCommands acceptedUserCommands;
    // time [s] since most recent transition to current state:
    TimeSeconds timeInState = 0L;
    // accumulated heating time [s] up to now:
    TimeSeconds heatingTime = 0L;
    TimeSeconds timeToGo = UNDEFINED_TIME_SECONDS;
    DS18B20_StatusID waterSensorStatus = DS18B20_SENSOR_INITIALISING;
    ACF_Temperature waterTemp = ACF_UNDEFINED_TEMPERATURE;
    DS18B20_StatusID ambientSensorStatus = DS18B20_SENSOR_INITIALISING;
    ACF_Temperature ambientTemp = ACF_UNDEFINED_TEMPERATURE;
  };


  class AbstractUI : public UserFeedback {
    public:
      AbstractUI() : UserFeedback() {}
      
      virtual void init(ExecutionContext *context) {
        this->context = context;
      }
      
      /*
       * Read one user request and store it in the context->op->request struct
       */
      virtual void readUserRequest() { }

      /*
       * Passes information in response to an explicit user request.
       */
      virtual void provideUserInfo(BoilerStateAutomaton *) {  }
    
      virtual void notifyNewLogEntry(LogEntry) { }
      
      virtual void notifyStatusChange(StatusNotification *) { }
   
    protected:
      ExecutionContext *context;
  };  

#endif
