#ifndef BC_UI_BLE_H_INCLUDED
  #define BC_UI_BLE_H_INCLUDED

  #include "ui.h"
  
  class BLEUI : public NullUI {
    public:
      BLEUI(ExecutionContext *context) : NullUI(context) { }
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
