#ifndef BOILER_UI_SER_H_INCLUDED
  #define BOILER_UI_SER_H_INCLUDED

  #include "ui.h"
  
  class SerialUI : public AbstractUI {
    public:
      void setup(ControlContext *context);
      
      void readUserCommand(ControlContext *context);
      
      void processReadWriteRequests(ReadWriteRequests requests, ControlContext *context, BoilerStateAutomaton *automaton);
    
      void notifyStatusChange(StatusNotification notification);
    
      void notifyNewLogEntry(LogEntry entry);
  };
  
#endif
