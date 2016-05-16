#ifndef BOILER_UI_H_INCLUDED
  #define BOILER_UI_H_INCLUDED

  #include "control.h"
  #include "state.h"
  
  struct StatusNotification {
    StateID state;
    // time [s] since most recent transition to current state:
    unsigned long timeInState = 0L;
    SensorStatusID waterSensorStatus = SENSOR_INITIALISING;
    Temperature waterTemp = UNDEFINED_TEMPERATURE;
    SensorStatusID ambientSensorStatus = SENSOR_INITIALISING;
    Temperature ambientTemp = UNDEFINED_TEMPERATURE;
    // accumulated heating time [s] up to now:
    unsigned long heatingTime = 0L;
  };


  class AbstractUI {
    public:
      /*
       * Reads one user command and stores them in context->op.userCommand.
       */
      virtual void readUserCommand(ControlContext *context) =  0;
      
      virtual void processReadWriteRequests(ReadWriteRequests requests, ControlContext *context, BoilerStateAutomaton *automaton) =  0;
    
      virtual void notifyStatusChange(StatusNotification notification) =  0;
    
      virtual void notifyNewLogEntry(LogEntry entry) =  0;
  };

  
  class SerialUI : public AbstractUI {
    public:
    
      void readUserCommand(ControlContext *context);
      
      void processReadWriteRequests(ReadWriteRequests requests, ControlContext *context, BoilerStateAutomaton *automaton);
    
      void notifyStatusChange(StatusNotification notification);
    
      void notifyNewLogEntry(LogEntry entry);
  };
  

  
  class BLEUI : public AbstractUI {
    public:
    
      void readUserCommand(ControlContext *context);
      
      void processReadWriteRequests(ReadWriteRequests requests, ControlContext *context, BoilerStateAutomaton *automaton);
    
      void notifyStatusChange(StatusNotification notification);
    
      void notifyNewLogEntry(LogEntry entry);
  };
  
#endif
