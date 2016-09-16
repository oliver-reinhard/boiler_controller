#ifndef BOILER_UI_H_INCLUDED
  #define BOILER_UI_H_INCLUDED

  #include "control.h"
  #include "state.h"

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
    StateID state;
    UserCommands acceptedUserCommands;
    // time [s] since most recent transition to current state:
    TimeSeconds timeInState = 0L;
    // accumulated heating time [s] up to now:
    TimeSeconds heatingTime = 0L;
    TimeSeconds timeToGo = UNDEFINED_TIME_SECONDS;
    SensorStatusID waterSensorStatus = SENSOR_INITIALISING;
    Temperature waterTemp = UNDEFINED_TEMPERATURE;
    SensorStatusID ambientSensorStatus = SENSOR_INITIALISING;
    Temperature ambientTemp = UNDEFINED_TEMPERATURE;
  };


  class NullUI : public UserFeedback {
    public:
      NullUI(ExecutionContext *context) {
        this->context = context;
      }
      
      virtual void setup() { }
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
