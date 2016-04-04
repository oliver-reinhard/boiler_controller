#ifndef BOILER_STORAGE_H_INCLUDED
  #define BOILER_STORAGE_H_INCLUDED
  
  #include "config.h"
  #include "log.h"
  
  // EEPROM content
  typedef unsigned short Version;
  
  #define EEPROM_LAYOUT_VERSION 1

  /*
   * The EEPROM storage structure is as follows:
   * - Version number
   * - ConfigParams
   * - Max number of log entries
   * - Log entries (LogEntry[MAX_LOG_ENTRIES])
   */

  class Storage {

    public:
      
      /*
       * Returns version identifier.
       */
      virtual Version version();
      
      /*
       * Returns size of storage in bytes.
       */
      unsigned short size();

      /*
       * CONFIG PARAMS
       */
      
      /**
       * Clears the version and all config-parameter values stored in the EEPROM.
       */
      virtual void clearConfigParams();
      
      /**
       * Reads the ConfigParams structure from the EEPROM and initialises unset parameter values to their defaults.
       * Writes initialised default values back to EEPROM.
       */
      virtual void getConfigParams(ConfigParams *configParams);
    
      
      /**
       * Updates changed parameter values on the EEPROM. Does not change values of the ConfigParams structure.
       */
      virtual void updateConfigParams(const ConfigParams *configParams);

      /*
       * LOGGING
       */
       
      /*
       * Returns maximum number of log entries.
       */
      unsigned short maxLogEntries();
      
      /*
       * Returns current number of log entries.
       */
      unsigned short currentLogEntries();
      
      /**
       * Clear all log entries on the EEPROM and reset in-memory log-managment structures.
       */
      virtual void resetLog();
      
      /**
       * Initialise in-memory log-managment structures from the log entries found in the EEPROM.
       * This is typically performed after an Arduino board-reset.
       * If the maximum number of log entries is found to be different from the previous run, then the
       * log is cleared and a message is logged that records the change in size.
       */
      virtual void initLog();

      /*
       * Log a value change.
       */
      virtual Timestamp logValues(Temperature water, Temperature ambient, Flags flags);

      /*
       * Log a state change.
       */
      virtual Timestamp logState(StateID previous, StateID current, EventID event);

      /*
       * Log a message.
       */
      virtual Timestamp logMessage(MessageID id, short param1, short param2);

    protected:
        
      /*
       * Index of the next empty log entry (yet to be written); the entry pointed to has been cleared already
       */
      unsigned short logHead = 0;

      /*
       * Index of the oldest log entry (there is always one!)
       */
      unsigned short logTail = 0;
    
    #ifdef UNIT_TEST
    public:
    #endif
      /*
       * Reads and returns the ConfigParams structure from the EEPROM. No initialisation of values is performed.
       */
      virtual void readConfigParams(ConfigParams *configParams);
  
      /*
       * Ensures every parameter has been set to a value different from 0 and, if not, sets it to its default value.
       * The out parameter 'updated' tells, whether any values were affected.
       */
      void initConfigParams(ConfigParams *configParams, boolean *updated /* OUT */);

      /**
       * Clears the log entry at the current index but does not update logHead or logTail.
       */
      void clearLogEntry(unsigned short index);
      
      /**
       * Writes a log entry at the current logHead position and updates logHead and logTail.
       */ 
      void writeLogEntry(const LogEntry *entry);

      /*
       * Public accessor to indices of log head and log tail.
       */
      unsigned short logHeadIndex();
      unsigned short logTailIndex();
  };

#endif // BOILER_STORAGE_H_INCLUDED
