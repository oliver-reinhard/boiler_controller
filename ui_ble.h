#ifndef BC_UI_BLE_H_INCLUDED
  #define BC_UI_BLE_H_INCLUDED

  #include "ui.h"
  #include "src/ble_gatt/Adafruit_BluefruitLE_GATT.h"

  #define USER_CMD_PARAMETER_MAX_SIZE 8
  
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

      /** Service ID */
      int8_t controllerSID;
      /* Status Characteristics IDs */
      int8_t stateCID;
      int8_t timeInStateCID;
      int8_t timeHeatingCID;
      int8_t acceptedUserCommandsCID;
      int8_t userCommandCID;
      int8_t waterSensorCID;
      int8_t ambientSensorCID;
      
      /* Configuration Characteristics IDs */
      int8_t configChangedCID;
      int8_t targetTempCID;
    
       /* Log Characteristics IDs */
      int8_t logEntryCID;

      /* A random value set by the remote interface to flag that one of the config values has changed. */
      uint32_t lastConfigChangedStamp = 0L;
  };
#endif

