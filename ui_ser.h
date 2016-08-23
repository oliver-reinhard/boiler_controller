#ifndef BC_UI_SER_H_INCLUDED
  #define BC_UI_SER_H_INCLUDED

  #include "ui.h"
  
  class SerialUI : public NullUI {
    public:
      SerialUI(ExecutionContext *context) : NullUI(context) { }
      
      void readUserRequest();
      
      void commandExecuted(boolean success);
      
      void provideUserInfo(BoilerStateAutomaton *automaton);
    
      void notifyStatusChange(StatusNotification *notification);
    
      void notifyNewLogEntry(LogEntry entry);
  };
  
#endif
