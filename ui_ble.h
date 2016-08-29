#ifndef BC_UI_BLE_H_INCLUDED
  #define BC_UI_BLE_H_INCLUDED

  #include "ui.h"
  #include "Adafruit_BLEGatt.h"
  #include <Adafruit_BluefruitLE_SPI.h>
  
  // ----------------------------------------------------------------------------------------------
  // The following macros declare the pins to use for HW and SW SPI communication.
  // SCK, MISO and MOSI should be connected to the HW SPI pins on the Uno when
  // using HW SPI.  This should be used with nRF51822 based Bluefruit LE modules
  // that use SPI (Bluefruit LE SPI Friend).
  // ----------------------------------------------------------------------------------------------
  #define BLUEFRUIT_SPI_CS               8
  #define BLUEFRUIT_SPI_IRQ              7
  #define BLUEFRUIT_SPI_RST              4    // Optional but recommended, set to -1 if unused
  
  #define USER_CMD_PARAMETER_MAX_SIZE 8
  
  class BLEUI : public NullUI {
    public:
      BLEUI(ExecutionContext *context) : NullUI(context) { }
      
      void setup();
    
      void readUserRequest();
      
      void commandExecuted(boolean success);
      
      void provideUserInfo(BoilerStateAutomaton *automaton);
    
      void notifyStatusChange(StatusNotification *notification);
    
      void notifyNewLogEntry(LogEntry entry);

    protected:
      Adafruit_BluefruitLE_SPI ble = Adafruit_BluefruitLE_SPI(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);
      Adafruit_BLEGatt gatt = Adafruit_BLEGatt(ble);

      void setDeviceName(const char *name);

      /** Service ID */
      int8_t controllerSID;
      /* Status Characteristics IDs */
      int8_t stateCID;
      int8_t timeInStateCID;
      int8_t timeHeatingCID;
      int8_t acceptedUserCommandsCID;
      int8_t userRequestCID;
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

