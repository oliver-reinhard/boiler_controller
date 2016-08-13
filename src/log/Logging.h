#ifndef BC_LOGGING_H_INCLUDED
  #define BC_LOGGING_H_INCLUDED
  
  #include <EEPROM.h>
  #include "LogTime.h"

  // Define this symbol in an including module (prior to #include "Logging.h") to have a different payload size (in Byte):
  #ifndef LOG_DATA_PAYLOAD_SIZE
    #define LOG_DATA_PAYLOAD_SIZE 5
  #endif

  // Define this symbol in an including module (prior to #include "Logging.h") to define the LED pin for issuing fatal S.O.S.:
  #ifndef SOS_LED_PIN
    #define SOS_LED_PIN 13
  #endif

  /* 
   *  Storage type for message identifiers.
   */
  typedef uint8_t MessageID;
  
  /*
   * The messages issued by the implementation of this module. Stored as MessageID.
   * Note: the actual message texts and the conversion of message IDs to text have to be implemented by 
   *       the consumer of this libraray 
   */
  typedef enum {
    MSG_SYSTEM_INIT = 0,      // The board is initialising from reset or power on [no parameters]
    MSG_LOG_INIT = 1,         // Log initialised [no parameters]
    MSG_LOG_SIZE_CHG = 2,     // Number of log entries has changed from [old] to [new])
  } AbstractLogMessageEnum;
  
  /**
   * Generic "supertype" for log data; "subtypes" are distinguished via LogDataTypeID.
   * Note: The actual payload size can be configured / changed via symbol definition (LOG_DATA_PAYLOAD_SIZE).
   */
  struct LogData {
    byte payload[LOG_DATA_PAYLOAD_SIZE]; // placeholder
  };
  
  /*
   * Discriminator for various types of log data.
   */
  typedef uint8_t LogDataTypeID;
  
  /**
   * Actual log record. At runtime the data field is an instance of a "subtype" of LogData.
   */
  struct LogEntry {
    Timestamp timestamp;
    LogDataTypeID type;
    LogData   data; // generic
  };

    
  typedef enum {
    LOG_READER_MOST_RECENT = 0,  // reads newer to older
    LOG_READER_UNNOTIFIED = 1    // reads older to newer
  } LogReaderKindEnum;


  /*
   * Simple, non-concurrent reader for log entries.
   */
  struct LogReader {
    /*
     * Determines reader behaviour.
     */
    LogReaderKindEnum kind;
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
   * Logging is done to the EEPROM of the Arduino.
   * 
   * The log structure is as follows:
   * - logEntrySlots (=total number of log entry slots; used to detect changes => reset)
   * - Actual log entries (LogEntry[logEntrySlots])
   */
  class AbstractLog {
    
    public:
      /*
       * Use all available EEPROM space after eepromOffset.
       * 
       * @param eepromOffset number of bytes this object's storage is offset from the first byte of the EEPROM store.
       */
      AbstractLog(uint16_t eepromOffset) : AbstractLog(eepromOffset, EEPROM.length() - eepromOffset) { };
      
      /*
       * @param eepromOffset number of bytes this object's storage is offset from the first byte of the EEPROM store.
       * @param number of bytes allocated for logging.
       */
      AbstractLog(uint16_t eepromOffset, uint16_t eepromlogSize);
      
      /**
       * Initialise in-memory log-managment structures from the log entries found in the EEPROM.
       * This is typically performed after an Arduino board-reset.
       * If the maximum number of log entries is found to be different from the previous run, then the
       * log is cleared and a message is logged to record the change in size.
       */
      virtual void init();
      
      /**
       * Clear all log entries on the EEPROM and reset in-memory log-managment structures.
       */
      virtual void clear();

      /*
       * The timestamp generator for this log.
       */
      LogTime logTime = LogTime();
      
      /*
       * Returns number of available slots for log entries.
       * Note: this is always 1 less than the actual number of slots because the next available slot is always cleared ahead of time).
       */
      uint16_t maxLogEntries();
      
      /*
       * Returns current number of log entries.
       */
      uint16_t currentLogEntries();

      /*
       * Log a message.
       * Note: this function is purely virtual. It has not been implemented in order to leave the definition of
       *       the message data structure to the consumers of this library. Use addLogEntry() to create of a new log entry.
       */
      virtual Timestamp logMessage(MessageID id, int16_t param1, int16_t param2) = 0;
      
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
       * Retuns the "next" entry of the log ("next" can be the next or the previous, depending on the reader kind). 
       * Precondition: readMostRecentLogEntries() or readUnnotifiedLogEntries() was called and the log has not been modified since.
       * 
       * @return true means the parameter 'entry' contains the next log entry, false means 'entry' has no defined semantics (i.e. after the last entry has been returned or if the log has been modified)
       */
      boolean nextLogEntry(LogEntry &entry);

      /*
       * Log a message, halt program execution and blink the universal S-O-S code on the Arduino board's LED.
       * Note: The actual LED pin can be configured / changed via symbol definition (SOS_LED_PIN).
       */
      void S_O_S(MessageID id, int16_t param1, int16_t param2);
      
      /*
       * Write a debug string to Serial, halt program execution and blink the universal S-O-S code on the Arduino board's LED.
       * Note: The actual LED pin can be configured / changed via symbol definition (SOS_LED_PIN).
       */
      void S_O_S(const __FlashStringHelper *debug);

    protected:
      /*
       * Byte offset of log space within EEPROM.
       */
      uint16_t eepromOffset;
      /*
       * Number of bytes of log space.
       */
      uint16_t eepromLogSize;
      /*
       * The number of slots reserved for log entries in log space.
       * Note: this is the number of slots with differs from 'maximum number' which is the actually available number of slots (one slot is always kept free)
       */
      uint16_t logEntrySlots;
      
  #ifdef UNIT_TEST  // make available for unit tests
    public:
  #endif
      /*
       * Index of the next empty log entry (yet to be written); the entry pointed to has been cleared already
       */
      uint16_t logHeadIndex = 0;
      /*
       * Index of the oldest log entry (there is always one!)
       */
      uint16_t logTailIndex = 0;
      /*
       * Non-concurrent reader (=cursor) to iterate over log entries.
       */
      LogReader reader;
      
  #ifdef UNIT_TEST
    protected:
  #endif
      /*
       * Index of last log entry that was notified to user.
       */
      uint16_t lastNotifiedLogEntryIndex = 0;

      /*
       * Calculates the byte-offset within the logging EEPROM space for the given entry index.
       */
      uint16_t entryOffset(uint16_t index);

      /**
       * Clears the log entry at the current index but does not update logHead or logTail.
       */
      void clearLogEntry(uint16_t index);
      
      /**
       * Creates and adds a log entry at the current logHead position, clears the next entry and updates logHead and logTail.
       */ 
      LogEntry addLogEntry(LogDataTypeID type, LogData *data);
  };


#endif
