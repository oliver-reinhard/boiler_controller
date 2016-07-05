#ifndef BC_UI_BLE_H_INCLUDED
  #define BC_UI_BLE_H_INCLUDED

  #include "ui.h"
  #include "ble_gatt/Adafruit_BluefruitLE_GATT.h"  
  
  class BLEUI : public NullUI {
    public:
      BLEUI(ExecutionContext *context) : NullUI(context) { }
      void setup();
    
      void readUserCommand();
      
      void processReadWriteRequests(ReadWriteRequests requests, BoilerStateAutomaton *automaton);
    
      void notifyStatusChange(StatusNotification *notification);
    
      void notifyNewLogEntry(LogEntry entry);

    protected:
      Adafruit_BluefruitLE_GATT ble = Adafruit_BluefruitLE_GATT();
      int8_t controllerServiceId;
      int8_t stateCharId;
      int8_t timeInStateCharId;
      int8_t timeHeatingCharId;
      int8_t targetTempCharId;
      int8_t waterSensorCharId;
      int8_t ambientSensorCharId;
  };
#endif

