#ifndef BC_UI_SER_H_INCLUDED
  #define BC_UI_SER_H_INCLUDED

  #include "BC_UI.h"
  
  class ConsoleUI : public AbstractUI {
    public:
      ConsoleUI() : AbstractUI() { }
      
      void readUserRequest();
      
      void commandExecuted(boolean success);
      
      void provideUserInfo(BoilerStateAutomaton *automaton);
    
      void notifyStatusChange(StatusNotification *notification);
    
      void notifyNewLogEntry(LogEntry entry);
  };
  
#endif
