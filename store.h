#ifndef BOILER_STORE_H_INCLUDED
  #define BOILER_STORE_H_INCLUDED
  
  #include "config.h"
  #include "log.h"
  
  // EEPROM content
  typedef uint16_t Version;
  
  #define EEPROM_LAYOUT_VERSION 1

  typedef enum {
    LOG_READER_MOST_RECENT = 0,  // reads newer to older
    LOG_READER_UNNOTIFIED = 1    // reads older to newer
  } LogReaderTypeEnum;

  /*
   * Simple, non-concurrent reader for log entries.
   */
  struct LogReader {
    /*
     * Determines reader behaviour.
     */
    LogReaderTypeEnum type;
    /*
     * The values in this struct are defined only if valid == true.
     */
    boolean valid;
    /*
     * Number of entries to be returned through this reader (remains constant).
     */
    uint16_t toRead;
    /*
     * Number of entries alreday returned by this reader (increases with each entry read);
     */
    uint16_t read = 0;
    /*
     * Index of next entry that will be returned.
     */
    uint16_t nextIndex;
  };

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
       * Returns static size of storage in uint8_ts.
       */
      uint16_t size();

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
      virtual void updateConfigParams(ConfigParams *configParams);
      

      /*
       * LOGGING
       */
       
      /*
       * Returns number of available slots for log entries.
       * Note: this is always 1 less than the actual number of slots because the next available slot is always cleared ahead of time).
       */
      uint16_t maxLogEntries();
      
      /*
       * Returns current number of log entries.
       */
      uint16_t currentLogEntries();
      
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
      virtual Timestamp logMessage(MessageEnum msg, int16_t param1, int16_t param2);

      /*
       * Log a config-param change.
       */
      virtual Timestamp logConfigParam(ConfigParamID id, float newValue);
      
      /*
       * Initialises the LogEntry reader to return at most maxResults of the most recent entries.
       * The entries are returned in decending order by timestamp (most recent first).
       * 
       * @param maxResults indicates how many log entries to return as a maximum; the special value 0 means to return all log entries
       * Note: the reader is only valid as long the log is not being modified.
       */
      void readMostRecentLogEntries(uint16_t maxResults);

      /*
       * Initialises the LogEntry reader to return all the log entries that have not yet been notified to the client(s). 
       * The entries are returned in ascending order by timestamp (oldest unnotified entry first).      
       * Note: the reader is only valid as long the log is not being modified.
       */
      void readUnnotifiedLogEntries();

      /*
       * Retuns the next entry of the log. 
       * Precondition: readMostRecentLogEntries() or readUnnotifiedLogEntries() was called and the log has not been modified since.
       * 
       * @return true means the parameter 'entry' contains the next log entry, false means 'entry' has no defined semantics (i.e. after the last entry has been returned or if the log has been modified)
       */
      boolean nextLogEntry(LogEntry *entry);

    protected:
        
      /*
       * Index of the next empty log entry (yet to be written); the entry pointed to has been cleared already
       */
      uint16_t logHead = 0;

      /*
       * Index of the oldest log entry (there is always one!)
       */
      uint16_t logTail = 0;

      /*
       * Non-concurrent reader (=cursor) to iterate over log entries.
       */
      LogReader reader;
      
      /*
       * Index of last log entry that was notified to user.
       */
      uint16_t lastNotifiedLogEntryIndex = 0;
    
    #ifdef UNIT_TEST
    // public for testing purposes only:
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
      void clearLogEntry(uint16_t index);
      
      /**
       * Writes a log entry at the current logHead position and updates logHead and logTail.
       */ 
      void writeLogEntry(const LogEntry *entry);

      /*
       * Accessors to indices of log head and log tail.
       */
      uint16_t logHeadIndex();
      uint16_t logTailIndex();

      /*
       * Accessor to LogReader.
       */
      LogReader *getLogReader();
  };

#endif // BOILER_STORAGE_H_INCLUDED
