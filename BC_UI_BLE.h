#ifndef BC_UI_BLE_H_INCLUDED
  #define BC_UI_BLE_H_INCLUDED

  #include "BC_UI.h"
  #include <Adafruit_BLEGatt.h>
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
  
  
  class BLEUI : public AbstractUI {
    public:
      
      BLEUI() : AbstractUI() { }
      
      void init(ExecutionContext *context);
    
      void readUserRequest();
      
      void commandExecuted(boolean success);
      
      void provideUserInfo(BoilerStateAutomaton *automaton);
    
      void notifyStatusChange(StatusNotification *notification);
    
      void notifyNewLogEntry(LogEntry entry);

    protected:
      Adafruit_BluefruitLE_SPI ble = Adafruit_BluefruitLE_SPI(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);
      Adafruit_BLEGatt gatt = Adafruit_BLEGatt(ble);

      void setDeviceName(const char *name);
      //void addServiceChecked(const uint16_t uuid, const uint8_t sid, PGM_P description, uint16_t line);
      void addServiceChecked(const uint8_t uuid128[], const uint8_t sid, PGM_P description, uint16_t line);
      void addCharacteristicChecked(const uint16_t uuid16, const uint8_t cid, uint8_t properties, uint8_t min_len, uint8_t max_len, BLEDataType_t datatype, PGM_P description, uint16_t line);

  };
#endif

