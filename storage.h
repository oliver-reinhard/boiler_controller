#ifndef BOILER_STORAGE_H_INCLUDED
  #define BOILER_STORAGE_H_INCLUDED
  
  #include "config.h"
  #include "log.h"
  
  // EEPROM content
  typedef unsigned short Version;
  
  #define EEPROM_LAYOUT_VERSION 1
  
  /*
   * Returns version identifier.
   */
  Version version();
  
  /*
   * Returns size of storage in bytes.
   */
  unsigned short size();
  
  /**
   * Clears the version and all config-parameter values on the EEPROM.
   */
  void clearConfigParams();
  
  /**
   * Reads the ConfigParams structure from the EEPROM and initialises unset parameter values to their defaults.
   * Writes initialised default values back to EEPROM.
   */
  void getConfigParams(ConfigParams *configParams);

  
  /**
   * Updates changed parameter values on the EEPROM. Does not change values of the ConfigParams structure.
   */
  void updateConfigParams(const ConfigParams *configParams);
  
  /*
   * Returns maximum number of log entries.
   */
  unsigned short maxLogEntries();
  
  /*
   * Returns current number of log entries.
   */
  unsigned short currentLogEntries();
  
  /**
   * Initialise in-memory log-managment structures from the log entries found in the EEPROM.
   * This is typically performed after an Arduino board-reset.
   */
  void initLog();
  
  /**
   * Clear all log entries on the EEPROM and reset in-memory log-managment structures.
   */
  void resetLog();
  
  
  void logValues(Temperature water, Temperature ambient, Flags flags);
  
  void logState(StateID previous, StateID current, EventID event);
  
  void logMessage(MessageID id, short param1, short param2);
  
  /*
   * TESTING ONLY
   */
  #ifdef UNIT_TEST
    
    /*
     * Reads and returns the ConfigParams structure from the EEPROM. No initialisation of values is performed.
     */
    void readConfigParams(ConfigParams *configParams);

    /*
     * Ensures every parameter has been set to a value different from 0 and, if not, sets it to its default value.
     * The out parameter 'updated' tells, whether any values were affected.
     */
    void initConfigParams(ConfigParams *configParams, boolean *updated /* OUT */);
    
    unsigned short logHeadIndex();
    unsigned short logTailIndex();
  
  #endif

#endif // BOILER_STORAGE_H_INCLUDED
