#ifndef BOILER_UI_H_INCLUDED
  #define BOILER_UI_H_INCLUDED

  #include "control.h"
  #include "state.h"
      
  struct StatusNotification {
    StateID state;
    // time [s] since most recent transition to current state:
    uint32_t timeInState = 0L;
    SensorStatusID waterSensorStatus = SENSOR_INITIALISING;
    Temperature waterTemp = UNDEFINED_TEMPERATURE;
    SensorStatusID ambientSensorStatus = SENSOR_INITIALISING;
    Temperature ambientTemp = UNDEFINED_TEMPERATURE;
    // accumulated heating time [s] up to now:
    uint32_t heatingTime = 0L;
  };


  class NullUI {
    public:
      NullUI(ControlContext *context) {
        this->context = context;
      }
      
      virtual void setup() { }
      /*
       * Reads one user command and stores them in context->op.userCommand.
       */
      virtual void readUserCommand() { }
      
      virtual void processReadWriteRequests(ReadWriteRequests, BoilerStateAutomaton *) {  }
    
      virtual void notifyStatusChange(StatusNotification) { }
    
      virtual void notifyNewLogEntry(LogEntry) { }
   
    protected:
      ControlContext *context;
  };  

  
  #ifdef BLE_UI
  
    class BLEUI : public NullUI {
      public:
        BLEUI(ControlContext *context) : NullUI(context) { }
        void setup();
      
        void readUserCommand();
        
        void processReadWriteRequests(ReadWriteRequests requests, BoilerStateAutomaton *automaton);
      
        void notifyStatusChange(StatusNotification notification);
      
        void notifyNewLogEntry(LogEntry entry);

      protected:
        int32_t bcServiceId;
        int32_t statusCharacteristicId;  // read + notify
        int32_t logCharacteristicId;     // read + notify
        int32_t cmdCharacteristicId;     // write
    };
  #endif
#endif
