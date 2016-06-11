#include "ui.h"


void AbstractUI::setup(ControlContext *context) {
  if (context != NULL) { } // prevent 'unused parameter' warning
}

void AbstractUI::readUserCommand(ControlContext *context) {
  if (context != NULL) { } // prevent 'unused parameter' warning
}

void AbstractUI::processReadWriteRequests(ReadWriteRequests requests, ControlContext *context, BoilerStateAutomaton *automaton) {
  if (requests != READ_WRITE_NONE || context != NULL || automaton != NULL) { } // prevent 'unused parameter' warning
}

void AbstractUI::notifyStatusChange(StatusNotification notification) {
  if (notification.timeInState == 0L) { } // prevent 'unused parameter' warning
}

void AbstractUI::notifyNewLogEntry(LogEntry entry) {
  if (entry.timestamp != UNDEFINED_TIMESTAMP) { } // prevent 'unused parameter' warning
}
  

#ifdef BLE_UI

  #include <SPI.h>
  #include "Adafruit_BLE.h"
  #include "Adafruit_BluefruitLE_SPI.h"

  #define DEBUG_BLE true

  // SHARED SPI SETTINGS
  // ----------------------------------------------------------------------------------------------
  // The following macros declare the pins to use for HW and SW SPI communication.
  // SCK, MISO and MOSI should be connected to the HW SPI pins on the Uno when
  // using HW SPI.  This should be used with nRF51822 based Bluefruit LE modules
  // that use SPI (Bluefruit LE SPI Friend).
  // ----------------------------------------------------------------------------------------------
  #define BLUEFRUIT_SPI_CS               8
  #define BLUEFRUIT_SPI_IRQ              7
  #define BLUEFRUIT_SPI_RST              4    // Optional but recommended, set to -1 if unused

  /* Communicate to BLE chip: hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
  Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

  
  // A small helper
  void error(const __FlashStringHelper *err) {
    Serial.println(err);
    while (1);
  }


  void BLEUI::setup(ControlContext *context) {
    if (context != NULL) { } // prevent 'unused parameter' warning
    
    boolean success;
    
    /* Initialise the module */
    Serial.print(F("Initialising the Bluefruit LE module: "));
  
    if ( !ble.begin(DEBUG_BLE) )
    {
      error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
    }
    Serial.println( F("OK!") );
  
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if (! ble.factoryReset() ){
         error(F("Couldn't factory reset"));
    }
  
    /* Disable command echo from Bluefruit */
    ble.echo(false);
  
    Serial.println("Requesting Bluefruit info:");
    /* Print Bluefruit information */
    ble.info();
  
    // this line is particularly required for Flora, but is a good idea
    // anyways for the super long lines ahead!
    // ble.setInterCharWriteDelay(5); // 5 ms
  
    /* Change the device name to make it easier to find */
    Serial.println(F("Setting device name to 'Bluefruit HRM': "));
  
    if (! ble.sendCommandCheckOK(F("AT+GAPDEVNAME=Boiler Controller")) ) {
      error(F("Could not set device name?"));
    }
  
    /* Add the Heart Rate Service definition */
    /* Service ID should be 1 */
    Serial.println(F("Adding Service definition (UUID = 0x180D): "));
    success = ble.sendCommandWithIntReply( F("AT+GATTADDSERVICE=UUID=0x180D"), &bcServiceId);
    if (! success) {
      error(F("Could not add service"));
    }

    /*
      0x02 - Read
      0x04 - Write Without Response 0x08 - Write
      0x10 - Notify
      0x20 - Indicate
    */
    /* Characteristic ID for Measurement should be 1 */
    Serial.println(F("Adding Status characteristic (UUID = 0x2A37): "));
    uint16_t structSize = sizeof(StatusNotification);
    success = ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2A37, PROPERTIES=0x10, MIN_LEN=2, MAX_LEN=3, VALUE=00-40"), &statusCharacteristicId);
    if (! success) {
      error(F("Could not add Status characteristic"));
    }
  
    /* Add the Body Sensor Location characteristic */
    /* Chars ID for Body should be 2 */
    Serial.println(F("Adding Log characteristic (UUID = 0x2A38): "));
    success = ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2A38, PROPERTIES=0x02, MIN_LEN=1, VALUE=3"), &logCharacteristicId);
    if (! success) {
      error(F("Could not add Log characteristic"));
    }
  
    /* Add the Service to the advertising data (needed for Nordic apps to detect the service) */
    Serial.print(F("Adding Service UUID to the advertising payload: "));
    ble.sendCommandCheckOK( F("AT+GAPSETADVDATA=02-01-06-05-02-0d-18-0a-18") );
  
    /* Reset the device for the new service setting changes to take effect */
    Serial.print(F("Performing a SW reset (service changes require a reset): "));
    ble.reset();
  
    Serial.println();
    
  }
  
  void BLEUI::readUserCommand(ControlContext *context) {
    if (context != NULL) { } // prevent 'unused parameter' warning
  }
  
  void BLEUI::processReadWriteRequests(ReadWriteRequests requests, ControlContext *context, BoilerStateAutomaton *automaton) {
    if (requests != READ_WRITE_NONE || context != NULL || automaton != NULL) { } // prevent 'unused parameter' warning
  }
  
  void BLEUI::notifyStatusChange(StatusNotification notification) {
    if (notification.timeInState == 0L) { } // prevent 'unused parameter' warning
    /* Command is sent when \n (\r) or println is called */
    /* AT+GATTCHAR=CharacteristicID,value */
    ble.print( F("AT+GATTCHAR=") );
    ble.print( statusCharacteristicId );
    ble.print( F(",00-") );
    uint16_t heart_rate = 144;
    ble.println(heart_rate, HEX);
  }
  
  void BLEUI::notifyNewLogEntry(LogEntry entry) {
    if (entry.timestamp != UNDEFINED_TIMESTAMP) { } // prevent 'unused parameter' warning
  }
  
#endif
