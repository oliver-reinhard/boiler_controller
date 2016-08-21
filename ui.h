#ifndef BOILER_UI_H_INCLUDED
  #define BOILER_UI_H_INCLUDED

  #include "control.h"
  #include "state.h"

  typedef enum {
    NOTIFY_NONE = 0x0,
    NOTIFY_STATE = 0x1,
    NOTIFY_TIME_IN_STATE = 0x2,
    NOTIFY_TIME_HEATING = 0x4,
    NOTIFY_WATER_SENSOR = 0x8,
    NOTIFY_AMBIENT_SENSOR = 0x10
  } NotifyPropertyEnum;

  // bitwise "OR" of NotifyPropertyEnum literals:
  typedef uint16_t NotifyProperties;
  
  struct StatusNotification {
    // the bits of notifyProperties tell which values have changed:
    NotifyProperties notifyProperties;
    StateID state;
    UserCommands userCommands;
    // time [s] since most recent transition to current state:
    uint32_t timeInState = 0L;
    // accumulated heating time [s] up to now:
    uint32_t heatingTime = 0L;
    SensorStatusID waterSensorStatus = SENSOR_INITIALISING;
    Temperature waterTemp = UNDEFINED_TEMPERATURE;
    SensorStatusID ambientSensorStatus = SENSOR_INITIALISING;
    Temperature ambientTemp = UNDEFINED_TEMPERATURE;
  };


  class NullUI {
    public:
      NullUI(ExecutionContext *context) {
        this->context = context;
      }
      
      virtual void setup() { }
      /*
       * Read one user command and store it in the context->op->command struct
       */
      virtual void readUserCommand() { }
      
      virtual void processReadWriteRequests(ReadWriteRequests, BoilerStateAutomaton *) {  }
    
      virtual void notifyStatusChange(StatusNotification *) { }
    
      virtual void notifyNewLogEntry(LogEntry) { }
   
    protected:
      ExecutionContext *context;
  };  

#endif
