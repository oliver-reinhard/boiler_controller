#ifndef BOILER_UI_SER_H_INCLUDED
  #define BOILER_UI_SER_H_INCLUDED

  #include "ui.h"
  
  class SerialUI : public NullUI {
    public:
      SerialUI(ControlContext *context) : NullUI(context) { }
      
      void setup();
      
      void readUserCommand();
      
      void processReadWriteRequests(ReadWriteRequests requests, BoilerStateAutomaton *automaton);
    
      void notifyStatusChange(StatusNotification notification);
    
      void notifyNewLogEntry(LogEntry entry);
  };
  
#endif
